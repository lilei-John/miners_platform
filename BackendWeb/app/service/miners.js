const Service = require('egg').Service;

const searchKeys = ['ip', 'position', 'number', 'status', 'version']

class MinersService extends Service {
  async miners(query) {    
    var query_string = 'select * from miners where 1=1 ';
    let queryTotalString = 'select count(*) from miners where 1=1 '

    for (let i = 0; i < searchKeys.length; i++) {
      let key = searchKeys[i];
      if (query[key]) {
        let str = 'and ' + key + ' like "%' + query[key] + '%" '
        query_string += str
        queryTotalString += str
      }
    }
    if (query.mhsg) {
      let str= ` and avg_mhs > ${query.mhsg} `
      query_string += str
      queryTotalString += str
    }
    if (query.mhsl) {
      let str = ` and avg_mhs < ${query.mhsl} `
      query_string += str
      queryTotalString += str
    }

    query_string += 'order by date desc'

    if((query.offset)&&(query.size)) {
      query_string += ' limit '+query.offset+' , '+query.size;
    }    

    const total_result = await this.app.mysql.query(queryTotalString);
    console.log(total_result);
    var total = total_result[0]["count(*)"];
    
    const results = await this.app.mysql.query(query_string);

    var miners = results;
    return {miners, total};
  }

  async minersAlerts(query) {
    var query_string = 'select * from miners where status <> "running" ';
    let queryTotalString = 'select count(*) from miners where status <> "running" '

    for (let i = 0; i < searchKeys.length; i++) {
      let key = searchKeys[i];
      if (query[key]) {
        let str = 'and ' + key + ' like "%' + query[key] + '%" '
        query_string += str
        queryTotalString += str
      }
    }
    if (query.mhsg) {
      let str= ` and avg_mhs > ${Number(query.mhsg) * 1000 * 1000 / 1024 /1024} `
      query_string += str
      queryTotalString += str
    }
    if (query.mhsl) {
      let str = ` and avg_mhs < ${query.mhsl * 1000 * 1000 / 1024 /1024} `
      query_string += str
      queryTotalString += str
    }
    query_string += 'order by date desc'
    
    if((query.offset)&&(query.size)) {
      query_string += ' limit '+query.offset+' , '+query.size;
    }

    const total_result = await this.app.mysql.query(queryTotalString);
    console.log(total_result);
    var total = total_result[0]["count(*)"];

    const results = await this.app.mysql.query(query_string);         
    var miners_alerts = results
    return {miners_alerts, total};        
  }  
}

module.exports = MinersService;