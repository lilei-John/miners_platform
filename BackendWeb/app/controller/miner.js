'use strict';

const Controller = require('egg').Controller;

class MinerController extends Controller {
  async miner() {
    let result = '';
    console.log('miner request');
    const query = this.ctx.query;
    let mac = query.mac
    result = await this.ctx.service.miner.miner(mac);
    let miner = result;
    this.ctx.body = {miner};
  }
  async putMiner() {
    let result = '';
    result = await this.ctx.service.miner.putMiner();
    this.ctx.body = result;
  }
  async patchMiner() {
    let result = '';
    let payload = this.ctx.request.body

    result = await this.ctx.service.miner.patchMiner(payload)
    this.ctx.body = {result}
  }
  async minerMHS() {
    let result = '';
    const query = this.ctx.query
    let mac = query.mac
    let period = query.period
    result = await this.ctx.service.miner.minerMHS(mac, period);
    this.ctx.body = result;
  }
  async minerTemperature() {
    let result = '';
    const query = this.ctx.query
    let mac = query.mac
    let period = query.period
    result = await this.ctx.service.miner.minerTemperature(mac, period)
    this.ctx.body = result
  }
  async deleteMiner() {
    let result = '';
    result = await this.ctx.service.miner.deleteMiner();
    this.ctx.body = result;
  }  
  async upgradeMiner() {
    let result = '';
    result = await this.ctx.service.miner.upgradeMiner();
    this.ctx.body = result;
  }  
  async configMiner() {
    let result = '';
    result = await this.ctx.service.miner.configMiner();
    this.ctx.body = result;
  }
  async rebootMiner() {
    let result = '';
    result = await this.ctx.service.miner.rebootMiner();
    this.ctx.body = result;
  }
  async poweroffMiner() {
    let result = '';
    result = await this.ctx.service.miner.poweroffMiner();
    this.ctx.body = result;
  }
}

module.exports = MinerController;