<template>
<div class="batch-upgrade-container">
  <miner-list class="mb" @addIps="addSelectedIps" :showProgress="true" :percentageList="percentageList" :export-visible="false"></miner-list>
  <el-row class="common-box mb" style="padding: 20px;">
    <ip-range ref="ipRange"></ip-range>
    <div v-loading="loading" element-loading-text="发送请求中">
      <div style="padding-top: 20px;">
        <div class="mb">
          <div class="upload-title">固件升级文件</div>
          <el-row type="flex">
            <div style="width: 200px; margin-right: 4px;">
              <el-input v-model.trim="fileName" readonly="ture"></el-input>
              <p class="clr-danger" v-text="tip"></p>
            </div>
            <el-col>
              <el-upload
                class="upload-demo"
                ref="upload"
                action="/v1/upload/firmwareImage"
                :headers="headers"
                :limit="2"
                :on-preview="handlePreview"
                :on-remove="handleRemove"
                :on-success="uploadSuccess"
                :before-upload="beforeUpload"
                :file-list="fileList"
                :auto-upload="true" style="display:inline-block;">
                <el-button slot="trigger" type="primary">选择固件文件</el-button>
                <el-button type="success" @click="upgrade">批量升级</el-button>
              </el-upload>
            </el-col>
          </el-row>
        </div>
      </div>
    </div>
  </el-row>
</div>
</template>

<script>
import IpRange from '@/components/IpRange'
import MinerList from '@/components/MinerList'

export default {
  name: 'BatchUpgrade',
  components: {
    IpRange,
    MinerList
  },
  data () {
    return {
      loading: false,
      fileList: [],
      fileUploadedUrl: '',
      percentageList: [],
      fileName: '',
      tip: ''
    }
  },
  computed: {
    checkedIps () {
      return this.$refs.ipRange.checkedIps
    },
    headers () {
      let csrfToken = this.$cookies.get('csrfToken')
      return {
        'x-csrf-token': csrfToken
      }
    }
  },
  methods: {
    submitUpload () {
      this.$refs.upload.submit()
    },
    handleRemove (file, fileList) {
      console.log('remove')
      if (fileList.length >= 1) {
        fileList.shift()
      }
      this.fileUploadedUrl = ''
      this.fileName = ''
    },
    handlePreview (file) {
      console.log(file)
    },
    beforeUpload (file) {
      console.log(file.name)
      let reg = /^FSFW.*[.]tar[.]gz$/
      if (!reg.test(file.name)) {
        this.tip = '文件名错误！正确格式应以FSFW开头 .tar.gz结尾!'
        return false
      } else {
        this.tip = ''
      }
    },
    uploadSuccess (response, file, fileList) {
      if (response && response.result === 'success') {
        this.fileUploadedUrl = response.url
      } else {
        this.$alert('上传失败！', '提示')
      }
      console.log(fileList)
      if (fileList.length >= 2) {
        fileList.shift()
      }
      this.fileName = file.name
    },
    addSelectedIps (ipList) {
      this.$refs.ipRange.addSelectedIps(ipList)
    },
    upgrade () {
      if (this.checkedIps.length === 0) {
        this.$alert('请添加待配置矿机', '提示')
        return
      }
      if (!this.fileUploadedUrl) {
        this.$alert('请上传固件升级文件', '提示')
        return
      }
      this.$confirm('此操作将按选定待配置矿机 批量升级，是否继续？', '提示').then(() => {
        console.log('upgrade')
        this.loading = true
        this.$ajax.put('/v1/batch/upgrade', {
          ips: this.checkedIps,
          url: this.fileUploadedUrl
        })
          .then((response) => {
            this.loading = false
            if (response.data.result === 'success') {
              this.$alert('已发送批量升级请求')
              this.upgradeProgress()
            } else {
              this.$alert('发送批量升级请求失败')
            }
          })
          .catch((error) => {
            this.loading = false
            this.$alert(error.message)
          })
      }).catch(() => {
        // 取消
      })
    },
    upgradeProgress () {
      let vm = this
      this.percentageList = []
      let onOpen = function () {
        console.log('Socket opened.')
        socket.send('Hi, Server!')
      }
      let onClose = function () {
        console.log('Socket closed.')
      }
      let onMessage = function (data) {
        console.log('We get signal:')
        vm.addPercentage(data.data)
      }
      let onError = function () {
        console.log('We got an error.')
      }
      let hostname = document.location.hostname
      let socket = new WebSocket('ws://' + hostname + ':1884/')
      socket.onopen = onOpen
      socket.onclose = onClose
      socket.onerror = onError
      socket.onmessage = onMessage
    },
    addPercentage (str) {
      let mac = str.split('~')[0] + '~'
      for (let i = 0; i < this.percentageList.length; i++) {
        let item = this.percentageList[i]
        if (item.indexOf(mac) !== -1) {
          this.percentageList.splice(i, 1, str)
          return
        }
      }
      this.percentageList.push(str)
    }
  }
}
</script>

<style>
  .upload-title {
    font-size: 16px;
    padding-bottom: 4px;
    margin-bottom: 10px;
    border-bottom: 1px solid #ebeef5;
  }
</style>
