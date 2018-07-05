'use strict';
const path = require('path')
const compressing = require('compressing')
const Controller = require('egg').Controller;

class SystemController extends Controller {
  async version() {
    let result = '';
    result = await this.ctx.service.system.version();
    this.ctx.body = result;
  }
  async getLogs() {
    this.ctx.attachment('logs.tar');
    this.ctx.set('Content-Type', 'application/x-tar');
    
    let logsPath = path.join(this.app.baseDir, '..', 'logs')

    const tarStream = new compressing.tar.Stream()
    tarStream.addEntry(logsPath)

    this.ctx.body = tarStream
  }

  async getLogConfig() {
    let log = '';
    log = await this.ctx.service.system.getLogConfig()
    this.ctx.body = { result: 'success', log }
  }

  async putLogConfig() {
    let result = ''
    result = await this.ctx.service.system.putLogConfig();
    this.ctx.body = result;
  }

  async SMTP() {
    let smtp = '';
    smtp = await this.ctx.service.system.SMTP();
    this.ctx.body = {result: 'success', smtp};
  }
  async putSMTP() {
    let result = '';
    result = await this.ctx.service.system.putSMTP();
    this.ctx.body = result;
  }
  async mineName() {
    let mineName = '';
    mineName = await this.ctx.service.system.mineName();
    this.ctx.body = {result: 'success', mineName};
  }
  async putMineName() {
    let result = '';
    result = await this.ctx.service.system.putMineName();
    this.ctx.body = result;
  }
  async user() {
    let result = '';
    result = await this.ctx.service.system.user();
    this.ctx.body = result;
  }
  async putUser() {
    let result = '';
    result = await this.ctx.service.system.putUser();
    this.ctx.body = result;
  }  
  async deleteUser() {
    let result = '';
    result = await this.ctx.service.system.deleteUser();
    this.ctx.body = result;
  }    
  async login() {
    let result = '';
    result = await this.ctx.service.system.login();
    this.ctx.body = result;
  }
}

module.exports = SystemController;