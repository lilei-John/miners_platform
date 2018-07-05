const Service = require('egg').Service;

class BatchService extends Service {
  async putBatchPools(payload) {
    let result = '';
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_pools', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    result = await fn()
    return result;
  }
  async putBatchReboot(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_reboot', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async putBatchUpgrade(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_update', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async pubBatchNetwork(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_network', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async pubBatchFans(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_fan_speed', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async pubBatchThreshold(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_threshould', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async pubBatchFrequency(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_frequency', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

  async pubBatchPeiod(payload) {
    let client = this.ctx.app.mqttClient
    console.log(payload)
    let fn = () => {
      return new Promise((resolve, reject) => {
        client.publish('batch_set_periodic_time', JSON.stringify(payload), function(err) {
          if (err) resolve('fail')
          else resolve('success')
        })
      })
    }
    let result = await fn()
    return result
  }

}

module.exports = BatchService;