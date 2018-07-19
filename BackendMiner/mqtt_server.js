//mqtt_server.js
//create on 2018-05-15 by wjun

var os = require('os');
var mosca = require('mosca');
var dateTime = require('silly-datetime');
var validator = require('validator');
var ws = require('nodejs-websocket');
var smtpFile = require('fs');
var jsonHandle = require('./modules/json_module');
var mysqlHandle = require('../DB/mysql_module');
var eventsHandle = require('./modules/events_module');
var minersHandle = require('./modules/miners_module');
var minerHandle = require('./modules/miner_module');
var miningHandle = require('./modules/mining_module');
var configHandle = require('./modules/config_module');
var logHandle = require('./modules/log_module');
var smtpHandle = require('./modules/smtp_module');

var broker_logger = logHandle.BwCreateLogger('broker');
var db_logger = logHandle.BwCreateLogger('db');
var BwLoggerOut = logHandle.BwLoggerOut;

var dbminers = mysqlHandle.mysqlDBInfo;
dbminers.host = configHandle.DBIP;
dbminers.port = configHandle.DBPORT;
dbminers.user = configHandle.DBUSER;
dbminers.password = configHandle.DBPWD;

var g_miner_table;

var interfaces = require('os').networkInterfaces();
for(var devName in interfaces){
	  var iface = interfaces[devName];
	  for(var i=0;i<iface.length;i++){
		   var alias = iface[i];
		   if(alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal){
			   BwLoggerOut(broker_logger,'local addr '+alias.address,'info',__filename,__line);
			   dbminers.host = alias.address;
		   }
	  }
}

var connection = mysqlHandle.BwMysqlCreate(dbminers);
var eventEmitter = eventsHandle.BwEventsEmitterCreate();

var miningInfo = miningHandle.miningInfo;
var totoalMiners = configHandle.TOTOAL_MINERS;
var minersMhsMap = new Map();
var minersUpdateProgressMap = new Map();
var webconn;
var Smtper;
var platfromCfg;
var smtpMap = new Map();
var logCfg;

var minersMhsInfo = {
	mac :'0',
	ip : '0',
	mhs : '0',
	timestamp : '0',
	status : '0',
	errno : '0',
	//errno:
	/*0:ok 1:offline 2:error 3:low mhs 4:zero mhs 5:ip conflict 6:fan1 stopped*/
	/*7:fan1_error 8:fan2_stopped 9:fan2_error  10: temp warm and so on*/
};

var updateProgressInfo = {
	mac : '0',
	progress : '0',
};

//operate mysql cmds
eventsHandle.BwEventsEmitterOn('insert',eventEmitter);
eventsHandle.BwEventsEmitterOn('update',eventEmitter);
eventsHandle.BwEventsEmitterOn('del',eventEmitter);
eventsHandle.BwEventsEmitterOn('select',eventEmitter);
eventsHandle.BwEventsLogSet(broker_logger);

var ascoltatore = {
  //using ascoltatore
  //type: 'mongo',
  //url: 'mongodb://localhost:27017/mqtt',
  //pubsubCollection: 'ascoltatori',
  //mongo: {}
};

var settings = {
	port: 1883,
	backend: ascoltatore
};

function isValidMacAddress(MacAddress)
{
	var c = '';
	var i = 0, j = 0;
  
	if ((MacAddress.toLowerCase() == 'ff:ff:ff:ff:ff:ff') || (MacAddress.toLowerCase() == '00:00:00:00:00:00')) {
		return false;
	}
  
	var addrParts = MacAddress.split(':');
	if (addrParts.length != 6) {
		 return false;
	}

	for (i = 0; i < 6; i++)
	{
		 if (addrParts[i] == ''){
			  return false;
		 }
		 if (addrParts[i].length != 2)
		 { 
			return false;
		 }
		 for (j = 0; j < addrParts[i].length; j++)
		 {
			c = addrParts[i].toLowerCase().charAt(j);
			if ((c >= '0' && c <= '9') || (c >= 'a' && c <='f')) {
				continue;
			} else {
				return false;
			}
		 }
	}
	return true;
}

