<template>
  <div>
    <div class="config-input-line">
      <span class="config-input-text" style="color: #606266;">温度告警门限(℃): </span>
      <el-input type="number" size="small" placeholder="请输入温度告警门限" v-model="temperature_threshould" style="width: 180px;"></el-input>
    </div>
    <div class="config-input-line">
      <span class="config-input-text" style="color: #606266;">算力告警门限(MHS): </span>
      <el-input type="number" size="small" placeholder="请输入算力告警门限" v-model="mhs_threshould" style="width: 180px;"></el-input>
    </div>
    <div style="text-align: center; margin-top: 15px;">
      <el-button @click="empty">清空</el-button>
      <el-button :disabled="temperature_threshould === '' && mhs_threshould === ''" type="primary" @click="save">保存</el-button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'BatchThreshold',
  data () {
    return {
      temperature_threshould: '',
      mhs_threshould: ''
    }
  },
  methods: {
    empty () {
      this.temperature_threshould = ''
      this.mhs_threshould = ''
    },
    save () {
      let payload = {}
      if (this.temperature_threshould !== '') {
        payload.temperature_threshould = Number(this.temperature_threshould)
      }
      if (this.mhs_threshould !== '') {
        payload.mhs_threshould = Number(this.mhs_threshould)
      }
      const h = this.$createElement
      let hs = []
      if (payload.temperature_threshould) {
        hs.push(h('p', null, `温度告警门限(℃): ${payload.temperature_threshould}`))
      }
      if (payload.mhs_threshould) {
        hs.push(h('p', null, `算力告警门限(MHS): ${payload.mhs_threshould}`))
      }
      let confirmMessage = h('div', null, hs)
      this.$emit('save', payload, confirmMessage)
    }
  }
}
</script>

<style>
</style>
