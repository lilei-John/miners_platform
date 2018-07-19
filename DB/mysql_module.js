//mysql_module.js 
//create on 2018-05-16 by wjun

var mysql = require('mysql');
var eventsHandle = require('../BackendMiner/modules/events_module');
var logHandle = require('../BackendMiner/modules/log_module');
var BwLoggerOut = logHandle.BwLoggerOut;
var DBLogger;
var DBTableIndexSetHandle;

var mysqlDBInfo = 
{
      host      : '0',
      port      : '0',
      user      : '0',
      password  : '0',
};

//create connection
function BwMysqlCreate(mysqlDBInfo)
{
  var connection = mysql.createConnection(mysqlDBInfo);
  return connection;
}

//connect
function BwMysqlConnect(connection)
{
    connection.connect(function(err){
    if (err){
      BwLoggerOut(DBLogger,'connection '+err,'error',__filename,__line);
      return -1;
    }
    BwLoggerOut(DBLogger,'connection sucess.','info',__filename,__line);
    return 0;
  });
}

//create db
function BwMysqlCreateDB(connection,sql)
{
    connection.query(sql, function(err,result){
        if(err){
            if (err.code == 'ER_DB_CREATE_EXISTS')//ER_DB_CREATE_EXISTS
            {
                BwLoggerOut(DBLogger,'database exists.','warn',__filename,__line);
            }
            else
                throw err
        }else{
            BwLoggerOut(DBLogger,'databases create suecess.','info',__filename,__line);
        }
    });
}

//use db
function BwMysqlUseDB(connection,DB)
{
    connection.query('USE '+DB, function(err,result){
        if(err){
                throw err
        }else{
        BwLoggerOut(DBLogger,'use this databases suecess.','info',__filename,__line);
        }
    });
}
//create table
function BwMysqlCreateTable(connection,sql)
{
    connection.query(sql, function(err,result){
        if(err){
            if (err.code == 'ER_TABLE_EXISTS_ERROR')//ER_TABLE_EXISTS_ERROR
            {
                BwLoggerOut(DBLogger,'table exists.','warn',__filename,__line);
            }
            else
                throw err
        }else{
            BwLoggerOut(DBLogger,'table create suecess.','info',__filename,__line);
        }
    });
}

//create index
function BwMysqlCreateIndex(connection,sql)
{
    connection.query(sql, function(err,result){
        if(err){
            if (err.code == 'ER_DUP_KEYNAME')//ER_DUP_KEYNAME
            {
                BwLoggerOut(DBLogger,'ER_DUP_KEYNAME,index exists.','warn',__filename,__line);
            }
            else
                throw err
        }else{
            BwLoggerOut(DBLogger,'index create suecess.','info',__filename,__line);
        }
    });
}