//create broker server
var server = new mosca.Server(settings);


//client connect
server.on('clientConnected', function(client) {
		BwLoggerOut(broker_logger,'client connected '+client.id,'info',__filename,__line);
});

//client disconnect
server.on('clientDisconnected', function(client) {
		BwLoggerOut(broker_logger,'client disconnected '+client.id,'info',__filename,__line);
		//miners leave
		//id协商为矿机MAC地址,当矿机离线后设置Mysql
		var minersleaveInfo = minersHandle.minersleaveInfo;
		minersleaveInfo.mac = client.id;
		minersleaveInfo.status = 'offline';
		minersleaveInfo.mhs = '0';
		minersleaveInfo.avg_mhs = '0';
		
		var isMac = isValidMacAddress(minersleaveInfo.mac);
		//emit 异步方式
		if (isMac == true){
			eventsHandle.BwEventsEmit('update',eventEmitter,connection,minersleaveInfo,'miners');
			eventsHandle.BwEventsEmit('update',eventEmitter,connection,minersleaveInfo,g_miner_table);
			var minersMhsInfo = {mac:'',ip:'',mhs:'',timestamp:'',status:'',errno:''};
			minersMhsInfo.mac = minersleaveInfo.mac;
			minersMhsInfo.status = 0;
			minersMhsInfo.mhs = 0;
			minersMhsInfo.errno = 1;
			minersMhsInfo.timestamp = dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			for (var value of minersMhsMap.values()) {
				if (value.mac != undefined && value.mac == minersMhsInfo.mac){
					minersMhsInfo.ip = value.ip;
				}
			}
			minersMhsMap.set(minersleaveInfo.mac,minersMhsInfo);
		}
});

