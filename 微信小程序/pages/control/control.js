var mqtt = require('../../utils/mqtt.min');
const crypto = require('../../utils/hex_hmac_sha1.js'); //根据自己存放的路径修改
var client = null;

/* 阿里云MQTT连接参数 */
// const clientId = 'i3c6vjLss9C.Wechat|securemode=2,signmethod=hmacsha256,timestamp=1675995798170|';
// const username = 'Wechat&i3c6vjLss9C';
// const password = 'e1a946abe3597cd2fc7d44bbc564263a8c065010692671234284c0b1ac0a8fee';
//设备身份三元组+区域
const deviceConfig = {
  productKey: "k18q7ceN4JU",
  deviceName: "WXminiprogram",
  deviceSecret: "1ba8de43e9e075ac71a5641f47ca1683",
  regionId: "cn-shanghai"
};
const connectUrl = 'wxs://k18q7ceN4JU.iot-as-mqtt.cn-shanghai.aliyuncs.com';
// const topic1 = `/sys/${deviceConfig.productKey}/${deviceConfig.deviceName}/thing/event/property/post`; // 微信小程序向ESP32发送控制信号
const topic1 = `/k18q7ceN4JU/WXminiprogram/user/get `;

//生成基于HmacSha1的password
function signHmacSha1(params, deviceSecret) {
  let keys = Object.keys(params).sort();
  // 按字典序排序
  keys = keys.sort();
  const list = [];
  keys.map((key) => {
    list.push(`${key}${params[key]}`);
  });
  const contentStr = list.join('');
  return crypto.hex_hmac_sha1(deviceSecret, contentStr);
}

//IoT平台mqtt连接参数初始化
function initMqttOptions(deviceConfig) {
  const params = {
    productKey: deviceConfig.productKey,
    deviceName: deviceConfig.deviceName,
    timestamp: Date.now(),
    clientId: Math.random().toString(36).substr(2),
  }
  //CONNECT参数
  const options = {
    keepalive: 60, //60s
    clean: true, //cleanSession不保持持久会话
    protocolVersion: 4 //MQTT v3.1.1
  }
  //1.生成clientId，username，password
  options.password = signHmacSha1(params, deviceConfig.deviceSecret);
  options.clientId = `${params.clientId}|securemode=2,signmethod=hmacsha1,timestamp=${params.timestamp}|`;
  options.username = `${params.deviceName}&${params.productKey}`;

  return options;
}

function getPostData(num) {
  const payloadJson = {
      id: Date.now(),
        params: {
          control: num
      },
      method: "thing.event.property.post"
  }
  console.log("===postData\n topic=" + topic1)
  console.log(payloadJson)
  return JSON.stringify(payloadJson);
}

Page({
  data: {
    width: 150, // 设置图片的宽度
    height: 150, // 设置图片的高度
    ESP32CAM: '连接图传',
    CamStatus: 0,
    pic: "/images/ESP32CAM.png",
    mqttStatus: 0,
    mqttControl_text: '连接MQTT'
  },
  onLoad(options) {
    // this.doConnect();
  },
  // 图传连接控制
  ESP32CAM_control() {
    if (this.data.CamStatus == 0) {
      this.setData({
        pic: " http://172.20.10.10/mjpeg/1", // 连接ESP32CAM
        CamStatus: 1,
        ESP32CAM: '断开图传'
      })
    } else if (this.data.CamStatus == 1) {
      this.setData({
        pic: "/images/ESP32CAM.png",
        CamStatus: 0,
        ESP32CAM: '连接图传'
      })
    }
  },

  connectMQTT() {
    // const options = {
    //   connectTimeout: 4000, // 超时时间
    //   reconnectPeriod: 1000, // 重连时间间隔
    //   clientId,
    //   username,
    //   password
    // }
    //根据三元组生成mqtt连接参数
    const options = initMqttOptions(deviceConfig);
    console.log(options);
    // 连接MQTT服务器
    client = mqtt.connect(connectUrl, options);
    client.subscribe(topic1);
    // 连接成功后触发的回调函数
    client.on('connect', () => {
      console.log('服务器连接成功')
      wx.showToast({
        title: 'MQTT连接成功',
      })
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
  },

  // 无人船运动控制
  up() {
    console.log('前进')
    client.publish(topic1, getPostData(1), {qos:1});
  },
  left() {
    console.log('左转')
    client.publish(topic1, getPostData(2), {qos:1});
  },
  stop() {
    console.log('停止')
    client.publish(topic1, getPostData(0), {qos:1});
  },
  right() {
    console.log('右转')
    client.publish(topic1, getPostData(3), {qos:1});
  },
  down() {
    console.log('后退')
    client.publish(topic1, getPostData(4),{qos:1});
  },
})