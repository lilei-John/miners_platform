'use strict';

const Controller = require('egg').Controller;

class MinersController extends Controller {
  async miners() {
    console.log('miners request');
    const query = this.ctx.query;
    let result = '';
    result = await this.ctx.service.miners.miners(query);
    this.ctx.body = result;
  }
  async minersAlerts() {
    console.log('minersAlerts request');
    const query = this.ctx.query;
    let result = '';
    result = await this.ctx.service.miners.minersAlerts(query);
    this.ctx.body = result;
  }  
}

module.exports = MinersController;