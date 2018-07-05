<template>
  <div>
    <el-row type="flex" class="mb" justify="space-between">
      <el-col :span="16">
        <div class="fl mr">
          <tween-number :precision="2" :number="Number(curHashrate.value)"></tween-number>
          <span v-text="curHashrate.unit" class="clr-black-light"></span>&nbsp;
        </div>
        <div class="fl mr">
          <tween-number :number="mining.total"></tween-number> <span class="clr-black-light">在线矿机</span>
        </div>
        <div class="fl mr">
          <tween-number :number="mining.normal"></tween-number> <span class="clr-black-light">正常</span>
        </div>
        <div>
          <tween-number class="clr-danger" :number="mining.abnormal"></tween-number> <span class="clr-black-light">异常</span>
        </div>
      </el-col>
      <!-- <el-col :span="8">
        <div class="fr">
          <div class="fl mr"><span class="clr-black-light">温度:</span>
            <tween-number :number="mining.temperature"></tween-number><span class="clr-black-light" style="font-size: 14px; margin-left: 1px;">℃</span>
          </div>
          <div class="fl"><span class="clr-black-light">湿度:</span>
            <tween-number :number="mining.humidity"></tween-number><span class="clr-black-light" style="margin-left: 1px;">%</span>
          </div>
        </div>
      </el-col> -->
    </el-row>
    <el-row class="mb30 spline-wrap">
      <hashrate-spline ref="spline" :hashrateList="hashrateList" @switchTimeInterval="switchTimeInterval" title="总算力图表" @export="exportHashrateList"></hashrate-spline>
    </el-row>
    <el-row>
      <miner-list :multiple-select="false"></miner-list>
    </el-row>
  </div>
</template>

<script>
import TweenNumber from '@/components/TweenNumber.vue'
import HashrateSpline from '@/components/HashrateSpline.vue'
import MinerList from '@/components/MinerList.vue'

export default {
  name: 'Home',
  data () {
    return {
      mining: {
        mhs: 0,
        total: 0,
        normal: 0,
        abnormal: 0
      },
      hashrateList: {},
      timerGetMining: null,
      timerGetHashrateList: null,
      hashrateInterval: 'day'
    }
  },
  computed: {
    curHashrate () {
      return this.$util.formatHashrate(this.mining.mhs * 1024 * 1024)
    }
  },
  components: {
    HashrateSpline,
    MinerList,
    TweenNumber
  },
  methods: {
    switchTimeInterval (interval) {
      clearInterval(this.timerGetHashrateList)
      this.hashrateInterval = interval

      this.getHashrateListBy(this.hashrateInterval)

      this.timerGetHashrateList = setInterval(() => {
        this.getHashrateListBy(this.hashrateInterval)
      }, 60000)
    },
    getHashrateListBy (interval) {
      this.$ajax.get('/v1/mining/mhs?period=' + interval) // 总算力天周月季半年年
        .then((response) => {
          let miningMhs = response.data.mining_mhs
          let res = []
          for (let i = 0; i < miningMhs.length; i++) {
            let item = miningMhs[i]
            res.push([new Date(item.date).getTime(), item.mhs])
          }
          this.hashrateList = { // 暂时都显示一样的数据
            day: res,
            week: res,
            month: res,
            season: res,
            half: res,
            year: res
          }
        })
    },
    getMiningInfo () {
      this.$ajax.get('/v1/mining') // 平台信息
        .then((response) => {
          this.mining = response.data.mining[0]
        })
    },
    exportHashrateList () {
      window.open('/v1/excel/hashrate/all/' + this.hashrateInterval)
    }
  },
  created () {
    this.getMiningInfo()
    this.timerGetMining = setInterval(() => {
      this.getMiningInfo()
    }, 10000)

    this.timerGetHashrateList = setInterval(() => {
      this.getHashrateListBy(this.hashrateInterval)
    }, 60000)

    this.getHashrateListBy(this.hashrateInterval)
  },
  beforeDestroy () {
    clearInterval(this.timerGetMining)
    clearInterval(this.timerGetHashrateList)
  }
}
</script>

<style scoped>
</style>
