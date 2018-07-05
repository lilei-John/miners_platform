//events_module.js 
//create on 2018-05-17 by wjun

var events = require('events');
var mysqlHandle = require('../../DB/mysql_module');
var logHandle = require('./log_module');
var BwLoggerOut = logHandle.BwLoggerOut;
var EventsLogger;

//create
function BwEventsEmitterCreate()
{
  var eventEmitter = new events.EventEmitter();
  return eventEmitter;
}

//callback
var EventsMysqlInsert = function insert(connection,params,table) {
  BwLoggerOut(EventsLogger,'recevied insert cmd','info',__filename,__line);
  mysqlHandle.BwMysqlQuery(connection,'insert',params,table);
}

var EventsMysqlUpdate = function update(connection,params,table) {
  BwLoggerOut(EventsLogger,'recevied update cmd','info',__filename,__line);
  mysqlHandle.BwMysqlQuery(connection,'update',params,table);
}

var EventsMysqlSelect = function select(connection,params,table) {
  BwLoggerOut(EventsLogger,'recevied select cmd','info',__filename,__line);
  return mysqlHandle.BwMysqlQuery(connection,'select',params,table);
}

var EventsMysqlDel = function del(connection,params,table) {
  BwLoggerOut(EventsLogger,'recevied del cmd','info',__filename,__line);
  mysqlHandle.BwMysqlQuery(connection,'del',params,table);
}

//bind
function BwEventsEmitterOn(cmd,eventEmitter)
{
  if (cmd == 'insert')
    eventEmitter.on(cmd, EventsMysqlInsert);    
  else if (cmd == 'update')
    eventEmitter.on(cmd, EventsMysqlUpdate);   
  else if (cmd == 'del')
    eventEmitter.on(cmd, EventsMysqlDel);
  else if (cmd == 'select')
    eventEmitter.on(cmd,EventsMysqlSelect)
  BwLoggerOut(EventsLogger,'bind cmd '+cmd,'info',__filename,__line);
}

//emit
function BwEventsEmit(cmd,eventEmitter,connection,params,table)
{
    BwLoggerOut(EventsLogger,'emit cmd '+cmd,'info',__filename,__line);
    eventEmitter.emit(cmd,connection,params,table);
}

//set log module
function BwEventsLogSet(logger)
{
  if (logger != undefined)
    EventsLogger = logger;
}

exports.BwEventsEmitterCreate = BwEventsEmitterCreate;
exports.BwEventsEmitterOn = BwEventsEmitterOn;
exports.BwEventsEmit = BwEventsEmit;
exports.BwEventsLogSet = BwEventsLogSet;
