<template>
  <div class="miner-detail">
    <el-row type="flex" class="mb" justify="space-between" align="bottom">
      <el-col><div class="clr-black-light">矿机状态: <span :class="{'clr-danger': infoData.status !== 'running'}" v-text="infoData.status"></span></div></el-col>
      <el-col>
        <div class=""><span class="clr-black-light">实时算力:</span>
          <tween-number :precision="2" :number="Number(infoData.curHashrate.value)"></tween-number>
          <span class="clr-black-light" v-text="infoData.curHashrate.unit"></span>
        </div>
      </el-col>
      <el-col>
        <div class=""><span class="clr-black-light">平均算力:</span>
          <tween-number :precision="2" :number="Number(infoData.avgHashrate.value)"></tween-number>
          <span class="clr-black-light" v-text="infoData.avgHashrate.unit"></span>
        </div>
      </el-col>
      <el-col><div class="clr-black-light">运行时间: <span v-text="infoData.duration"></span></div></el-col>
    </el-row>
    <el-row style="margin-bottom: 20px;">
      <el-tabs type="border-card">
        <el-tab-pane label="矿机信息">
          <miner-info :table-data="tableData" @refresh="refreshMinerTable"></miner-info>
        </el-tab-pane>
        <el-tab-pane label="算力板">
          <miner-board :table-data="boardData"></miner-board>
        </el-tab-pane>
        <el-tab-pane label="风扇">
          <miner-fan :table-data="fanData"></miner-fan>
        </el-tab-pane>
        <el-tab-pane label="矿池">
          <miner-pool :table-data="poolData" @refresh="refreshMinerTable"></miner-pool>
        </el-tab-pane>
        <el-tab-pane label="网络">
          <miner-network :table-data="tableData" @refresh="refreshMinerTable"></miner-network>
        </el-tab-pane>
      </el-tabs>
    </el-row>
    <el-row class="mb30 spline-wrap">
      <hashrate-spline :hashrateList="fakeDatas" @export="exportHashrate" @switchTimeInterval="switchTimeInterval" title="矿机算力图表"></hashrate-spline>
    </el-row>
    <el-row class="mb30 spline-wrap">
      <temperature-spline :current="infoData.temperature" :temperatureList="temperatureList" @export="exportTemperature"></temperature-spline>
    </el-row>
  </div>
</template>

<script>
import MinerInfo from '@/components/MinerInfo.vue'
import HashrateSpline from '@/components/HashrateSpline.vue'
import TemperatureSpline from '@/components/TemperatureSpline.vue'
import MinerBoard from '@/components/MinerBoard.vue'
import MinerFan from '@/components/MinerFan.vue'
import MinerPool from '@/components/MinerPool.vue'
import MinerNetwork from '@/components/MinerNetwork.vue'
import TweenNumber from '@/components/TweenNumber.vue'

