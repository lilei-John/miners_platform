//config_module.js 
//create on 2018-05-22 by wjun

var cfgFs = require('fs');

var MQTT_BROKER_VERSION = 'Mqtt_broker_v1.1';
var DBIP = '127.0.0.1';
var DBPORT = '3306';
var DBNAME = 'bwminerdb';
var DBUSER = 'bwminer_user';
var DBPWD = 'bwcon';
var TABLE1 = 'mining';
var TABLE2 = 'miners';
var TABLE3 = 'miner';
var CFGPATH = '../BackendWeb/config/config.json';
var MAILFILE = '../BackendWeb/config/mailfile.txt'
var MAILDELAYTIME = 12*60*60*1000

//cfg
var platfromCfg = 
{ 
  name: '',
  smtp: 
   { host: '',
     port: '',
     secure: '',
     auth: 
     {  user: '', 
        pass: '' 
     },
     from: '',
     to: '',
     period : '',
     enable : '',
     timerHandle:''
    },
   log:
   {
     outToConsole : '',
     logLevel : ''
   }
}

//get smtp cfg
function BwGetSmtpCfg()
{
    var cfgData = cfgFs.readFileSync(CFGPATH);
    var jsonCfgData = JSON.parse(cfgData.toString());
    platfromCfg.smtp.enable = jsonCfgData.smtp.enable;
    platfromCfg.smtp.host = jsonCfgData.smtp.host;
    platfromCfg.smtp.port = jsonCfgData.smtp.port;
    platfromCfg.smtp.secure = jsonCfgData.smtp.secure;
    platfromCfg.smtp.auth.user = jsonCfgData.smtp.auth.user;
    platfromCfg.smtp.auth.pass = jsonCfgData.smtp.auth.pass;
    platfromCfg.smtp.from = jsonCfgData.smtp.from;
    platfromCfg.smtp.to = jsonCfgData.smtp.to;
    platfromCfg.smtp.period = jsonCfgData.smtp.period;

    return platfromCfg;
}

function BwGetLogCfg()
{
    var cfgData = cfgFs.readFileSync(CFGPATH);
    var jsonCfgData = JSON.parse(cfgData.toString());
    platfromCfg.log.outToConsole = jsonCfgData.log.outToConsole;
    platfromCfg.log.logLevel = jsonCfgData.log.logLevel;
    
    return platfromCfg;
}
exports.MQTT_BROKER_VERSION = MQTT_BROKER_VERSION;
exports.DBIP = DBIP;
exports.DBPORT = DBPORT;
exports.DBNAME = DBNAME;
exports.DBUSER = DBUSER;
exports.DBPWD = DBPWD;
exports.TABLE1 = TABLE1;
exports.TABLE2 = TABLE2;
exports.TABLE3 = TABLE3;
exports.MAILFILE = MAILFILE;
exports.platfromCfg = platfromCfg;
exports.BwGetSmtpCfg = BwGetSmtpCfg;
exports.BwGetLogCfg = BwGetLogCfg;
exports.MAILDELAYTIME = MAILDELAYTIME;