//fired when a message is received
server.on('published', function(packet, client) {
		var topic = packet.topic;		
		var payload = packet.payload.toString();
		BwLoggerOut(broker_logger,'topic is  payload is '+topic + packet.payload.toString(),'info',__filename,__line);
		if (!validator.isJSON(packet.payload.toString())){
			BwLoggerOut(broker_logger,'payload json format error. '+packet.payload.toString(),'error',__filename,__line);
			return;
		}

		if (topic == 'join')//miners join
		{
			//miners info
			var minersJoinInfo = minersHandle.minersJoinInfo;
			minersJoinInfo = jsonHandle.BwJsonParse(packet.payload.toString());//string-->json
			minersJoinInfo.date=dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');

			//emit 异步方式
			eventsHandle.BwEventsEmit('update',eventEmitter,connection,minersJoinInfo,'miners');
			eventsHandle.BwEventsEmit('insert',eventEmitter,connection,minersJoinInfo,'miners');
			
			var minersMhsInfo = {mac:'',ip:'',mhs:'',timestamp:'',status:'',errno:''};
			minersMhsInfo.mac = minersJoinInfo.mac;
			minersMhsInfo.ip = minersJoinInfo.ip;
			minersMhsInfo.timestamp = dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			minersMhsInfo.status = 1;
			minersMhsInfo.errno = 0;
			minersMhsInfo.mhs = 0;
			minersMhsMap.set(minersJoinInfo.mac,minersMhsInfo);			

		}else if (topic == 'miners normal publish' || topic == 'miners periodic publish' || topic == 'miners alert'|| topic == 'miners leave')
		{
			//miners info
			var minersJoinInfo = minersHandle.minersJoinInfo;
			minersJoinInfo = jsonHandle.BwJsonParse(packet.payload.toString());//string-->json
			minersJoinInfo.date=dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');

			//emit 异步方式
			eventsHandle.BwEventsEmit('update',eventEmitter,connection,minersJoinInfo,'miners');

			var minersMhsInfo = {mac:'',ip:'',mhs:'',timestamp:'',status:'',errno:''};
			if (topic == 'miners alert')
			{
				minersMhsInfo.status = 2;

				if (minersJoinInfo.status == 'error')
					minersMhsInfo.errno = 2;
				else if (minersJoinInfo.status == 'low mhs')
					minersMhsInfo.errno = 3;
				else if (minersJoinInfo.status == 'zero mhs')
					minersMhsInfo.errno = 4;
				else if (minersJoinInfo.status == 'ip conflict')
					minersMhsInfo.errno = 5;
				else
					minersMhsInfo.errno = -1;//unkown error
			}else{
				minersMhsInfo.status = 1;
				minersMhsInfo.errno = 0;
			}
			minersMhsInfo.mac = minersJoinInfo.mac;
			minersMhsInfo.ip = minersJoinInfo.ip;
			minersMhsInfo.timestamp = dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			minersMhsInfo.mhs = minersJoinInfo.mhs;
			minersMhsMap.set(minersJoinInfo.mac,minersMhsInfo);

		}else if (topic == 'miner normal publish' || topic == 'miner periodic publish' || topic == 'miner alert')
		{
			//miner info
			var minerInfo = minerHandle.minerInfo;
			minerInfo = jsonHandle.BwJsonParse(packet.payload.toString());//string-->json
			minerInfo.date=dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');

			//emit 异步方式
			eventsHandle.BwEventsEmit('insert',eventEmitter,connection,minerInfo,g_miner_table);
			
			//将矿机上报的算力/状态实时保存在map中
			var minersMhsInfo = {mac:'',ip:'',mhs:'',timestamp:'',status:'',errno:''};
			if (topic == 'miner alert')
			{	
				minersMhsInfo.status = 2;

				if (parseInt(minerInfo.mhs) <= parseInt(minerInfo.mhs_threshould))
					minersMhsInfo.errno = 3;
				else if (parseInt(minerInfo.mhs) == 0)
					minersMhsInfo.errno = 4;
				else if (minerInfo.fan1_status == 'stopped')
					minersMhsInfo.errno = 6;
				else if (minerInfo.fan1_status == 'error')
					minersMhsInfo.errno = 7;
				else if (minerInfo.fan2_status == 'stopped')
					minersMhsInfo.errno = 8;
				else if	(minerInfo.fan2_status == 'error')
					minersMhsInfo.errno = 9;
				else if (parseInt(minerInfo.temperature) >= parseInt(minerInfo.temperature_threshould))
					minersMhsInfo.errno = 10;
				else
					minersMhsInfo.errno = -1;//unkown error			
			}
			else{
				minersMhsInfo.status = 1;
				minersMhsInfo.errno = 0;
			}
				
			minersMhsInfo.mac = minerInfo.mac;
			minersMhsInfo.ip = minerInfo.ip;
			minersMhsInfo.timestamp = dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			minersMhsInfo.mhs = minerInfo.mhs;
			minersMhsMap.set(minerInfo.mac,minersMhsInfo);
		}else if (topic == 'update_progress'){
			//mac + progress
			updateProgressInfo = jsonHandle.BwJsonParse(packet.payload.toString());
			minersUpdateProgressMap.set(updateProgressInfo.mac,updateProgressInfo);
			BwLoggerOut(broker_logger,'updateProgressInfo '+updateProgressInfo.mac,updateProgressInfo,'info',__filename,__line);
		}else if (topic == 'smtp_config'){
			//配置smtp
			BwLoggerOut(broker_logger,'Recevie Smtp Cfg Cmd','info',__filename,__line);
			platfromCfg = configHandle.BwGetSmtpCfg();
			BwLoggerOut(broker_logger,'smtpCfg: '+platfromCfg.smtp.host+platfromCfg.smtp.port+platfromCfg.smtp.secure+platfromCfg.smtp.auth.user+platfromCfg.smtp.auth.pass+platfromCfg.smtp.from+platfromCfg.smtp.to+platfromCfg.smtp.period+platfromCfg.smtp.enable,'info',__filename,__line);
			if (platfromCfg.smtp.enable == true){
				if (platfromCfg.smtp.timerHandle != undefined)
					clearInterval(platfromCfg.smtp.timerHandle);
				Smtper = smtpHandle.BwSmtpCreate(platfromCfg.smtp.host,platfromCfg.smtp.port,platfromCfg.smtp.secure,platfromCfg.smtp.auth.user,platfromCfg.smtp.auth.pass,platfromCfg.smtp.from,platfromCfg.smtp.to);
				platfromCfg.smtp.timerHandle = setInterval(SmtpProcess,platfromCfg.smtp.period * 60 * 1000);//邮件发送间隔
			}else if (platfromCfg.smtp.enable == false){
				if (platfromCfg.smtp.timerHandle != undefined)
					clearInterval(platfromCfg.smtp.timerHandle);
			}		
		}else if (topic == 'log_config'){
			//配置log
			platfromCfg = configHandle.BwGetLogCfg();
			logHandle.BwSetLoggerLevel(platfromCfg.log.outToConsole,platfromCfg.log.logLevel);
		}else if (topic == 'backendweb_connect'){
			//web后台连接,上报当前miner分表
			BwLoggerOut(broker_logger,'backend web connected','error',__filename,__line);
			connection.query('show tables', function(error, result){
				if (result){
					var miner_table_name;
					var miner_table_num = result.length - 2;
					if (parseInt(miner_table_num) == 1){
						miner_table_name = 'miner';
						g_miner_table = miner_table_name;
					}
					else if (parseInt(miner_table_num) > 1){
						miner_table_name = 'miner'+parseInt(miner_table_num-1);
						g_miner_table = miner_table_name;
					}
					var msg = {
						topic: 'switch_miner_table',
						payload: miner_table_name,
						qos: 1,
						retain: true
					};
					//publish msg to backendweb
					server.publish(msg, function () {
						BwLoggerOut(broker_logger,'publish msg to backendweb,msg: '+miner_table_name,'error',__filename,__line);
					});
				}
			});
		}
});

