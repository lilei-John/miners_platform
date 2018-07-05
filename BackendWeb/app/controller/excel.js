const fs = require('fs')
const Controller = require('egg').Controller
const xlsx = require('xlsx')
const Util = require('../util/util.js')


class ExcelController extends Controller {
  async getHashrateAll() {
    let period = this.ctx.params.type

    let query = {
      period,
    }

    let data = await this.ctx.service.mining.miningMHS(query);
    
    let mhsList = data.mining_mhs.map(item => {
      return [Util.formatDate(item.date), Util.formatHashrate(item.mhs * 1024 * 1024).text]
    })
    
    let mySheetName = 'hashrate_' + period

    let xlsxContentType = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'

    this.ctx.attachment(mySheetName + '.xlsx');
    this.ctx.set('Content-Type', xlsxContentType);
    
    let sheetDatas = [['时间', '算力']].concat(mhsList)

    let ws = xlsx.utils.aoa_to_sheet(sheetDatas)
    ws['!cols'] = []
    ws['!cols'].push({
      wpx: 180,
    },{
      wpx: 180,
    })
    let wb = xlsx.utils.book_new()
    xlsx.utils.book_append_sheet(wb, ws, mySheetName)

    let buf = xlsx.write(wb, {type: 'buffer', bookType: 'xlsx'})

    this.ctx.body = buf
  }

  async getHashrateSingle() {
    let period = this.ctx.params.type
    let mac = this.ctx.params.mac

    let data = await this.ctx.service.miner.minerMHS(mac, period);
    
    let mhsList = data.miner_mhs.map(item => {
      return [Util.formatDate(item.date), Util.formatHashrate(item.mhs * 1024 * 1024).text]
    })
    
    let mySheetName = 'hashrate_' + period

    let xlsxContentType = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'

    this.ctx.attachment(mySheetName + '.xlsx');
    this.ctx.set('Content-Type', xlsxContentType);
    
    let sheetDatas = [['时间', '算力']].concat(mhsList)

    let ws = xlsx.utils.aoa_to_sheet(sheetDatas)
    ws['!cols'] = []
    ws['!cols'].push({
      wpx: 180,
    },{
      wpx: 180,
    })
    let wb = xlsx.utils.book_new()
    xlsx.utils.book_append_sheet(wb, ws, mySheetName)

    let buf = xlsx.write(wb, {type: 'buffer', bookType: 'xlsx'})

    this.ctx.body = buf
  }

  async getTemperature() {
    let mac = this.ctx.params.mac

    let data = await this.ctx.service.miner.minerTemperature(mac);
    
    console.log(data.miner_temperature)

    let tempList = data.miner_temperature.map(item => {
      return [Util.formatDate(item.date), item.temperature]
    })
    
    let mySheetName = 'temperature'

    let xlsxContentType = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'

    this.ctx.attachment(mySheetName + '.xlsx');
    this.ctx.set('Content-Type', xlsxContentType);
    
    let sheetDatas = [['时间', '温度(℃)']].concat(tempList)

    let ws = xlsx.utils.aoa_to_sheet(sheetDatas)
    ws['!cols'] = []
    ws['!cols'].push({
      wpx: 180,
    })
    let wb = xlsx.utils.book_new()
    xlsx.utils.book_append_sheet(wb, ws, mySheetName)

    let buf = xlsx.write(wb, {type: 'buffer', bookType: 'xlsx'})

    this.ctx.body = buf
  }

  async getMiners() {
    let type = this.ctx.params.type

    let name = 'alerts'
    let sheetName = '告警矿机'
    var query_string = 'select ip, mac, type, version, status, number, position, mhs,avg_mhs,duration, date from miners where status <> "running" ';

    if (type === 'all') {
      name = 'all'
      query_string = 'select ip, mac, type, version, status, number, position, mhs,avg_mhs,duration, date from miners'
      sheetName = '所有矿机'
    } 

    let minersData = await this.app.mysql.query(query_string);
    let miners = minersData.map(item => {
      return [item.ip, item.mac, item.type, item.version, item.status, item.number, item.position, 
        Util.formatHashrate(item.mhs * 1024 * 1024).text, 
        Util.formatHashrate(item.avg_mhs).text, 
        Util.formatDuration(item.duration), 
        Util.formatDate(item.date)]
    })

    console.log(miners)

    let xlsxContentType = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'

    this.ctx.attachment(name + '.xlsx');
    this.ctx.set('Content-Type', xlsxContentType);

    let sheetDatas = [
      [
        'ip', 'mac', '型号', '版本', '状态', '设备条码', '位置', '算力', '平均算力', '运行时间', 'date',  
      ],
    ].concat(miners)

    let ws = xlsx.utils.aoa_to_sheet(sheetDatas)
    ws['!cols'] = [
      { wpx: 100 }, //A
      { wpx: 120 },
      { wpx: 60 }, //C
      { wpx: 100 },      
      { wpx: 65 }, //E  
      { wpx: 65 },    
      { wpx: 65 }, //G
      { wpx: 86 },    
      { wpx: 86 }, //I
      { wpx: 110 }, 
      { wpx: 156 }, //k
       
        
    ]
    let wb = xlsx.utils.book_new()
    xlsx.utils.book_append_sheet(wb, ws, sheetName)
    let buf = xlsx.write(wb, {type: 'buffer', bookType: 'xlsx'})

    this.ctx.body = buf
    }
  }

module.exports = ExcelController