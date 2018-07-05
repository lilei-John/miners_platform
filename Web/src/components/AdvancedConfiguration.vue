<template>
  <div class="advanced-configuration">
    <div class="common-box" v-loading="loadingLog" element-loading-text="保存中">
      <p class="platform-title">日志</p>
      <div class="log-item-wrap">
        <span class="log-item-name">是否导出到控制台:</span>
        <el-checkbox v-model="outToConsole"></el-checkbox>
      </div>
      <div class="log-item-wrap">
        <span class="log-item-name">日志级别:</span>
        <el-select v-model.trim="logLevel" size="small" style="width: 140px;">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </div>
      <div style="text-align: center; margin-top: 15px;">
        <el-button type="primary" @click="save">保存</el-button>
      </div>
      <div>
        <el-button @click="exportLogs" type="primary">导出</el-button>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AdvancedConfiguration',
  data () {
    return {
      outToConsole: false,
      logLevel: 1,
      options: [1, 2, 3, 4],
      loadingLog: false
    }
  },
  methods: {
    exportLogs () {
      window.open('/v1/system/logs')
    },
    save () {
      let logEdit = {
        outToConsole: this.outToConsole,
        logLevel: this.logLevel
      }

      this.loadingLog = true
      this.$ajax.put('/v1/system/log', {
        outToConsole: logEdit.outToConsole,
        logLevel: logEdit.logLevel
      })
        .then((response) => {
          if (response.data.result === 'success') {
            let log = response.data.log
            this.logLevel = log.logLevel
            this.outToConsole = log.outToConsole
            this.$message({
              message: '保存log配置成功',
              type: 'success'
            })
          } else {
            this.$alert('保存log配置失败')
          }
          this.loadingLog = false
        })
        .catch((error) => {
          this.$alert(error.message)
          this.loadingLog = false
        })
    }
  },
  created () {
    this.$ajax.get(`/v1/system/log`)
      .then((response) => {
        let log = response.data.log
        this.logLevel = log.logLevel
        this.outToConsole = log.outToConsole
      })
      .catch((error) => {
        console.log(error)
        this.$alert('获取log配置失败')
      })
  }
}
</script>

<style>
  .advanced-configuration .common-box {
    padding: 20px;
  }

  .platform-title {
    font-size: 16px;
    padding-bottom: 4px;
    margin-bottom: 10px;
    border-bottom: 1px solid #ebeef5;
  }

  .log-box .el-input,
  .log-box .el-textarea {
    width: 360px;
  }
  .log-item-name {
    display: inline-block;
    width: 300px;
    text-align: left;
  }
  .log-item-wrap:nth-of-type(n+1) {
    margin-top: 10px;
  }
</style>