//query
function BwMysqlQuery(connection,cmd,params,table)
{
    if (cmd == 'insert')
    {
        if (table == 'miners')
        {
            var  Sql = 'INSERT INTO miners(id,date,type,ip,mac,status,version,ip_type,netmask,gateway,dns,periodic_time,pool1_status,\
                        pool1_addr,pool1_miner_addr,pool1_password,pool2_status,pool2_addr,pool2_miner_addr,pool2_password,pool3_status,\
                        pool3_addr,pool3_miner_addr,pool3_password,number,position,mhs,avg_mhs,duration) \
                        VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)';
            var  SqlParams = [params.id,params.date,params.type,params.ip,params.mac,params.status,params.version,params.ip_type,params.netmask,params.gateway,
                            params.dns,params.periodic_time,params.pool1_status,params.pool1_addr,params.pool1_miner_addr,params.pool1_password,params.pool2_status,
                            params.pool2_addr,params.pool2_miner_addr,params.pool2_password,params.pool3_status,params.pool3_addr,params.pool3_miner_addr,params.pool3_password,
                            params.number,params.position,params.mhs,params.avg_mhs,params.duration];
            
        }else if (table != 'miners' && table != 'mining')
        {
            var Sql = 'INSERT INTO '+table+'(id,date,ip,mac,mhs,avg_mhs,accept,reject,diff,temperature,boards,board1_mhs,board1_temperate,board1_frequency,board1_accept,\
                        board1_reject,board2_mhs,board2_temperate,board2_frequency,board2_accept,board2_reject,board3_mhs,board3_temperate,board3_frequency,\
                        board3_accept,board3_reject,board4_mhs,board4_temperate,board4_frequency,board4_accept,board4_reject,fans,fan1_status,fan1_speed,\
                        fan2_status,fan2_speed,duration,temperature_threshould,mhs_threshould)\
                        VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)';
            var  SqlParams = [params.id,params.date,params.ip,params.mac,params.mhs,params.avg_mhs,params.accept,params.reject,params.diff,params.temperature,
                            params.boards,params.board1_mhs,params.board1_temperate,params.board1_frequency,params.board1_accept,params.board1_reject,params.board2_mhs,
                            params.board2_temperate,params.board2_frequency,params.board2_accept,params.board2_reject,params.board3_mhs,params.board3_temperate,
                            params.board3_frequency,params.board3_accept,params.board3_reject,params.board4_mhs,params.board4_temperate,params.board4_frequency,
                            params.board4_accept,params.board4_reject,params.fans,params.fan1_status,params.fan1_speed,params.fan2_status,params.fan2_speed,
                            params.duration,params.temperature_threshould,params.mhs_threshould];
            
        }else if (table == 'mining')
        {
            var Sql = 'INSERT INTO mining(id,date,mhs,normal,abnormal,total,temperature,humidity) VALUES(?,?,?,?,?,?,?,?)';
            var SqlParams = [params.id,params.date,params.mhs,params.normal,params.abnormal,params.total,params.temperature,params.humidity];
        }

        connection.query(Sql,SqlParams, function(error, result){
        if(error){
            BwLoggerOut(DBLogger,error.message +table,'error',__filename,__line);
        }else{
            BwLoggerOut(DBLogger,'insert id: '+result.insertId+table,'info',__filename,__line);
        }
        });
    }else if (cmd == 'update')
    {
        //只有改变了的数据才会上报
        //update miners table
        if (table == 'miners')
        {
            var setstring = '';
            if (params.date != undefined)
                setstring += 'date = ' + '\'' + params.date + '\'';  
            
            if (params.type != undefined)
            {
                if (setstring != '')
                    setstring += ','+'type = ' + '\'' + params.type + '\'';
                else
                    setstring += 'type = ' + '\'' + params.type + '\'';
            }

            if (params.ip != undefined)
            {
                if (setstring != '')
                    setstring += ','+'ip = ' + '\'' + params.ip + '\'';
                else
                    setstring += 'ip = ' + '\'' + params.ip + '\'';
            }
                
    
            // if (params.mac != undefined)
            //     setstring += 'mac = ' + '\'' + params.mac + '\'';
    
            if (params.status != undefined)
            {
                if (setstring != '')
                    setstring += ','+'status = ' + '\'' + params.status + '\'';
                else
                    setstring += 'status = ' + '\'' + params.status + '\'';
            }

            if (params.version != undefined)
            {
                if (setstring != '')
                    setstring += ','+'version = ' + '\'' + params.version + '\'';
                else
                    setstring += 'version = ' + '\'' + params.version + '\'';
            }
 
            if (params.ip_type != undefined)
            {
                if (setstring != '')
                    setstring += ','+'ip_type = ' + '\'' + params.ip_type + '\'';
                else
                    setstring += 'ip_type = ' + '\'' + params.ip_type + '\'';
            }

            if (params.netmask != undefined)
            {
                if (setstring != '')
                    setstring += ','+'netmask = ' + '\'' + params.netmask + '\'';
                else
                    setstring += 'netmask = ' + '\'' + params.netmask + '\'';
            }

            if (params.gateway != undefined)
            {
                if (setstring != '')
                    setstring += ','+'gateway = ' + '\'' + params.gateway + '\'';
                else
                    setstring += 'gateway = ' + '\'' + params.gateway + '\'';
            }

            if (params.dns != undefined)
            {
                if (setstring != '')
                    setstring += ','+'dns = ' + '\'' + params.dns + '\'';
                else
                    setstring += 'dns = ' + '\'' + params.dns + '\'';
            }

            if (params.periodic_time != undefined)
            {
                if (setstring != '')
                    setstring += ','+'periodic_time = ' + '\'' + params.periodic_time + '\'';
                else
                    setstring += 'periodic_time = ' + '\'' + params.periodic_time + '\'';
            }

            if (params.pool1_status != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool1_status = ' + '\'' + params.pool1_status + '\'';
                else
                    setstring += 'pool1_status = ' + '\'' + params.pool1_status + '\'';
            }

            if (params.pool1_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool1_addr = ' + '\'' + params.pool1_addr + '\'';
                else
                    setstring += 'pool1_addr = ' + '\'' + params.pool1_addr + '\'';
            }

            if (params.pool1_miner_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool1_miner_addr = ' + '\'' + params.pool1_miner_addr + '\'';
                else
                    setstring += 'pool1_miner_addr = ' + '\'' + params.pool1_miner_addr + '\'';
            }

            if (params.pool1_password != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool1_password = ' + '\'' + params.pool1_password + '\'';
                else
                    setstring += 'pool1_password = ' + '\'' + params.pool1_password + '\'';
            }

            if (params.pool2_status != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool2_status = ' + '\'' + params.pool2_status + '\'';
                else
                    setstring += 'pool2_status = ' + '\'' + params.pool2_status + '\'';
            }

            if (params.pool2_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool2_addr = ' + '\'' + params.pool2_addr + '\'';
                else
                    setstring += 'pool2_addr = ' + '\'' + params.pool2_addr + '\'';
            }

            if (params.pool2_miner_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool2_miner_addr = ' + '\'' + params.pool2_miner_addr + '\'';
                else
                    setstring += 'pool2_miner_addr = ' + '\'' + params.pool2_miner_addr + '\'';
            }

            if (params.pool2_password != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool2_password = ' + '\'' + params.pool2_password + '\'';
                else
                    setstring += 'pool2_password = ' + '\'' + params.pool2_password + '\'';
            }

            if (params.pool3_status != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool3_status = ' + '\'' + params.pool3_status + '\'';
                else
                    setstring += 'pool3_status = ' + '\'' + params.pool3_status + '\'';
            }

            if (params.pool3_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool3_addr = ' + '\'' + params.pool3_addr + '\'';
                else
                    setstring += 'pool3_addr = ' + '\'' + params.pool3_addr + '\'';
            }

            if (params.pool3_miner_addr != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool3_miner_addr = ' + '\'' + params.pool3_miner_addr + '\'';
                else
                    setstring += 'pool3_miner_addr = ' + '\'' + params.pool3_miner_addr + '\'';
            }

            if (params.pool3_password != undefined)
            {
                if (setstring != '')
                    setstring += ','+'pool3_password = ' + '\'' + params.pool3_password + '\'';
                else
                    setstring += 'pool3_password = ' + '\'' + params.pool3_password + '\'';
            }

            if (params.number != undefined)
            {
                if (setstring != '')
                    setstring += ','+'number = ' + '\'' + params.number + '\'';
                else
                    setstring += 'number = ' + '\'' + params.number + '\'';
            }

            if (params.position != undefined)
            {
                if (setstring != '')
                    setstring += ','+'position = ' + '\'' + params.position + '\'';
                else
                    setstring += 'position = ' + '\'' + params.position + '\'';
            }

            if (params.mhs != undefined)
            {
                if (setstring != '')
                    setstring += ','+'mhs = ' + '\'' + params.mhs + '\'';
                else
                    setstring += 'mhs = ' + '\'' + params.mhs + '\'';
            }

            if (params.avg_mhs != undefined)
            {
                if (setstring != '')
                    setstring += ','+'avg_mhs = ' + '\'' + params.avg_mhs + '\'';
                else
                    setstring += 'avg_mhs = ' + '\'' + params.avg_mhs + '\'';
            }

            if (params.duration != undefined)
            {
                if (setstring != '')
                    setstring += ','+'duration = ' + '\'' + params.duration + '\'';
                else
                    setstring += 'duration = ' + '\'' + params.duration + '\'';
            }
            var Sql = 'UPDATE miners SET '+setstring+' WHERE mac = '+ '\'' + params.mac + '\'';
        }
        //update miner table
        if (table != 'miners' && table != 'mining')
        {
            var setstring = '';

            if (params.mhs != undefined)
            {
                if (setstring != '')
                    setstring += ','+'mhs = ' + '\'' + params.mhs + '\'';
                else
                    setstring += 'mhs = ' + '\'' + params.mhs + '\'';
            }

            if (params.avg_mhs != undefined)
            {
                if (setstring != '')
                    setstring += ','+'avg_mhs = ' + '\'' + params.avg_mhs + '\'';
                else
                    setstring += 'avg_mhs = ' + '\'' + params.avg_mhs + '\'';
            }
            var Sql = 'UPDATE '+table+' SET '+setstring+' WHERE mac = '+ '\'' + params.mac + '\'';
        }

        connection.query(Sql, function(error, result){
        if(error){
            BwLoggerOut(DBLogger,error.message+table,'error',__filename,__line);
        }else{
            BwLoggerOut(DBLogger,'update id: '+result.insertId+table,'info',__filename,__line);
        }
        });
    }else if (cmd == 'select'){
        if (table == 'miners')
        {
            var Sql = 'SELECT * from miners WHERE mac = '+ '\'' + params.mac + '\'';
        } 

        connection.query(Sql, function(error, result){
        if(error){
            BwLoggerOut(DBLogger,error.message+table,'error',__filename,__line);

        }else{
            BwLoggerOut(DBLogger,'select id: '+result.insertId+table,'info',__filename,__line);
        }
        });
    }else if (cmd == 'del'){
        if (table == 'miners')
        {
            var Sql = 'DEL * from miners WHERE mac = '+ '\'' + params.mac + '\'';
        }

        connection.query(Sql, function(error, result){
        if(error){
            BwLoggerOut(DBLogger,error.message+table,'error',__filename,__line);
        }else{
            BwLoggerOut(DBLogger,'del id: '+result.insertId+table,'info',__filename,__line);
        }
        });
    }
}

//disconnect
function BwMysqlDisConnect(connection)
{
  return connection.end();
}

//set log module
function BwMysqlLogSet(logger)
{
  if (logger != undefined)
    DBLogger = logger;
}

exports.mysqlDBInfo = mysqlDBInfo;
exports.BwMysqlCreate = BwMysqlCreate;
exports.BwMysqlConnect = BwMysqlConnect;
exports.BwMysqlCreateDB = BwMysqlCreateDB;
exports.BwMysqlUseDB = BwMysqlUseDB;
exports.BwMysqlCreateTable = BwMysqlCreateTable;
exports.BwMysqlQuery = BwMysqlQuery;
exports.BwMysqlDisConnect = BwMysqlDisConnect;
exports.BwMysqlLogSet = BwMysqlLogSet;
exports.BwMysqlCreateIndex = BwMysqlCreateIndex;
exports.DBTableIndexSetHandle = DBTableIndexSetHandle;