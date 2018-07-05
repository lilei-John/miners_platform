const Service = require('egg').Service;
const Util = require('../util/util.js')

class MinerService extends Service {
  async miner(macAddress) {
    let minerName = this.app.minerTableName
    let lastIndex = Number(minerName.replace('miner', '') || 0)

    for (let i = lastIndex; i > 0; i--) {
      minerName = ' miner' + (i == 0 ? '':i)
      let query_string = `select count(mac) from ${minerName} where mac = ? `; 
      let results1 = await this.app.mysql.query(query_string, [macAddress]);
      console.log(results1)        
      if (results1['count(mac)'] !== 0) {
        console.log('into count(mac)')
        break
      } 
    }

    let query_string = 'select * from ' + minerName + ' where mac = ? order by date desc limit 1'; 
    console.log('miner sql string: ' + query_string)
    let results1 = await this.app.mysql.query(query_string, [macAddress]);        
    
    let query_string2 = 'select * from miners where mac = ? order by date desc limit 1'
    let results2 = await this.app.mysql.query(query_string2, [macAddress])

    let miner = []

    let minerInfo1 = {}
    let minerInfo2 = {}
    if (results1 && results1.length) {
      minerInfo1 = results1[0]
    }
    if (results2 && results2.length) {
      minerInfo2 = results2[0]
    }
    miner = Object.assign({}, minerInfo1, minerInfo2)
    // console.log(miner);
    return miner;
  }
  async putMiner(macAddress) {
    return 'save miner Service';
  }
  async patchMiner(payload) {
    let result = '';
    let mac = payload.mac
    let patchAction = payload.patchAction
    let client = this.ctx.app.mqttClient
    console.log(payload)
    if (patchAction && patchAction === 'setNetwork') {
      let data = {
        patchAction,
        mac,
        ip_type: payload.ip_type,
        ip: payload.ip,
        netmask: payload.netmask,
        gateway: payload.gateway,
        dns: payload.dns,
      }
      let fn = () => {
        return new Promise((resolve, reject) => {
          client.publish('set_network', JSON.stringify(data), function(err) {
            if (err) {
              resolve('fail')
            } else {
              resolve('success')
            }
          });
        })
      }

      result = await fn()
      console.log(result)
      console.log('---> data', JSON.stringify(data))
    } else if (patchAction && patchAction === 'setPosition') {
      let position = payload.position
      
      let fn = () => {
        return new Promise((resolve, reject) => {
          client.publish('set_position', JSON.stringify(payload), function(err) {
            if (err) {
              resolve('fail')
            } else {
              resolve('success')
            }
          })
        })
      }
      let fnResult = await fn()
      if (fnResult === 'success') {
        result = await this.app.mysql.query('update miners set position = ? where mac = ?', [position, mac])
      } else {
        result = fnResult
      }
    } else if (patchAction && patchAction === "setNumber") {
      let number = payload.number
      
      let fn = () => {
        return new Promise((resolve, reject) => {
          client.publish('set_number', JSON.stringify(payload), function(err) {
            if (err) {
              resolve('fail')
            } else {
              resolve('success')
            }
          })
        })
      }
      let fnResult = await fn()
      if (fnResult === 'success') {
        result = await this.app.mysql.query('update miners set number = ? where mac = ?', [number, mac])
      } else {
        result = fnResult
      }
    } else if (patchAction && patchAction === "setPool") {
      let fn = () => {
        return new Promise((resolve, reject) => {
          client.publish('set_pools', JSON.stringify(payload), function(err) {
            if (err) {
              resolve('fail')
            } else {
              resolve('success')
            }
          })
        })
      }
      result = await fn()
      console.log(result)
      console.log('---> data', JSON.stringify(payload))
    }
    console.log(new Date())
    return result;
  }
  async minerMHS(macAddress, period = 'day') {
    let minerName = this.app.minerTableName
    let lastIndex = Number(minerName.replace('miner', '') || 0)

    let query_string = ''
    for (let i = 0; i <= lastIndex; i++) {
      let minerName = ' miner' + (i == 0 ? '' : i)
      query_string += ' select date,mhs from ' + minerName + ' where mac = ? and date ';
      query_string += ` between DATE_SUB(NOW(), INTERVAL ${Util.dbPeriodMap[period]}) and NOW() `;
      if (lastIndex > 0 && i != lastIndex) {
        query_string += ' union '
      }
    }
    query_string += ' order by date '

    let params = Array(lastIndex + 1).fill(macAddress)

    const results = await this.app.mysql.query(query_string, params);
    console.log('minerMHS length: ' + results.length)
    let miner_mhs = []
    if (results && results.length > 100) { //最多取100条
      miner_mhs = Util.pick(results, 100)
    } else {
      miner_mhs = results;
    }

    return { miner_mhs };
  }
  async minerTemperature(macAddress, period = 'day') {
    let minerName = this.app.minerTableName
    let lastIndex = Number(minerName.replace('miner', '') || 0)
    console.log('lastIndex ' + lastIndex)
    let queryString = ''
    for (let i =0; i <= lastIndex; i++) {
      let minerName = ' miner' + (i == 0 ? '' : i)
      queryString += ' select date,temperature from ' + minerName + ' where mac = ? and date ';
      queryString += ` between DATE_SUB(NOW(), INTERVAL ${Util.dbPeriodMap[period]}) and NOW() `;
      if (lastIndex > 0 && i != lastIndex) {
        queryString += ' union '
      }
    }
    queryString += ' order by date '
    console.log(queryString)
    let params = Array(lastIndex + 1).fill(macAddress)
    
    const results = await this.app.mysql.query(queryString, params)
    console.log('minerTemperature length: ' + results.length)

    let miner_temperature = []
    if (results && results.length > 100) {
      miner_temperature = Util.pick(results, 100)
    } else {
      miner_temperature = results
    }

    return {
      miner_temperature,
    }
  }

  async deleteMiner({macAddress}) {

    return 'deleteMiner Service';
  } 
  async upgradeMiner({macAddress, version, firmwarePath}) {

    return 'upgradeMiner Service';
  }  
  async configMiner({macAddress, configFilePath}) {

    return 'configMiner Service';
  }
  async rebootMiner(macAddress) {

    return 'rebootMiner Service';
  }
  async poweroffMiner(macAddress) {

    return 'poweroffMiner Service';
  }      
}

module.exports = MinerService;