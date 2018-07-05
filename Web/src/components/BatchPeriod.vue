<template>
  <div>
    <div class="config-input-line">
      <span class="config-input-text" style="color: #606266;">上报周期(分钟): </span>
      <el-input type="number" size="small" placeholder="请输入上报周期" v-model="periodic_time" style="width: 180px;"></el-input>
    </div>
    <div style="text-align: center; margin-top: 15px;">
      <el-button @click="empty">清空</el-button>
      <el-button :disabled="periodic_time === ''" type="primary" @click="save">保存</el-button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'BatchPeriod',
  data () {
    return {
      periodic_time: ''
    }
  },
  methods: {
    empty () {
      this.periodic_time = ''
    },
    save () {
      const h = this.$createElement
      let hs = [
        h('p', null, `上报周期(分钟): ${this.periodic_time}`)
      ]
      let confirmMessage = h('div', null, hs)
      this.$emit('save', {periodic_time: Number(this.periodic_time)}, confirmMessage)
    }
  }
}
</script>

<style>
</style>
