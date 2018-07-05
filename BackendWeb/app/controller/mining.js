'use strict';

const Controller = require('egg').Controller;

class MiningController extends Controller {
  async mining() {
    let result = '';
    result = await this.ctx.service.mining.mining();
    this.ctx.body = result;
  }

  async miningMHS() {
    console.log('miningMHS request');
    const query = this.ctx.query;
    let result = '';    
    result = await this.ctx.service.mining.miningMHS(query);
    this.ctx.body = result;
  }
}

module.exports = MiningController;