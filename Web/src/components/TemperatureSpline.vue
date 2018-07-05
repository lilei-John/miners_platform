<template>
  <div>
    <div class="chart-title">矿机温度图表  当前温度:<span> {{current}}</span><span style="font-size:14px;">℃</span></div>
    <div class="clearfix">
      <div class="chart-toolBox fr">
        <el-button size="small" @click="exportExcel">导出</el-button>
      </div>
    </div>
    <div class="flex-center">
      <div id="temperaturespline-container" class="flex-center echart-container"></div>
    </div>
  </div>
</template>

<script>
import echarts from 'echarts'
import chart from '@/common/chart.js'

export default {
  name: 'TemperatureSpline',
  props: {
    temperatureList: {
      type: Array
    },
    current: {
      default: ''
    }
  },
  data () {
    return {
      myChart: null
    }
  },
  methods: {
    initData () {
      let vm = this
      const colors = ['#df2639', '#675bba', '#d14a61']
      const option = {
        color: colors,
        tooltip: {
          trigger: 'axis',
          axisPointer: {
            animation: !1,
            lineStyle: {
              color: '#df2639'
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
            return vm.$util.formatTime(params[0].data[0]) + '<br>\n' + '温度: ' + params[0].data[1].toFixed(2) + ' ℃'
          }
        },
        grid: {
          left: '3%',
          right: '4%',
          bottom: '3%',
          top: 40,
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
            name: '温度 (℃)',
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
            data: this.temperatureList,
            yAxisIndex: 0,
            symbol: 'emptyCircle',
            symbolSize: 4,
            itemStyle: {
              color: '#df2639'
            },
            lineStyle: {
              color: '#df2639',
              width: '2'
            },
            areaStyle: {
              color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
                offset: 0,
                color: '#df2639'
              }, {
                offset: 1,
                color: '#fff'
              }])
            },
            emphasis: {
              itemStyle: {
                color: '#df2639'
              }
            }
          }
        ]
      }
      this.myChart.setOption(option)
    },
    refreshData (data) {
      if (!this.myChart) {
        this.myChart = echarts.init(document.getElementById('temperaturespline-container'))
        this.initData()
        return
      }
      var opt = this.myChart.getOption()
      opt.series[0].data = data
      this.myChart.setOption(opt)
    },
    exportPic () {
      chart.exportPic(this.myChart, 'miner_temperature_spline')
    },
    exportExcel () {
      this.$emit('export')
    }
  },
  watch: {
    temperatureList (newList) {
      this.refreshData(newList)
    }
  },
  mounted () {
    this.refreshData()
  }
}
</script>

<style>
#temperaturespline-container {
  height: 260px;
}
</style>
