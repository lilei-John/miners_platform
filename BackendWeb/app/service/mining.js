'use strict';

const Service = require('egg').Service;
const Util = require('../util/util.js')

class MiningService extends Service {
  async mining() {    
    const results = await this.app.mysql.query('select * from mining order by date desc limit 0, 1');        
    
    var results_json = JSON.stringify(results);            
    var mining = JSON.parse(results_json);    
    return { mining };
  }

  async miningMHS(query) {
    var query_string = 'select date,mhs from mining where date ';
    if (!query.period) {
      query.period = 'day'
    }

    query_string += `between DATE_SUB(NOW(), INTERVAL ${Util.dbPeriodMap[query.period]}) and NOW() order by date`;    

    const results = await this.app.mysql.query(query_string);

    let mining_mhs = []
    if (results && results.length > 100) { //最多取100条
      mining_mhs = Util.pick(results, 100)
    } else {
      mining_mhs = results;
    }

    return { mining_mhs };
  }
}

module.exports = MiningService;