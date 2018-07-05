<template>
  <div>
    <div class="chart-title" v-text="title"></div>
    <div class="chart-select-time clearfix">
      <el-button-group>
        <el-button size="small" @click="filter('day')" :type="btnTypeBy('day')">天</el-button>
        <el-button size="small" @click="filter('week')" :type="btnTypeBy('week')">周</el-button>
        <el-button size="small" @click="filter('month')" :type="btnTypeBy('month')">月</el-button>
        <el-button size="small" @click="filter('season')" :type="btnTypeBy('season')">季</el-button>
        <el-button size="small" @click="filter('half')" :type="btnTypeBy('half')">半年</el-button>
        <el-button size="small" @click="filter('year')" :type="btnTypeBy('year')">年</el-button>
      </el-button-group>
      <div class="chart-toolBox fr">
        <el-button size="small" @click="exportExcel">导出</el-button>
      </div>
    </div>
    <div class="flex-center">
      <div id="hashratespline-container" class="flex-center echart-container">
      </div>
    </div>
  </div>
</template>

<script>
import echarts from 'echarts'
import chart from '@/common/chart.js'

export default {
  name: 'HashrateSpline',
  props: {
    hashrateList: {
      default () {
        return {
          day: [],
          week: [],
          month: [],
          season: [],
          half: [],
          year: []
        }
      }
    },
    title: ''
  },
  data () {
    return {
      filterKey: 'day',
      myChart: null,
      unit: 'MH/S'
    }
  },
  methods: {
    btnTypeBy (fk) {
      return fk === this.filterKey ? 'primary' : ''
    },
    filter (fk) {
      this.filterKey = fk
      this.$emit('switchTimeInterval', fk)
      // this.refreshData(this.hashrateList[this.filterKey])
    },
    pad0 (num) {
      return num < 10 ? '0' + num : num
    },
    formatTime (timespan) {
      let date = new Date(timespan)
      let y = date.getFullYear().toString() + '/'
      let M = this.pad0(date.getMonth() + 1) + '/'
      let d = this.pad0(date.getDate()) + ' '
      let h = this.pad0(date.getHours()) + ':'
      let m = this.pad0(date.getMinutes()) + ':'
      let s = this.pad0(date.getSeconds())
      return y + M + d + h + m + s
    },
    initData (data) {
      let vm = this
      const colors = ['#5793f3', '#675bba', '#d14a61']
      const option = {
        color: colors,
        title: {
          show: false
        },
        tooltip: {
          trigger: 'axis',
          axisPointer: {
            animation: !1,
            lineStyle: {
              color: '#5793f3'
            }
          },
          enterable: !1,
          backgroundColor: 'rgba(0,0,0,0.5)',
          borderColor: '#333',
          padding: 7,
          textStyle: {
            fontSize: 12,
            fontWeight: 'lighter'
          },
          formatter: function (params) {
            return vm.formatTime(params[0].data[0]) + '<br>\n' + '算力: ' + params[0].data[1].toFixed(2)
          }
        },
        grid: {
          left: '3%',
          right: '4%',
          bottom: '3%',
          top: '40',
          containLabel: !0
        },
        xAxis: {
          type: 'time',
          splitLine: {
            show: !1
          },
          boundaryGap: !1,
          offset: 10,
          axisLabel: {
            margin: 2,
            textStyle: {
              color: '#888'
            }
          },
          axisLine: {
            lineStyle: {
              color: '#aaa'
            }
          }
        },
        yAxis: [
          {
            type: 'value',
            name: `算力(${this.unit})`,
            position: 'left',
            boundaryGap: !1,
            splitLine: {
              show: !1
            },
            splitArea: {
              show: true,
              areaStyle: {
                color: ['rgba(250,250,250,0.1)', 'rgba(200,200,200,0.05)']
              }
            },
            axisLine: {
              lineStyle: {
                color: '#aaa'
              }
            },
            axisLabel: {
              formatter: '{value}',
              textStyle: {
                color: '#888'
              }
            }
          }
        ],
        series: [
          {
            name: '算力',
            type: 'line',
            smooth: 0.3,
            data: data,
            yAxisIndex: 0,
            symbol: 'emptyCircle',
            symbolSize: 4,
            itemStyle: {
              color: '#5793f3',
              borderWidth: 1
            },
            lineStyle: {
              color: '#5793f3',
              width: '2'
            },
            areaStyle: {
              color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
                offset: 0,
                color: '#C3DCF3'
              }, {
                offset: 1,
                color: '#fff'
              }])
            },
            emphasis: {
              itemStyle: {
                color: '#5793f3'
              }
            }
          }
        ]
      }
      this.myChart.setOption(option)
    },
    exportPic () {
      chart.exportPic(this.myChart, 'hashrate_spline')
    },
    exportExcel () {
      this.$emit('export')
    }
  },
  watch: {
    hashrateList (newList) {
      let data = Object.assign({}, newList)
      data = data[this.filterKey]
      let max = 0
      for (let i = 0; i < data.length; i++) {
        if (data[i][1] > max) {
          max = data[i][1]
        }
      }
      let fmtMaxHashrate = this.$util.formatHashrate(max * 1024 * 1024)
      this.unit = fmtMaxHashrate.unit
      for (let i = 0; i < data.length; i++) {
        data[i][1] = data[i][1] * 1024 * 1024 / fmtMaxHashrate.totalRate
      }
      if (!this.myChart) {
        this.myChart = echarts.init(document.getElementById('hashratespline-container'))
        this.initData(data)
      } else {
        var opt = this.myChart.getOption()
        opt.yAxis[0].name = `算力(${this.unit})`
        opt.series[0].data = data
        this.myChart.setOption(opt)
      }
    }
  },
  created () {
  },
  mounted () {
  }
}
</script>

<style scoped>

</style>
