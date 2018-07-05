'use strict';

const path = require('path')
const fs = require('fs')

const Service = require('egg').Service;

class SystemService extends Service {
  async version() {

    return 'version Service';
  }

  async SMTP() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    return config.smtp;
  }  
  async mineName() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    return config.mineName;
  }

  async getLogConfig() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    return config.log;
  }

  async putLogConfig() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    let body = this.ctx.request.body
    let { outToConsole,logLevel } = body

    config.log.outToConsole = outToConsole
    config.log.logLevel = logLevel

    let res = fs.writeFileSync(configPath, JSON.stringify(config, null, 2))

    let client = this.ctx.app.mqttClient
    client.publish('log_config', JSON.stringify(config, null, 2))
    return { result: 'success', log: config.log}
  }

  async putSMTP() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    let body = this.ctx.request.body
    let {host, port, secure, user, pass, from, to, period, enable} = body

    config.smtp.host = host
    config.smtp.port = port
    config.smtp.secure = secure
    config.smtp.host = host
    config.smtp.from = from
    config.smtp.to = to
    config.smtp.period = period
    config.smtp.enable = enable
    config.smtp.auth.user = user
    config.smtp.auth.pass = pass

    let res = fs.writeFileSync(configPath, JSON.stringify(config, null, 2))

    let client = this.ctx.app.mqttClient    
    client.publish('smtp_config',JSON.stringify({empty: ''}), function(err) {

    })

    return {result: 'success', smtp: config.smtp};
  }

  async putMineName() {
    let configPath = path.join(this.app.baseDir, 'config', 'config.json')
    let configStr = fs.readFileSync(configPath)
    let config = JSON.parse(configStr)

    let body = this.ctx.request.body
    let {mineName} = body

    config.mineName = mineName

    let res = fs.writeFileSync(configPath, JSON.stringify(config, null, 2))

    return {result: 'success', mineName: config.mineName};
  }

  async user() {
    return 'user Service';
  }

  async putUser() {
    return 'user Service';
  }

  async deleteUser() {
    return 'user Service';
  }

  async login({name, password}) {

    return 'login Service';
  }  
}

module.exports = SystemService;