//若用户名与密码有效，接受链接
var authenticate = function(client, username, password, callback) {
	var authorized = (username == 'admin' && password.toString() == 'admin');
	if (authorized) client.user = username;
	callback(null, authorized);
}

//server ready
server.on('ready', setup);

//fired when the mqtt server is ready
function setup() {	

		BwLoggerOut(broker_logger,'Mqtt broker version '+configHandle.MQTT_BROKER_VERSION,'info',__filename,__line);
		BwLoggerOut(broker_logger,'Mosca server is up and running','info',__filename,__line);
		BwLoggerOut(broker_logger,'start create db and tables','info',__filename,__line);

		var sql_db = "CREATE DATABASE "+configHandle.DBNAME;
		var sql_mining_table = miningHandle.miningTable;
		var sql_miners_table = minersHandle.minersTable;
		var sql_miner_table = minerHandle.minerTable;

		mysqlHandle.BwMysqlCreateDB(connection,sql_db);
		mysqlHandle.BwMysqlUseDB(connection,configHandle.DBNAME);
		mysqlHandle.BwMysqlCreateTable(connection,sql_mining_table);
		mysqlHandle.BwMysqlCreateTable(connection,sql_miners_table);
		mysqlHandle.BwMysqlCreateTable(connection,'create table miner'+sql_miner_table);
		mysqlHandle.DBTableIndexSetHandle = setInterval(createTableIndex,100)//100ms
		mysqlHandle.BwMysqlLogSet(db_logger);

		connection.query('show tables', function(error, result){
			if (result){
				var miner_table_name;
				var miner_table_num = result.length - 2;
	
				if (parseInt(miner_table_num) == 1){
					miner_table_name = 'miner';
					g_miner_table = miner_table_name;
				}
				else if (parseInt(miner_table_num) > 1){
					miner_table_name = 'miner'+parseInt(miner_table_num-1);
					g_miner_table = miner_table_name;
				}
				BwLoggerOut(broker_logger,'current miner table is '+g_miner_table,'error',__filename,__line);
			}
		});

		platfromCfg = configHandle.BwGetSmtpCfg();
		BwLoggerOut(broker_logger,'smtpCfg: '+platfromCfg.smtp.host+platfromCfg.smtp.port+platfromCfg.smtp.secure+platfromCfg.smtp.auth.user+platfromCfg.smtp.auth.pass+platfromCfg.smtp.from+platfromCfg.smtp.to+platfromCfg.smtp.period+platfromCfg.smtp.enable,'info',__filename,__line);
		if (platfromCfg.smtp.enable == true){
			Smtper = smtpHandle.BwSmtpCreate(platfromCfg.smtp.host,platfromCfg.smtp.port,platfromCfg.smtp.secure,platfromCfg.smtp.auth.user,platfromCfg.smtp.auth.pass,platfromCfg.smtp.from,platfromCfg.smtp.to);
			platfromCfg.smtp.timerHandle = setInterval(SmtpProcess,platfromCfg.smtp.period * 60 * 1000);//邮件发送间隔
		}

		platfromCfg = configHandle.BwGetLogCfg();
		logHandle.BwSetLoggerLevel(platfromCfg.log.outToConsole,platfromCfg.log.logLevel);

		server.authenticate = authenticate;

		//getMinintTableInfo();
		
		setInterval(updateMiningTable,15*1000,connection);//15s
		
		createWebsocketServer();
		setInterval(WebsocketServerProcess,1*1000);//1s

		setInterval(SelectMinerTableCount,60*1000);//1min
}

