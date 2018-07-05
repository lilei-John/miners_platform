const Controller = require('egg').Controller

class BatchController extends Controller {
  async putBatchPools() {
    let result = ''
    let payload = this.ctx.request.body
    result = await this.ctx.service.batch.putBatchPools(payload)
    this.ctx.body = {result}
  }
  async putBatchReboot() {
    let payload = this.ctx.request.body
    let result = await this.ctx.service.batch.putBatchReboot(payload)
    this.ctx.body = {result}
  }
  async putBatchUpgrade() {
    let result = await this.ctx.service.batch.putBatchUpgrade(this.ctx.request.body)
    this.ctx.body = {result}
  }
  async pubBatchNetwork() {
    let result = await this.ctx.service.batch.pubBatchNetwork(this.ctx.request.body)
    this.ctx.body = {result}
  }
  async pubBatchFans() {
    let result = await this.ctx.service.batch.pubBatchFans(this.ctx.request.body)
    this.ctx.body = {result}
  }
  async pubBatchThreshold() {
    let result = await this.ctx.service.batch.pubBatchThreshold(this.ctx.request.body)
    this.ctx.body = {result}
  }
  async pubBatchFrequency() {
    let result = await this.ctx.service.batch.pubBatchFrequency(this.ctx.request.body)
    this.ctx.body = {result}
  }
  async pubBatchPeiod() {
    let result = await this.ctx.service.batch.pubBatchPeiod(this.ctx.request.body)
    this.ctx.body = {result}
  }
  
}
module.exports = BatchController