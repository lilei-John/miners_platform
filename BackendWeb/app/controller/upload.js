const path = require('path')
const fs = require('fs')
const sendToWormhome = require('stream-wormhole')
const awaitWriteSream = require('await-stream-ready').write;
const Controller = require('egg').Controller

class UploadController extends Controller {
  async postFirmwareImage() {
    const ctx = this.ctx;
    const stream = await ctx.getFileStream()
    const savePath = path.join(this.app.baseDir, 'app/public', stream.filename)
    console.log('upload-path: ' + savePath)
    const writeStream = fs.createWriteStream(savePath)
    try {
      await awaitWriteSream(stream.pipe(writeStream))
    } catch (err) {
      await sendToWormhome(stream)
      this.ctx.body = {result: 'fail'}
    }
    this.ctx.body = {result: 'success', url: this.ctx.request.origin + '/public/' + stream.filename}
  }
}
module.exports = UploadController