//create table index after create table
function createTableIndex()
{
	mysqlHandle.BwMysqlCreateIndex(connection,'CREATE INDEX mining_index_id ON mining (date)');
	mysqlHandle.BwMysqlCreateIndex(connection,'CREATE INDEX miners_index_id ON miners (mac)');
	mysqlHandle.BwMysqlCreateIndex(connection,'CREATE INDEX miner_index_id ON miner (mac,date)');
	if (mysqlHandle.DBTableIndexSetHandle != undefined)
		clearInterval(mysqlHandle.DBTableIndexSetHandle);
}
//get minint table info
function getMinintTableInfo()
{
	//paas刚上线时统计开机前离线的,待矿机上线后再更新
	var Sql = 'select count(*) from miners where status='+ '\''+ 'offline'+'\'';
	console.log('sql =  ',Sql);
	connection.query(Sql, function(error, result){
		if (result){
			miningInfo.mhs = 0;
			miningInfo.date=dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			miningInfo.normal = 0;//正常矿机（只包含正常矿机）
			miningInfo.abnormal = result.length;//异常矿机（包含告警/离线矿机）,PAAS刚起来时统计
			miningInfo.total = 0;//在线矿机（包含正常/告警矿机）
			BwLoggerOut(broker_logger,'mining totoal mhs '+0+' normal miners '+miningInfo.normal+' abnormal miners '+miningInfo.abnormal+' total miners '+miningInfo.total,'info',__filename,__line);
			//emit 异步方式
			eventsHandle.BwEventsEmit('insert',eventEmitter,connection,miningInfo,'mining');
		}
	});
}
//update mining table
function updateMiningTable(connection)
{
	//mining info
	miningInfo.temperature = '37';
	miningInfo.humidity = '10';

	//遍历map统计总算力/正常矿机/异常矿机
	var totalMhs = 0;
	var totalActiveMinerNum = 0;
	var totalAbnormalMinerNUm = 0;
	var totalLeaveMinerNum = 0;

	for (var value of minersMhsMap.values()) {
		var status = value.status;
		var mhs = value.mhs;
		if (status == 1){
			totalMhs = parseInt(mhs) + parseInt(totalMhs);
			totalActiveMinerNum++;
		}else if(status == 2){
			totalMhs = parseInt(mhs) + parseInt(totalMhs);
			totalAbnormalMinerNUm++;
		}else if (status == 0){
			totalLeaveMinerNum++;
		}
	}

	miningInfo.mhs = parseInt(totalMhs);
	miningInfo.date=dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
	miningInfo.normal = parseInt(totalActiveMinerNum);//正常矿机（只包含正常矿机）
	miningInfo.abnormal = parseInt(totalAbnormalMinerNUm)+parseInt(totalLeaveMinerNum);//异常矿机（包含告警/离线矿机）
	miningInfo.total = parseInt(totalActiveMinerNum) + parseInt(totalAbnormalMinerNUm);//在线矿机（包含正常/告警矿机）
	BwLoggerOut(broker_logger,'mining totoal mhs '+totalMhs+' normal miners '+miningInfo.normal+' abnormal miners '+miningInfo.abnormal+' total miners '+miningInfo.total,'info',__filename,__line);
	//emit 异步方式
	eventsHandle.BwEventsEmit('insert',eventEmitter,connection,miningInfo,'mining');
}


