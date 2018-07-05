<template>
<div class="platform-configuration">
  <div class="common-box mb" v-loading="loadingMineName" element-loading-text="保存中">
    <p class="platform-title">矿场名字</p>
    <el-input placeholder="请输入矿场名字" v-model.trim="mineNameCopy" style="width:300px;"></el-input>
    <!-- <el-button style="margin-left: 20px;" size="medium" @click="cancelName">取消</el-button> -->
    <el-button style="margin-left: 20px;" size="medium" type="primary" @click="saveName">保存</el-button>
  </div>
  <div class="common-box smtp-box" v-loading="loadingSmtp" element-loading-text="保存中">
    <p class="platform-title">SMTP配置</p>
    <div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">enable:</span>
        <el-checkbox v-model="smtpCopy.enable"></el-checkbox>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">host:</span>
        <el-input size="small" v-model.trim="smtpCopy.host" placeholder="请输入host"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">port:</span>
        <el-input size="small" v-model.trim="smtpCopy.port" placeholder="请输入port"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">secure:</span>
        <el-input size="small" v-model.trim="smtpCopy.secure" placeholder="请输入secure"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">auth username:</span>
        <el-input size="small" v-model.trim="smtpCopy.auth.user" placeholder="请输入auth username"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">auth password:</span>
        <el-input size="small" v-model.trim="smtpCopy.auth.pass" placeholder="请输入auth password"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">from</span>
        <el-input size="small" v-model.trim="smtpCopy.from" placeholder="请输入from"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name  --to">to</span>
        <el-input type="textarea" v-model.trim="smtpCopy.to" size="small" placeholder="请输入to 多个邮件地址以逗号隔开"></el-input>
      </div>
      <div class="smtp-item-wrap">
        <span class="smtp-item-name">send interval(minutes)</span>
        <el-input v-model.trim="smtpCopy.period" type="number" size="small"></el-input>
      </div>
    </div>
    <div style="margin-top: 10px; text-align: center;">
      <!-- <el-button size="medium" @click="cancelSmtp">取消</el-button> -->
      <el-button size="medium" type="primary" @click="saveSmtp">保存</el-button>
    </div>
  </div>
</div>
</template>

<script>
export default {
  name: 'PlatformConfiguration',
  data () {
    return {
      mineName: '',
      mineNameCopy: '',
      smtpOrigin: {
        host: '',
        port: '',
        secure: '',
        auth: {
          user: '',
          pass: ''
        },
        from: '',
        to: '',
        period: '',
        enable: false
      },
      smtpCopy: {
        host: '',
        port: '',
        secure: '',
        auth: {
          user: '',
          pass: ''
        },
        from: '',
        to: '',
        period: '',
        enable: false
      },
      loadingSmtp: false,
      loadingMineName: false
    }
  },
  methods: {
    cancelName () {
      this.mineNameCopy = this.mineName
    },
    saveName () {
      this.loadingMineName = true
      this.$ajax.put('/v1/system/mineName', {
        mineName: this.mineNameCopy
      })
        .then((response) => {
          if (response.data.result === 'success') {
            this.mineName = response.data.mineName
            this.mineNameCopy = this.mineName
            this.$message({
              message: '保存矿场名字成功',
              type: 'success'
            })
            document.getElementsByTagName('title')[0].innerHTML = this.mineName + '管理系统'
          } else {
            this.$alert('保存矿场名字失败')
          }
          this.loadingMineName = false
        })
        .catch((error) => {
          this.$alert(error.message)
          this.loadingMineName = false
        })
    },
    saveSmtp () {
      let {host, port, secure, auth, from, to, period, enable} = this.smtpCopy
      this.loadingSmtp = true
      this.$ajax.put('/v1/system/smtp', {
        host,
        port,
        secure,
        user: auth.user,
        pass: auth.pass,
        from,
        to,
        period,
        enable
      })
        .then((response) => {
          if (response.data.result === 'success') {
            this.smtpOrigin = response.data.smtp
            this.smtpCopy = JSON.parse(JSON.stringify(this.smtpOrigin))
            this.$message({
              message: '保存smtp配置成功',
              type: 'success'
            })
          } else {
            this.$alert('保存smtp配置失败')
          }
          this.loadingSmtp = false
        })
        .catch((error) => {
          this.$alert(error.message)
          this.loadingSmtp = false
        })
    },
    cancelSmtp () {
      this.smtpCopy = JSON.parse(JSON.stringify(this.smtpOrigin))
    }
  },
  created () {
    this.$ajax.get(`/v1/system/smtp`)
      .then((response) => {
        this.smtpOrigin = response.data.smtp
        this.smtpCopy = JSON.parse(JSON.stringify(this.smtpOrigin))
      })
      .catch((error) => {
        console.log(error)
        this.$alert('获取smtp配置失败')
      })

    this.$ajax.get(`/v1/system/mineName`)
      .then((response) => {
        this.mineName = response.data.mineName
        this.mineNameCopy = this.mineName
      })
      .catch((error) => {
        console.log(error)
        this.$alert('获取矿场名字失败')
      })
  }
}
</script>

<style>
  .platform-configuration .common-box {
    padding: 20px;
  }
  .platform-title {
    font-size: 16px;
    padding-bottom: 4px;
    margin-bottom: 10px;
    border-bottom: 1px solid #ebeef5;
  }
  .smtp-box .el-input,
  .smtp-box .el-textarea {
    width: 360px;
  }
  .smtp-item-name {
    display: inline-block;
    width: 300px;
    text-align: left;
  }
  .smtp-item-name.--to {
    height: 30px;
    line-height: 30px;
    vertical-align: top;
  }
  .smtp-item-wrap:nth-of-type(n+1) {
    margin-top: 10px;
  }
</style>
