#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "OV2640.h"
#include "Config.h"

#include "camera_pins.h"

OV2640 cam; // 创建一个摄像头对象
WebServer server(80);

/*Commonly used variables*/
volatile size_t camSize; // size of the current frame, byte
volatile char *camBuf;   // pointer to the current frame

/*RTOS Task handles*/
TaskHandle_t tMjpeg = NULL;  // handles client connections to the webserver
TaskHandle_t tCam = NULL;    // handles getting picture frames from the camera and storing them locally
TaskHandle_t tStream = NULL; // actually streaming frames to all connected clients

SemaphoreHandle_t frameSync = xSemaphoreCreateBinary(); // 二进制信号量

//=============摄像头配置======================
void CamConfig()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  if (cam.init(config) != ESP_OK)
  {
    Serial.println("Error initializing the camera");
    ESP.restart();
  }
}

//============== Server Connection Handler Task ==============
void mjpegCB(void *pvParameters)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(interval); // 毫秒转换为内部时间单位 TickType_t

  xSemaphoreGive(frameSync);

  xTaskCreatePinnedToCore(camCB, "CAM", 1024 * 5, NULL, 2, &tCam, 1);
  xTaskCreatePinnedToCore(streamCB, "streamCB", 1024 * 5, NULL, 2, &tStream, 1);

  /*Registering webserver handling routines*/
  server.on("/mjpeg/1", handleJPGSstream);
  server.onNotFound(handleNotFound);

  server.begin(); // Starting webserver
  xLastWakeTime = xTaskGetTickCount();

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    server.handleClient();
    taskYIELD();
  }
}

/*RTOS task to grab frames from the camera*/
void camCB(void *pvParameters)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(1000 / FPS); // A running interval associated with currently desired frame rate
  portMUX_TYPE xSemaphore = portMUX_INITIALIZER_UNLOCKED;  // 用于控制对共享资源的访问，防止多个任务同时访问共享资源时发生竞争和冲突。

  // Pointers to the 2 frames, their respective sizes and index of the current frame
  char *fbs[2] = {NULL, NULL};
  size_t fSize[2] = {0, 0};
  int ifb = 0;

  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    // Grab a frame from the camera and query its size
    cam.run();                // 获取视频帧
    size_t s = cam.getSize(); //  fb->len

    // If frame size is more that we have previously allocated - request  125% of the current frame space
    if (s > fSize[ifb])
    {
      fSize[ifb] = s * 4 / 3;
      fbs[ifb] = allocateMemory(fbs[ifb], fSize[ifb]);
    }

    // Copy current frame into local buffer
    char *b = (char *)cam.getfb();
    memcpy(fbs[ifb], b, s);

    taskYIELD(); // 将当前任务挂起
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xSemaphoreTake(frameSync, portMAX_DELAY);

    //  Do not allow interrupts while switching the current frame
    portENTER_CRITICAL(&xSemaphore);
    camBuf = fbs[ifb];
    camSize = s;
    ifb++;
    ifb &= 1; // this should produce 1, 0, 1, 0, 1 ... sequence
    portEXIT_CRITICAL(&xSemaphore);

    //  Let anyone waiting for a frame know that the frame is ready
    xSemaphoreGive(frameSync);
    xTaskNotifyGive(tStream);
    taskYIELD();
    if (eTaskGetState(tStream) == eSuspended)
    {
      vTaskSuspend(NULL); // passing NULL means "suspend yourself"
    }
  }
}

char *allocateMemory(char *aPtr, size_t aSize)
{
  //  Since current buffer is too smal, free it
  if (aPtr != NULL)
    free(aPtr);

  size_t freeHeap = ESP.getFreeHeap();
  char *ptr = NULL;

  // If memory requested is more than 2/3 of the currently free heap, try PSRAM immediately
  if (aSize > freeHeap * 2 / 3)
  {
    if (psramFound() && ESP.getFreePsram() > aSize)
    {
      ptr = (char *)ps_malloc(aSize);
    }
  }
  else
  {
    //  Enough free heap - let's try allocating fast RAM as a buffer
    ptr = (char *)malloc(aSize);

    //  If allocation on the heap failed, let's give PSRAM one more chance:
    if (ptr == NULL && psramFound() && ESP.getFreePsram() > aSize)
    {
      ptr = (char *)ps_malloc(aSize);
    }
  }

  // Finally, if the memory pointer is NULL, we were not able to allocate any memory, and that is a terminal condition.
  if (ptr == NULL)
  {
    ESP.restart();
  }
  return ptr;
}

