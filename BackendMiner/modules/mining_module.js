//mining module.js
//create on 2018-05-21 by wjun

var miningInfo = {
    id : '0',
    date : '0',
    mhs : '0',
    normal : '0',
    abnormal : '0',
    total : '0',
    temperature : '0',
    humidity : '0',
};

var miningTable = 'create table mining(id int not null AUTO_INCREMENT,date DATETIME not null,mhs int not null,normal int not null,abnormal int not null,total int not null,temperature int not null,humidity int not null,PRIMARY KEY (`id`))';

exports.miningInfo = miningInfo;
exports.miningTable = miningTable;