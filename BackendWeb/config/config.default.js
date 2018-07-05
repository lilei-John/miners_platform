'use strict';

const path = require('path')

module.exports = appInfo => {
  const config = exports = {};

  // use for cookie sign key, should change to your own and keep security
  config.keys = appInfo.name + '_1526888441186_3463';

  // add your config here
  config.middleware = [];

  config.mysql = {
    // ?????ݿ???Ϣ????
    client: {
      // host
      host: 'localhost',
      // host: '192.168.4.191',
      // ?˿ں?
      port: '3306',
      // ?û???
      user: 'bwminer_user',
      // ????
      password: 'bwcon',
      // ???ݿ???
      database: 'bwminerdb',
    },
    // ?Ƿ????ص? app ?ϣ?Ĭ?Ͽ???
    app: true,
    // ?Ƿ????ص? agent ?ϣ?Ĭ?Ϲر?
    agent: false,
  };

  config.multipart = {
    fileSize: '200mb',
  }

  config.mqtt = {
    host: 'mqtt://localhost:1883',
    username: 'admin',
    password: 'admin',
  }

  config.logger = {
    dir: path.join(appInfo.root, '..', 'logs', 'BackendWeb'),
  }

  return config;
};