// ======= Streaming =======================
const char HEADER[] = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);

/* Handle connection request from clients*/
void handleJPGSstream(void)
{
  //  Can only acommodate 10 clients. The limit is a default for WiFi connections
  if (!uxQueueSpacesAvailable(streamingClients))
    return;

  //  Create a new WiFi Client object to keep track of this one
  WiFiClient *client = new WiFiClient();
  *client = server.client();

  //  Immediately send this client a header
  client->write(HEADER, hdrLen);
  client->write(BOUNDARY, bdrLen);

  // Push the client to the streaming queue
  xQueueSend(streamingClients, (void *)&client, 0);

  // Wake up streaming tasks, if they were previously suspended:
  if (eTaskGetState(tCam) == eSuspended)
    vTaskResume(tCam);
  if (eTaskGetState(tStream) == eSuspended)
    vTaskResume(tStream);
}

/*Actually stream content to all connected clients*/
void streamCB(void *pvParameters)
{
  char buf[16];
  TickType_t xLastWakeTime;
  TickType_t xFrequency;

  ulTaskNotifyTake(pdTRUE,         /* Clear the notification value before exiting. */
                   portMAX_DELAY); /* Block indefinitely. */

  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    xFrequency = pdMS_TO_TICKS(1000 / FPS);
    //  Only bother to send anything if there is someone watching
    UBaseType_t activeClients = uxQueueMessagesWaiting(streamingClients);
    if (activeClients)
    {
      // Adjust the period to the number of connected clients
      xFrequency /= activeClients;

      //  Since we are sending the same frame to everyone,
      //  pop a client from the the front of the queue
      WiFiClient *client;
      xQueueReceive(streamingClients, (void *)&client, 0);

      //  Check if this client is still connected.
      if (!client->connected())
      {
        //  delete this client reference if s/he has disconnected
        //  and don't put it back on the queue anymore. Bye!
        delete client;
      }
      else
      {
        //  Ok. This is an actively connected client.
        //  Let's grab a semaphore to prevent frame changes while we
        //  are serving this frame
        xSemaphoreTake(frameSync, portMAX_DELAY);

        client->write(CTNTTYPE, cntLen);
        sprintf(buf, "%d\r\n\r\n", camSize);
        client->write(buf, strlen(buf));
        client->write((char *)camBuf, (size_t)camSize);
        client->write(BOUNDARY, bdrLen);

        // Since this client is still connected, push it to the end
        // of the queue for further processing
        xQueueSend(streamingClients, (void *)&client, 0);

        //  The frame has been served. Release the semaphore and let other tasks run.
        //  If there is a frame switch ready, it will happen now in between frames
        xSemaphoreGive(frameSync);
        taskYIELD();
      }
    }
    else
    {
      //  Since there are no connected clients, there is no reason to waste battery running
      vTaskSuspend(NULL);
    }
    //  Let other tasks run after serving every client
    taskYIELD();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

/*Handle invalid URL requests */
void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found");
}

/* Configure and connect to WiFi*/
void ConnectWIFI()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.print("Stream Link: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/mjpeg/1");
}

//===============Websocket部分，把图传数据发送给ESP32C3==============================
void Websocket(void *pt)
{

  while (!client1.connect(websockets_server_host, websockets_server_port, "/"))
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Socket Connected!");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(100);

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    camera_fb_t *fb = esp_camera_fb_get();
    uint8_t *imagebuff = fb->buf;
    size_t imagelen = fb->len;
    esp_camera_fb_return(fb);
    client1.sendBinary((const char *)imagebuff, imagelen);
    taskYIELD();
  }
}
//==================================================================================

void setup()
{
  Serial.begin(115200);
  ConnectWIFI();
  CamConfig();

  xTaskCreatePinnedToCore(Websocket, "", 1024 * 5, NULL, 2, NULL, 1);  // 用于ESP32C3图像显示
  xTaskCreatePinnedToCore(mjpegCB, "", 1024 * 6, NULL, 2, &tMjpeg, 1); // 通过访问URL显示图像

  // vTaskDelete(NULL);
}

void loop()
{
  // taskYIELD();
}