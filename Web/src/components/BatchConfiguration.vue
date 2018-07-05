<template>
<div class="batch-container">
  <miner-list class="mb" @addIps="addSelectedIps" :export-visible="false"></miner-list>
  <el-row type="flex" class="common-box mb">
    <ip-range ref="ipRange"></ip-range>
  </el-row>
  <el-tabs type="border-card" v-loading="loading" element-loading-text="发送请求中">
    <el-tab-pane label="矿池">
      <batch-pools-table :option="batchPoolTableOption" @save="saveBatchPools"></batch-pools-table>
    </el-tab-pane>
    <el-tab-pane label="网络">
      <batch-network-table @save="saveBatchNetwork"></batch-network-table>
    </el-tab-pane>
    <el-tab-pane v-if="false" label="风扇">
      <batch-fan-table @save="saveBatchFan"></batch-fan-table>
    </el-tab-pane>
    <el-tab-pane label="告警门限">
      <batch-threshold @save="saveBatchThreshold"></batch-threshold>
    </el-tab-pane>
    <!-- <el-tab-pane label="工作频率">
      <batch-frequency @save="saveBatchFrequency"></batch-frequency>
    </el-tab-pane> -->
    <el-tab-pane label="上报周期">
      <batch-period @save="saveBatchPeriod"></batch-period>
    </el-tab-pane>
  </el-tabs>
</div>
</template>

<script>
import MinerList from '@/components/MinerList'
import IpRange from '@/components/IpRange'
import BatchPoolsTable from '@/components/BatchPoolsTable.vue'
import BatchNetworkTable from '@/components/BatchNetworkTable.vue'
import BatchFanTable from '@/components/BatchFanTable.vue'
import BatchThreshold from '@/components/BatchThreshold.vue'
import BatchPeriod from '@/components/BatchPeriod.vue'
import BatchFrequency from '@/components/BatchFrequency.vue'

export default {
  name: 'BatchConfiguration',
  data () {
    return {
      batchPoolsChecked: false,
      batchPoolTableOption: {
        cancel: {
          visible: false
        }
      },
      loading: false
    }
  },
  components: {
    BatchPoolsTable,
    BatchNetworkTable,
    BatchFanTable,
    BatchThreshold,
    BatchFrequency,
    BatchPeriod,
    MinerList,
    IpRange
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
    sendBatch (url, params, name, confirmMsg) {
      if (this.checkedIps.length === 0) {
        this.$alert('请添加待配置矿机')
        return
      }
      this.$msgbox({
        title: '提示',
        message: confirmMsg,
        showCancelButton: true
      }).then(() => {
        let payload = Object.assign({}, {ips: this.checkedIps}, params)
        this.loading = true
        this.$ajax.put(url, payload).then((response) => {
          if (response.data.result === 'success') {
            this.$alert(`已发送批量配置${name}请求`)
          } else {
            this.$alert(`发送批量配置${name}请求失败`)
          }
          this.loading = false
        }).catch((error) => {
          this.$alert(error.message)
          this.loading = false
        })
      })
    },
    saveBatchPools (pools, confirmMsg) {
      this.sendBatch('/v1/batch/pools', {data: pools}, '矿池', confirmMsg)
    },
    saveBatchNetwork (network, confirmMsg) {
      this.sendBatch('/v1/batch/network', {data: network}, '网络', confirmMsg)
    },
    saveBatchFan (fans, confirmMsg) {
      this.sendBatch('/v1/batch/fans', {data: fans}, '风扇', confirmMsg)
    },
    saveBatchThreshold (threshold, confirmMsg) {
      this.sendBatch('/v1/batch/threshold', {data: threshold}, '告警门限', confirmMsg)
    },
    saveBatchPeriod (period, confirmMsg) {
      this.sendBatch('/v1/batch/period', {data: period}, '上报周期', confirmMsg)
    },
    saveBatchFrequency (freqs, confirmMsg) {
      this.sendBatch('/v1/batch/frequency', {data: freqs}, '频率', confirmMsg)
    }
  }
}
</script>

<style>
  .batch-container .common-box {
    padding: 20px;
  }
  .batch-card-title {
    margin-left: 4px;
  }
  .batch-pools-card .el-card__body {
    padding-top: 2px;
  }
</style>
