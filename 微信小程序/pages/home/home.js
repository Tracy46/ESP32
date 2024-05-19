const key = '9a15fc50a5414cb4b1615d49d390030a'; // 和风天气KEY
const cityID = '101200113'; // 城市ID，武汉市，洪山区
var mqtt = require('../../utils/mqtt.min');

var client = null;

/* 阿里云MQTT连接参数 */
const clientId = 'i3c6vjLss9C.Wechat|securemode=2,signmethod=hmacsha256,timestamp=1682511035727|';
const username = 'Wechat&i3c6vjLss9C';
const password = '4c86cf1aeac5f648740b6886a267ee8665df7f255ce176f4ec608b420601dade';
const connectUrl = 'wxs://iot-06z00formvsxwnm.mqtt.iothub.aliyuncs.com';
const topic1 = '/i3c6vjLss9C/Wechat/user/MPPT_data'; // 订阅ESP32发送过来的数据主题

Page({
  data: {
    date: '',
    temp: 0, // 温度
    humi: 0, // 湿度
    airText: '良', // 空气质量等级
    weather: '晴', // 天气状况
    weatherAdvice: '儿童、老年人及心脏病、呼吸系统疾病患者应尽量减少外出，停留在室内，一般人群应适量减少户外运动。', // 天气建议
    windSpeed: 0, // 风速
    pressure: 0, //大气压强
    PM10: 0,
    PM25: 0,
    NO2: 0,
    SO2: 0,
    CO: 0,
    O3: 0,
    mqttStatus: 0,
    mqttControl_text: '连接MQTT',
    S1: 0, // 工作温度
    S2: 0, //偏航角
    S3: 0, // 偏航角速度
  },
  onLoad(options) {
    this.weatherRequest();
  },
  weatherRequest() {
    var that = this;
    wx.request({
      url: `https://devapi.qweather.com/v7/weather/now?location=${cityID}&key=${key}`, //实时天气API
      success(res) {
        // console.log(res.data)
        // 数据绑定
        that.setData({
          temp: res.data.now.temp,
          weather: res.data.now.text,
          humi: res.data.now.humidity,
          windSpeed: ((res.data.now.windSpeed) / 3.6).toFixed(1),
          pressure: res.data.now.pressure
        })
      }
    });
    wx.request({
      url: `https://devapi.qweather.com/v7/air/now?location=${cityID}&key=${key}`, //实时空气质量
      success(res) {
        // console.log(res.data)
        that.setData({
          PM10: res.data.now.pm10,
          PM25: res.data.now.pm2p5,
          NO2: res.data.now.no2,
          SO2: res.data.now.so2,
          CO: res.data.now.co,
          O3: res.data.now.o3
        })
      }
    });
    wx.request({
      url: `https://devapi.qweather.com/v7/indices/1d?type=1,2&location=${cityID}&key=${key}`, //天气指数预报
      success(res) {
        // console.log(res.data)
        that.setData({
          date: res.data.daily[0].date,
          weatherAdvice: res.data.daily[0].text
        })
      }
    });
  },
  connectMQTT() {
    const options = {
      connectTimeout: 4000, // 超时时间
      reconnectPeriod: 30 * 1000, // 重连时间间隔
      clientId,
      username,
      password
    }
    // 连接MQTT服务器
    client = mqtt.connect(connectUrl, options);
    // 连接成功后触发的回调函数
    client.on('connect', () => {
      console.log('服务器连接成功')
      wx.showToast({
        title: 'MQTT连接成功',
      })
      client.subscribe(topic1, {
        qos: 0
      }, function (err) {
        if (!err) {
          console.log('订阅成功')
        } else {
          console.log('订阅失败')
        }
      })
    });
    var that = this;
    // 当客户端收到一个发布过来的消息时触发回调
    client.on('message', function (topic, message) {
      // message是16进制的buffer字节流
      // console.log('收到:', message.toString())
      // console.log('收到:', message)
      // console.log('数据对应订阅主题：', topic)
      let dataFromDev = {}
      try {
        dataFromDev = JSON.parse(message);
        console.log('收到:', dataFromDev);
        that.setData({
          S1: dataFromDev.S1.toFixed(2)
        })
      } catch (error) {
        console.log("JSON信息解析失败");
      }
    });
    client.on('reconnect', (error) => {
      console.log('正在重连：', error)
      wx.showToast({
        title: 'MQTT正在重连',
      })
    });
    client.on('error', (error) => {
      console.log('连接失败：', error)
      wx.showToast({
        title: 'MQTT连接失败',
      })
    });
    client.on('close', function () {
      console.log('MQTT已断开连接')
      wx.showToast({
        title: 'MQTT已断开连接',
      })
    })
  },
  disconnectMQTT() {
    client.end();
    client.on('close', function () {
      console.log('MQTT已断开连接')
      wx.showToast({
        title: 'MQTT已断开连接',
      })
    })
  },
  // MQTT连接控制
  MQTTControl() {
    if (this.data.mqttStatus == 0) {
      this.connectMQTT();
      this.setData({
        mqttStatus: 1,
        mqttControl_text: '断开MQTT'
      })
    } else if (this.data.mqttStatus == 1) {
      this.disconnectMQTT()
      this.setData({
        mqttStatus: 0,
        mqttControl_text: '连接MQTT'
      })
    }
  }
})