export default {
  name: 'MinerDetail',
  data () {
    return {
      temperatureList: null,
      tableData: null,
      fakeDatas: null,
      timerInfo: null,
      timerTemperatureList: null,
      timerHashrateList: null,
      hashrateInterval: 'day'
    }
  },
  components: {
    MinerInfo,
    HashrateSpline,
    TemperatureSpline,
    MinerBoard,
    MinerFan,
    MinerPool,
    MinerNetwork,
    TweenNumber
  },
  computed: {
    infoData () {
      let rtn = {
        status: '',
        duration: '',
        curHashrate: '',
        avgHashrate: '',
        temperature: ''
      }
      if (this.tableData && this.tableData.length) {
        let data = this.tableData[0]
        rtn.status = data.status
        rtn.duration = this.$util.formatDuration(data.duration)
        rtn.temperature = data.temperature
        rtn.curHashrate = this.$util.formatHashrate((data.mhs || 0) * 1024 * 1024)
        rtn.avgHashrate = this.$util.formatHashrate((data.avg_mhs || 0) * 1024 * 1024)
      }
      return rtn
    },
    poolData () {
      let rtn = []
      if (this.tableData && this.tableData.length) {
        let data = this.tableData[0]
        let pool1 = {name: 'pool1', addr: data.pool1_addr, miner_addr: data.pool1_miner_addr, password: data.pool1_password, status: data.pool1_status, mac: data.mac}
        let pool2 = {name: 'pool2', addr: data.pool2_addr, miner_addr: data.pool2_miner_addr, password: data.pool2_password, status: data.pool2_status, mac: data.mac}
        let pool3 = {name: 'pool3', addr: data.pool3_addr, miner_addr: data.pool3_miner_addr, password: data.pool3_password, status: data.pool3_status, mac: data.mac}
        rtn.push(pool1, pool2, pool3)
      }
      return rtn
    },
    fanData () {
      let rtn = []
      if (this.tableData && this.tableData.length) {
        let data = this.tableData[0]
        let fan1 = {name: 'fan1', speed: data.fan1_speed, status: data.fan1_status}
        let fan2 = {name: 'fan2', speed: data.fan2_speed, status: data.fan2_status}
        rtn.push(fan1, fan2)
      }
      return rtn
    },
    boardData () {
      let rtn = []
      if (this.tableData && this.tableData.length) {
        let data = this.tableData[0]
        let board1 = {name: 'board1', accept: data.board1_accept, frequency: data.board1_frequency, mhs: data.board1_mhs, reject: data.board1_reject, temperature: data.board1_temperate}
        let board2 = {name: 'board2', accept: data.board2_accept, frequency: data.board2_frequency, mhs: data.board2_mhs, reject: data.board2_reject, temperature: data.board2_temperate}
        let board3 = {name: 'board3', accept: data.board3_accept, frequency: data.board3_frequency, mhs: data.board3_mhs, reject: data.board3_reject, temperature: data.board3_temperate}
        let board4 = {name: 'board4', accept: data.board4_accept, frequency: data.board4_frequency, mhs: data.board4_mhs, reject: data.board4_reject, temperature: data.board4_temperate}
        rtn.push(board1, board2, board3, board4)
      }
      return rtn
    }
  },
  methods: {
    exportHashrate () {
      let mac = this.$route.query.mac
      window.open('/v1/excel/hashrate/single/' + this.hashrateInterval + '/' + mac)
    },
    exportTemperature () {
      let mac = this.$route.query.mac
      window.open(`/v1/excel/temperature/${mac}`)
    },
    fetchData () {
      // let ip = this.$route.query.ip
      let mac = this.$route.query.mac
      this.$ajax.get(`/v1/miner?mac=${mac}`)
        .then((response) => {
          this.tableData = [response.data.miner]
        })
    },
    refreshMinerTable (doneFn) {
      // let ip = this.$route.query.ip
      let mac = this.$route.query.mac
      this.$ajax.get(`/v1/miner?mac=${mac}`)
        .then((response) => {
          this.tableData = [response.data.miner]
          doneFn()
        })
        .catch((error) => {
          this.$alert(error.message)
          doneFn()
        })
    },
    getHashrateList (interval) {
      this.$ajax.get('/v1/miner/mhs?period=' + interval + '&mac=' + this.$route.query.mac) // 总算力天周月季半年年
        .then((response) => {
          let minerMhs = response.data.miner_mhs
          let res = []
          for (let i = 0; i < minerMhs.length; i++) {
            let item = minerMhs[i]
            res.push([new Date(item.date).getTime(), item.mhs])
          }
          this.fakeDatas = { // 暂时都显示一样的数据
            day: res,
            week: res,
            month: res,
            season: res,
            half: res,
            year: res
          }
        })
    },
    switchTimeInterval (interval) {
      clearInterval(this.timerHashrateList)
      this.hashrateInterval = interval
      this.getHashrateList(interval)
      this.timerHashrateList = setInterval(() => {
        this.getHashrateList(interval)
      }, 60000)
    },
    fetchTemperautreList () {
      this.$ajax.get('/v1/miner/temperature?period=day&mac=' + this.$route.query.mac)
        .then(response => {
          let temps = response.data.miner_temperature
          let res = []
          for (let i = 0; i < temps.length; i++) {
            let item = temps[i]
            res.push([new Date(item.date).getTime(), item.temperature])
          }
          this.temperatureList = res
        })
    }
  },
  created () {
    this.fetchData()
    this.timerInfo = setInterval(() => {
      this.fetchData()
    }, 10000)
    this.switchTimeInterval('day')

    setTimeout(() => {
      this.fetchTemperautreList()
    }, 1000)

    this.timerTemperatureList = setInterval(() => {
      this.fetchTemperautreList()
    }, 60000)
  },
  mounted () {
  },
  beforeDestroy () {
    clearInterval(this.timerInfo)
    clearInterval(this.timerTemperatureList)
    clearInterval(this.timerHashrateList)
  }
}
</script>

<style>

</style>