//create websocket server
function  createWebsocketServer()
{
	var webserver = ws.createServer(function (conn) {
		BwLoggerOut(broker_logger,'new web client Connection','info',__filename,__line);
		webconn = conn;
	}).listen(1884)
}

//process websocket server
function WebsocketServerProcess()
{
	if (webconn != undefined){
		for (var value of minersUpdateProgressMap.values()) {
			var progress = value.progress;
			var mac = value.mac;
			if (parseInt(progress) !=0 && progress != undefined){
				BwLoggerOut(broker_logger,'updating mac '+mac+' progress '+progress+'%','info',__filename,__line);
				webconn.sendText(mac+'~'+progress);
				if (parseInt(progress) == 100){
					BwLoggerOut(broker_logger,'client: '+mac+' update sucess!',__filename,__line);
					value.progress = 0;
					minersUpdateProgressMap.set(mac,value);
				}
			}
		}
	}
}

//SmtpProcess
function SmtpProcess()
{
	var mailMsg = '';
	for (var value of minersMhsMap.values()) {
		var status = value.status;
		var errno = value.errno;
		var mac = value.mac;
		var ip = value.ip;
		var time = value.timestamp;
		var flag = 0;

		if (status == 1){//running
		}else if(status == 2){//alert
			for (var value of smtpMap.values()) {
				//if the alert miners have already send mail with same erro,then send mail after 12 hours.
				if (value.mac == mac && value.errno == errno && (Date.now() - Date.parse(new Date(time))) <= configHandle.MAILDELAYTIME){
					flag = 1;
					BwLoggerOut(broker_logger,'IP: '+ip+' MAC: '+mac+' ERRNO: '+errno+' have already send mail,pls wait '+configHandle.MAILDELAYTIME,'warn',__filename,__line);	
				} else if (value.mac == mac && value.errno == errno && (Date.now() - Date.parse(new Date(time))) > configHandle.MAILDELAYTIME){
					flag = 2;
				}
			}
			if (0 == flag){
				var smtpMapInfo = {mac:'',errno:''};
				smtpMapInfo.mac = mac;
				smtpMapInfo.errno = errno;
				smtpMap.set(smtpMapInfo.mac,smtpMapInfo);
				var msg = 'IP: '+ip+' MAC: '+mac+' ERRNO: '+errno+' DATE: '+time+'\n';
				mailMsg += msg;
			}
		}else if (status == 0){//leave
			for (var value of smtpMap.values()) {
				//if the alert miners have already send mail with same erro,then send mail after 12 hours.
				if (value.mac == mac && value.errno == errno && (Date.now() - Date.parse(new Date(time))) <= configHandle.MAILDELAYTIME){
					flag = 1;
					BwLoggerOut(broker_logger,'IP: '+ip+' MAC: '+mac+' ERRNO: '+errno+' have already send mail,pls wait '+configHandle.MAILDELAYTIME,'warn',__filename,__line);	
				} else if (value.mac == mac && value.errno == errno && (Date.now() - Date.parse(new Date(time))) > configHandle.MAILDELAYTIME){
					flag = 2;
				}
			}
			if (0 == flag){
				var smtpMapInfo = {mac:'',errno:''};
				smtpMapInfo.mac = mac;
				smtpMapInfo.errno = errno;
				smtpMap.set(smtpMapInfo.mac,smtpMapInfo);			
				var msg = 'IP: '+ip+' MAC: '+mac+' ERRNO: '+errno+' DATE: '+time+'\n';
				mailMsg += msg;
			}
		}
		//wait time > mail_delay_time ,recyle errno time
		if (flag == 2){
			var minersMhsInfo = {mac:'',ip:'',mhs:'',timestamp:'',status:'',errno:''};
			minersMhsInfo.mac = mac;
			minersMhsInfo.ip = ip;
			minersMhsInfo.timestamp = dateTime.format(new Date(), 'YYYY-MM-DD HH:mm:ss');
			minersMhsInfo.status = status;
			minersMhsInfo.errno = errno;
			minersMhsInfo.mhs = value.mhs;
			minersMhsMap.set(mac,minersMhsInfo);
		}
	}
	if (mailMsg != undefined && mailMsg != ''){
		BwLoggerOut(broker_logger,mailMsg,'info',__filename,__line);
		smtpFile.open(configHandle.MAILFILE,"w",0644,function(e,fd){
		try{
			let errno_string = 'Warning Description'+'\n';
			errno_string += '1:offline'+'\n';
			errno_string += '2:error'+'\n';
			errno_string += '3:low mhs'+'\n';
			errno_string += '4:zero mhs'+'\n';
			errno_string += '5:ip conflict'+'\n';
			errno_string += '6:fan1 stopped'+'\n';
			errno_string += '7:fan1_error'+'\n';
			errno_string += '8:fan2_stopped'+'\n';
			errno_string += '9:fan2_error'+'\n';
			errno_string += '10:temp warm'+'\n\n';
			smtpFile.write(fd,errno_string+mailMsg,0,'utf8',function(e){
				smtpFile.closeSync(fd);
			})
		} catch(E){
			throw e;
		}
		});
		smtpHandle.BwSmtpSendMail(Smtper,platfromCfg.smtp.from,platfromCfg.smtp.to,mailMsg);
	}
}

