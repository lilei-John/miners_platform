<template>
<div class="batch-control-container">
  <miner-list class="mb" @addIps="addSelectedIps" :export-visible="false"></miner-list>
  <el-row class="common-box mb" style="padding: 20px;">
    <ip-range ref="ipRange"></ip-range>
    <div v-loading="loading" element-loading-text="发送请求中">
    <div style="padding-top: 20px;">
        <el-button type="primary" @click="reboot">批量重启</el-button>
        <el-button v-if="false" type="primary" @click="shutdown">批量关机</el-button>
      </div>
    </div>
  </el-row>

</div>
</template>

<script>
import IpRange from '@/components/IpRange'
import MinerList from '@/components/MinerList'

export default {
  name: 'BatchControl',
  components: {
    IpRange,
    MinerList
  },
  data () {
    return {
      loading: false
    }
  },
  computed: {
    checkedIps () {
      return this.$refs.ipRange.checkedIps
    }
  },
  methods: {
    addSelectedIps (ipList) {
      this.$refs.ipRange.addSelectedIps(ipList)
    },
    reboot () {
      if (this.checkedIps.length === 0) {
        this.$alert('请添加待配置矿机', '提示')
        return
      }
      this.$confirm('此操作将按选定待配置矿机 批量重启，是否继续？', '提示').then(() => {
        console.log('reboot')
        this.loading = true
        this.$ajax.put('/v1/batch/reboot', {
          ips: this.checkedIps
        })
          .then((response) => {
            this.loading = false
            if (response.data.result === 'success') {
              this.$alert('已发送批量重启请求')
            } else {
              this.$alert('发送批量重启请求失败')
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
    shutdown () {
      if (this.checkedIps.length === 0) {
        this.$alert('请添加待配置矿机', '提示')
        return
      }
      this.$confirm('此操作将按选定待配置矿机 批量关机，是否继续？，是否继续？', '提示').then(() => {
        console.log('shutdown')
        this.loading = true
        this.$ajax.put('/v1/batch/shutdown', {
          ips: this.checkedIps
        })
          .then((response) => {
            this.loading = false
            if (response.data.result === 'success') {
              this.$alert('已发送批量关机请求')
            } else {
              this.$alert('发送批量关机请求失败')
            }
          })
          .catch((error) => {
            this.loading = false
            this.$alert(error.message)
          })
      }).catch(() => {
        // 取消
      })
    }
  }
}
</script>

<style>

</style>
