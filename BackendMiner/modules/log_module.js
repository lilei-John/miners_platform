//log_module.js 
//create on 2018-06-14 by wjun

var winston = require('winston');
require('winston-daily-rotate-file');
var date = require('silly-datetime');

//var debug_swtich = true;//默认打印到控制台

var logLevel = {
  outToConsle : 'true',
  level       : '1',
}

Object.defineProperty(global, '__stack', {
  get: function(){
    var orig = Error.prepareStackTrace;
    Error.prepareStackTrace = function(_, stack){ return stack; };
    var err = new Error;
    Error.captureStackTrace(err, arguments.callee);
    var stack = err.stack;
    Error.prepareStackTrace = orig;
    return stack;
  }
});

Object.defineProperty(global, '__line', {
  get: function(){
    return __stack[1].getLineNumber();
  }
});

var transport_broker = new (winston.transports.DailyRotateFile)({
  filename: '../logs/BackendMiner/BackendMiner-%DATE%.log',
  datePattern: 'YYYY-MM-DD',//split by day
  zippedArchive: false,
  maxSize: '50m',
  maxFiles: '30d',
  
});

var transport_db = new (winston.transports.DailyRotateFile)({
  filename: '../logs/DB/DB-%DATE%.log',
  datePattern: 'YYYY-MM-DD',//split by day
  zippedArchive: false,
  maxSize: '50m',
  maxFiles: '30d',
});

transport_broker.on('rotate', function(oldFilename, newFilename) {
  // do something fun
});

transport_db.on('rotate', function(oldFilename, newFilename) {
  // do something fun
});

//create logger
function BwCreateLogger(module)
{
  var logger;
  if (module == 'broker')
  {
    logger = winston.createLogger({
      transports: [
        transport_broker,
      ]
    });
  }else if (module == 'db'){
    logger = winston.createLogger({
      transports: [
        transport_db,
      ]
    });
  }
  return logger;
}

//logger info output
function BwLoggerOut(logger,msg,level,file,line)
{
  if (logger == undefined || msg == undefined || level == undefined)
    return;

  if (level == 'debug' && logLevel.level >= 4){
    logger.debug(msg, {Time: date.format(new Date(), 'YYYY-MM-DD HH:mm:ss'),File:file,Line:line});
    if (logLevel.outToConsle == true){
      console.log(msg);
    }
  }else if (level == 'info' && logLevel.level  >= 3){
    logger.info(msg, {Time: date.format(new Date(), 'YYYY-MM-DD HH:mm:ss'),File:file,Line:line});
    if (logLevel.outToConsle == true){
      console.log(msg);
    }
  }else if (level == 'warn' && logLevel.level >= 2){
    logger.warn(msg, {Time: date.format(new Date(), 'YYYY-MM-DD HH:mm:ss'),File:file,Line:line});
    if (logLevel.outToConsle == true){
      console.log(msg);
    }
  }else if (level == 'error' && logLevel.level >= 1){
    logger.error(msg, {Time: date.format(new Date(), 'YYYY-MM-DD HH:mm:ss'),File:file,Line:line});
    if (logLevel.outToConsle == true){
      console.log(msg);
    }
  }else if (level == 'none'){}
}

//set logger level
function BwSetLoggerLevel(outToConsle,level)
{
  logLevel.outToConsle = outToConsle;
  logLevel.level = level;
}

exports.BwCreateLogger = BwCreateLogger;
exports.BwLoggerOut = BwLoggerOut;
exports.BwSetLoggerLevel = BwSetLoggerLevel;
