'use strict';

/**
 * @param {Egg.Application} app - egg application
 */
module.exports = app => {
  const { router, controller } = app;
  //router.get('/', controller.home.index);
  app.router.redirect('/', '/public/index.html', 302);
  router.get('/v1/mining', controller.mining.mining);
  router.get('/v1/mining/mhs', controller.mining.miningMHS);


  router.get('/v1/miners', controller.miners.miners);
  router.get('/v1/miners/alerts', controller.miners.minersAlerts)


  router.get('/v1/miner', controller.miner.miner);
  router.put('/v1/miner', controller.miner.putMiner);
  router.patch('/v1/miner', controller.miner.patchMiner);
  router.get('/v1/miner/mhs', controller.miner.minerMHS);
  router.get('/v1/miner/temperature', controller.miner.minerTemperature)

  router.delete('/v1/miner/delete', controller.miner.deleteMiner);
  router.put('/v1/miner/upgrade', controller.miner.upgradeMiner);
  router.put('/v1/miner/config', controller.miner.configMiner);
  router.put('/v1/miner/reboot', controller.miner.rebootMiner);
  router.put('/v1/miner/poweroff', controller.miner.poweroffMiner);

  router.put('/v1/batch/pools', controller.batch.putBatchPools)
  router.put('/v1/batch/reboot', controller.batch.putBatchReboot)
  router.put('/v1/batch/upgrade', controller.batch.putBatchUpgrade)
  router.put('/v1/batch/network', controller.batch.pubBatchNetwork)
  router.put('/v1/batch/fans', controller.batch.pubBatchFans)
  router.put('/v1/batch/threshold', controller.batch.pubBatchThreshold)
  router.put('/v1/batch/frequency', controller.batch.pubBatchFrequency)
  router.put('/v1/batch/period', controller.batch.pubBatchPeiod)

  router.get('/v1/system/version', controller.system.version);
  
  router.get('/v1/system/smtp', controller.system.SMTP);
  router.get('/v1/system/mineName', controller.system.mineName);
  router.put('/v1/system/smtp', controller.system.putSMTP);
  router.put('/v1/system/mineName', controller.system.putMineName);
  
  router.get('/v1/system/logs', controller.system.getLogs)
  router.get('/v1/system/log', controller.system.getLogConfig)
  router.put('/v1/system/log', controller.system.putLogConfig)

  router.get('/v1/system/user', controller.system.user);
  router.put('/v1/system/user', controller.system.putUser);
  router.delete('/v1/system/user', controller.system.deleteUser);
  
  router.put('/v1/system/login', controller.system.login);
  
  router.post('/v1/upload/firmwareImage', controller.upload.postFirmwareImage);

  router.get('/v1/excel/hashrate/all/:type', controller.excel.getHashrateAll);
  router.get('/v1/excel/hashrate/single/:type/:mac', controller.excel.getHashrateSingle);
  router.get('/v1/excel/temperature/:mac', controller.excel.getTemperature);
  router.get('/v1/excel/miners/:type', controller.excel.getMiners);
};
