<template>
<div>
  <el-table :data="[network]" style="width: 100%">
    <el-table-column type="index"></el-table-column>

    <el-table-column prop="ip_type" label="Protocol" :width="isStatic ? 200 : ''">
      <template slot-scope="scope">
        <el-select v-model.trim="scope.row.ip_type" size="small" style="width: 140px;">
          <el-option v-for="item in options" :key="item.value" :label="item.label" :value="item.value"></el-option>
        </el-select>
      </template>
    </el-table-column>

    <!-- <el-table-column v-if="isStatic" prop="ip_start" label="起始IP">
      <template slot-scope="scope">
        <el-input placeholder="请输入起始IP" v-model.trim="scope.row.ip_start" size="small"></el-input>
      </template>
    </el-table-column>

    <el-table-column v-if="isStatic" prop="ip_end" label="结束IP">
      <template slot-scope="scope">
        <el-input placeholder="请输入结束IP" v-model.trim="scope.row.ip_end" size="small"></el-input>
      </template>
    </el-table-column> -->

    <el-table-column v-if="isStatic" prop="netmask" label="Netmask">
      <template slot-scope="scope">
        <el-input placeholder="请输入netmask" v-model.trim="scope.row.netmask" size="small"></el-input>
      </template>
    </el-table-column>

    <el-table-column v-if="isStatic" prop="gateway" label="Gateway">
      <template slot-scope="scope">
        <el-input placeholder="请输入gateway" v-model.trim="scope.row.gateway" size="small"></el-input>
      </template>
    </el-table-column>

    <el-table-column v-if="isStatic" prop="dns" label="DNS">
      <template slot-scope="scope">
        <el-input placeholder="请输入DNS server" v-model.trim="scope.row.dns" size="small"></el-input>
      </template>
    </el-table-column>
  </el-table>
  <div style="text-align: center; margin-top: 15px;">
    <el-button @click="empty">清空</el-button>
    <el-button :disabled="isSaveDisabled" type="primary" @click="save">保存</el-button>
  </div>
</div>
</template>

<script>
export default {
  name: 'BatchNetworkTable',
  data () {
    return {
      isEdit: false,
      copy: null,
      loading: false,
      options: [
        { value: 'dhcp', label: 'dhcp' },
        { value: 'static', label: 'static' }
      ],
      network: {
        ip_type: 'dhcp',
        ip_start: '',
        ip_end: '',
        netmask: '',
        gateway: '',
        dns: ''
      }
    }
  },
  computed: {
    isStatic () {
      return this.network.ip_type === 'static'
    },
    isSaveDisabled () {
      return this.isStatic && this.network.netmask === '' && this.network.gateway === '' && this.network.dns === ''
    }
  },
  methods: {
    empty () {
      this.network.netmask = ''
      this.network.gateway = ''
      this.network.dns = ''
    },
    checkIpRange (ip) {
      let nums = ip.split('.')
      for (let i = 0; i < nums.length; i++) {
        let num = Number(nums[i])
        if (num > 255) {
          return false
        }
      }
      return true
    },
    inIpSegment (ipStart, ipEnd) {
      let numsStart = ipStart.split('.')
      let numsEnd = ipEnd.split('.')
      for (let i = 0; i < numsStart.length - 1; i++) {
        let num1 = numsStart[i]
        let num2 = numsEnd[i]
        if (num1 !== num2) {
          return false
        }
      }
      return true
    },
    save () {
      console.log('save')

      let payload = {
        ip_type: 'dhcp'
      }

      const checkip = this.$util.checkip
      if (this.network.ip_type === 'static') {
        if (this.network.netmask !== '' && !checkip(this.network.netmask)) {
          this.$alert('请输入正确的 netmask')
          return
        }
        if (this.network.ip_start !== '' && !checkip(this.network.ip_start)) {
          this.$alert('请输入正确的 起始IP')
          return
        }
        if (this.network.ip_end !== '' && !checkip(this.network.ip_end)) {
          this.$alert('请输入正确的 结束IP')
          return
        }

        if (this.checkIpRange(this.network.ip_start, this.network.ip_end)) {
          this.$alert('起始IP和结束IP需在同一网段, 比如 192.168.4.2  192.168.4.253')
          return
        }

        if (this.network.gateway !== '' && !checkip(this.network.gateway)) {
          this.$alert('请输入正确的 gateway')
          return
        }
        if (this.network.dns !== '' && !checkip(this.network.dns)) {
          this.$alert('请输入正确的 dns')
          return
        }
        payload = {
          ip_type: this.network.ip_type
        }
        if (this.network.netmask !== '') {
          payload.netmask = this.network.netmask
        }
        if (this.network.gateway !== '') {
          payload.gateway = this.network.gateway
        }
        if (this.network.dns !== '') {
          payload.dns = this.network.dns
        }
      }
      const h = this.$createElement
      let hs = [
        h('p', null, `protocol: ${payload.ip_type}`)
      ]
      if (this.network.ip_type === 'static') {
        if (payload.netmask) {
          hs.push(h('p', null, `netmask: ${payload.netmask}`))
        }
        if (payload.gateway) {
          hs.push(h('p', null, `gateway: ${payload.gateway}`))
        }
        if (payload.dns) {
          hs.push(h('p', null, `dns: ${payload.dns}`))
        }
      }
      let confirmMessage = h('div', null, hs)

      this.$emit('save', payload, confirmMessage)
    }
  },
  mounted () {
  }
}
</script>
