<template>
  <el-tabs type="border-card">
    <el-tab-pane label="告警矿机">
      <miner-table-list miner-type="alert" v-loading="loadingError" element-loading-text="请求中" :export-visible="exportVisible" :multiple-select="multipleSelect" :show-progress="showProgress" :table-data="errorMiners" :page-size="pageSize" :total="errorMinerAmount" @search="searchError" @handleCurrentChange="handleErrorCurrentChange" @addIpRange="addIpRange">
        <template v-if="multipleSelect" slot="operation">
          <el-button type="primary" @click="addIps('alert')">加入待配置矿机列表</el-button>
        </template>
      </miner-table-list>
    </el-tab-pane>
    <el-tab-pane label="所有矿机">
      <miner-table-list miner-type="all" v-loading="loadingFull" element-loading-text="请求中" :export-visible="exportVisible" :multiple-select="multipleSelect" :show-progress="showProgress" :table-data="fullMiners" :page-size="pageSize" :total="allMinerAmount" @search="searchFull" @handleCurrentChange="handleAllCurrentChange" @addIpRange="addIpRange">
        <template v-if="multipleSelect" slot="operation">
          <el-button type="primary" @click="addIps('all')">加入待配置矿机列表</el-button>
        </template>
      </miner-table-list>
    </el-tab-pane>
  </el-tabs>
</template>

<script>
import MinerTableList from '@/components/MinerTableList.vue'
import {mapGetters} from 'vuex'

export default {
  name: 'MinerList',
  props: {
    showProgress: {
      type: Boolean,
      default: false
    },
    percentageList: {
      type: Array,
      default () {
        return []
      }
    },
    multipleSelect: {
      type: Boolean,
      default: true
    },
    exportVisible: {
      type: Boolean,
      default: true
    }
  },
  data () {
    return {
      errorTableData: [],
      fullTableData: [],
      errorMinerAmount: 0,
      allMinerAmount: 0,
      pageSize: 50,
      loadingError: false,
      loadingFull: false
    }
  },
  computed: {
    ...mapGetters({
      batchAlertSelection: 'batchAlertSelection',
      batchAllSelection: 'batchAllSelection'
    }),
    percentageMap () {
      let map = {}
      this.percentageList.forEach((item) => {
        let mac = item.split('~')[0]
        let val = Number(item.split('~')[1])
        map[mac] = val
      })
      return map
    },
    errorMiners () {
      let rtn = this.errorTableData.concat()

      rtn.forEach((item) => {
        item.percentage = this.percentageMap[item.mac]
      })
      return rtn
    },
    fullMiners () {
      let rtn = this.fullTableData.concat()
      rtn.forEach((item) => {
        item.percentage = this.percentageMap[item.mac]
      })
      return rtn
    }
  },
  components: {
    MinerTableList
  },
  methods: {
    getErrorMinerListBy (pageNum, searchText = '') {
      let url = '/v1/miners/alerts?offset=' + ((pageNum - 1) * this.pageSize) + '&size=' + this.pageSize
      if (searchText) {
        url += '&' + searchText
      }
      this.$ajax.get(url) // 告警矿机
        .then((response) => {
          this.loadingError = false
          this.errorTableData = response.data.miners_alerts
          this.errorMinerAmount = response.data.total
        })
        .catch((error) => {
          this.$alert(error.message)
          this.loadingError = false
        })
    },
    getFullMinerListBy (pageNum, searchText = '') {
      let url = '/v1/miners?offset=' + ((pageNum - 1) * this.pageSize) + '&size=' + this.pageSize
      if (searchText) {
        url += '&' + searchText
      }
      this.$ajax.get(url) // 所有矿机
        .then((response) => {
          this.loadingFull = false
          this.fullTableData = response.data.miners
          this.allMinerAmount = response.data.total
        })
        .catch((error) => {
          this.$alert(error.message)
          this.loadingFull = false
        })
    },
    searchError (text) {
      this.loadingError = true
      this.getErrorMinerListBy(1, text)
    },
    searchFull (text) {
      this.loadingFull = true
      this.getFullMinerListBy(1, text)
    },
    handleErrorCurrentChange (pageNum, searchText) {
      this.getErrorMinerListBy(pageNum, searchText)
    },
    handleAllCurrentChange (pageNum, searchText) {
      this.getFullMinerListBy(pageNum, searchText)
    },
    addIps (type) {
      let ipList = []

      if (type === 'alert') {
        ipList = this.batchAlertSelection
      }
      if (type === 'all') {
        ipList = this.batchAllSelection
      }

      ipList = ipList.map((value, index) => {
        return value.ip
      })

      this.$emit('addIps', ipList)
    },
    addIpRange (ips) {
      this.$emit('addIps', [ips])
    }
  },
  created () {
    this.getErrorMinerListBy(1, '')

    this.getFullMinerListBy(1, '')
  }
}
</script>