//SelectMinerTableCount 查询Miner表大小,当大小超过百万级时,miner需分表,否则paas查询矿机详情时会很卡.
//miner -> miner1 -> miner2 -> miner3......
function SelectMinerTableCount()
{
	connection.query('show tables', function(error, result){
		if (result){
			var Sql;
			var miner_table_name;
			var miner_table_num = result.length - 2;
			var sql_miner_table = minerHandle.minerTable;

			if (miner_table_num == 1){
				miner_table_name = 'miner';
				Sql = 'select count(*) from miner';
				g_miner_table = miner_table_name;
			}
			else if (miner_table_num > 1){
				miner_table_name = 'miner'+parseInt(miner_table_num-1);
				Sql = 'select count(*) from miner'+parseInt(miner_table_num-1);
				g_miner_table = miner_table_name;
			}
			BwLoggerOut(db_logger,'current miner table is '+g_miner_table,'error',__filename,__line);
			connection.query(Sql, function(error, result){
				if(error){
					BwLoggerOut(db_logger,'select sql: '+Sql+' err msg: '+error.message,'error',__filename,__line);
		
				}else{
					BwLoggerOut(db_logger,'select sql: '+Sql+' length: '+result[0]["count(*)"],'error',__filename,__line);
				}

				if (result){
					if (result[0]["count(*)"] >= 100*10*1000){
						BwLoggerOut(db_logger,'select table counts larger than 100*10*1000,need to switch tables '+' select sql: '+Sql,'error',__filename,__line);
						if (miner_table_num >= 1){
							miner_table_name = 'miner'+parseInt(miner_table_num);
							g_miner_table = miner_table_name;
						}
						BwLoggerOut(db_logger,'switch miner table ','create table '+miner_table_name+sql_miner_table,'error',__filename,__line);
						mysqlHandle.BwMysqlCreateTable(connection,'create table '+miner_table_name+sql_miner_table);
						var table_index = 'CREATE INDEX miner_index_id ON '+miner_table_name+' (mac,date)';
						mysqlHandle.BwMysqlCreateIndex(connection,table_index);
						var msg = {
							topic: 'switch_miner_table',
							payload: miner_table_name,
							qos: 1,
							retain: true
						};
						//publish msg to backendweb
						server.publish(msg, function () {
							BwLoggerOut(db_logger,'publish msg to backendweb,msg: '+miner_table_name,'error',__filename,__line);
							});
						}
					}
			});
		}
	});
}