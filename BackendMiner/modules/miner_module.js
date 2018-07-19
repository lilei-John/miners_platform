//miner module.js
//create on 2018-05-21 by wjun

var minerInfo = {
    id : '0',
    date : '0',
    ip : '0',
    mac : '0',
    mhs : '0',
    avg_mhs : '0',
    accept : '0',
    reject : '0',
    diff : '0',
    temperature : '0',
    boards : '0',
    board1_mhs : '0',
    board1_temperate : '0',
    board1_frequency : '0',
    board1_accept : '0',
    board1_reject : '0',
    board2_mhs : '0',
    board2_temperate : '0',
    board2_frequency : '0',
    board2_accept : '0',
    board2_reject : '0',
    board3_mhs : '0',
    board3_temperate : '0',
    board3_frequency : '0',
    board3_accept : '0',
    board3_reject : '0',
    board4_mhs : '0',
    board4_temperate : '0',
    board4_frequency : '0',
    board4_accept : '0',
    board4_reject : '0',
    fans : '0',
    fan1_status : '0',
    fan1_speed : '0',
    fan2_status : '0',
    fan2_speed : '0',
    duration : '0',
    temperature_threshould : '0',
    mhs_threshould : '0',
};

var minerTable = '(id int not null AUTO_INCREMENT,date DATETIME not null,ip varchar(32) not null,\
mac varchar(32) not null,mhs int not null,avg_mhs int not null,accept int not null,reject int not null,diff int not null,temperature int not null,boards int not null,\
board1_mhs int not null,board1_temperate int not null,board1_frequency int not null,board1_accept int not null,board1_reject int not null,board2_mhs int not null,\
board2_temperate int not null,board2_frequency int not null,board2_accept int not null,board2_reject int not null,board3_mhs int not null,board3_temperate int not null,\
board3_frequency int not null,board3_accept int not null,board3_reject int not null,board4_mhs int not null,board4_temperate int not null,board4_frequency int not null,\
board4_accept int not null,board4_reject int not null,fans int not null,fan1_status varchar(8) not null,fan1_speed int not null,fan2_status varchar(8) not null,fan2_speed int not null,\
duration int not null,temperature_threshould int not null,mhs_threshould int not null,PRIMARY KEY (`id`))';

exports.minerInfo = minerInfo;
exports.minerTable = minerTable;
