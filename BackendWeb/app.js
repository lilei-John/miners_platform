const mqtt = require('mqtt')
const fs = require('fs')
const path = require('path')

module.exports = app => {
  app.beforeStart(async () => {
    console.log('app_before_start')
    let configPath = path.join(app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)
    let minerTableName = config.minerTableName
    app.minerTableName = minerTableName

    let opt = {
      username: app.config.mqtt.username,
      password: app.config.mqtt.password,
    }
    app.mqttClient = mqtt.connect(app.config.mqtt.host, opt)
    app.mqttClient.on('connect', function() {
      console.log('>>> mqtt connected');
      app.mqttClient.subscribe('switch_miner_table');
      app.mqttClient.publish('backendweb_connect',JSON.stringify({empty: ''}), function(err) {
      })
    })
    app.mqttClient.on('message', function(topic, message) {
      console.log('topic ' + topic)
      if (topic === 'switch_miner_table') {
        let tableName = message.toString();
        console.log(tableName)
        if (app.minerTableName !== tableName) {
          app.minerTableName = tableName;
          config.minerTableName = tableName;
          fs.writeFileSync(configPath, JSON.stringify(config, null, 2))
        }
        console.log('miner table name ' + app.minerTableName)
      }
    })
  })
}