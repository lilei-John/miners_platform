//miners module.js
//create on 2018-05-18 by wjun

//miners join info
var minersJoinInfo = {
    id : '0',
    date : '0',
    type : '0',
    ip : '0',
    mac : '0',
    status : '0',
    version : '0',
    ip_type : '0',
    netmask : '0',
    gateway : '0',
    dns : '0',
    periodic_time : '0',
    pool1_status : '0',
    pool1_addr : '0',
    pool1_miner_addr : '0',
    pool1_password : '0',
    pool2_status : '0',
    pool2_addr : '0',
    pool2_miner_addr : '0',
    pool2_password : '0',
    pool3_status : '0',
    pool3_addr : '0',
    pool3_miner_addr : '0',
    pool3_password : '0',
    number : '0',
    position : '0',
    mhs : '0',
    avg_mhs : '0',
    duration : '0',
};

//leave info
var minersleaveInfo = {
    mac : '0',
    status : '0',
    mhs:'0',
    avg_mhs:'0',
};

var minersTable = 'create table miners(id int not null,date DATETIME not null,type varchar(64) not null,\
ip varchar(32) not null,mac varchar(32) unique not null,status varchar(8) not null,version varchar(64) not null,ip_type varchar(8) not null,\
netmask varchar(32) not null,gateway varchar(32) not null,dns varchar(32) not null,periodic_time int not null,pool1_status varchar(16) not null,pool1_addr varchar(64) not null,\
pool1_miner_addr varchar(64) not null,pool1_password varchar(32) not null,pool2_status varchar(16) not null,pool2_addr varchar(64) not null,pool2_miner_addr varchar(64) not null,pool2_password varchar(32) not null,\
pool3_status varchar(16) not null,pool3_addr varchar(64) not null,pool3_miner_addr varchar(64) not null,pool3_password varchar(32) not null,number varchar(16) not null,position varchar(64) not null,mhs int not null,avg_mhs int not null,duration int not null)';

exports.minersJoinInfo = minersJoinInfo;
exports.minersleaveInfo = minersleaveInfo;
exports.minersTable = minersTable;
