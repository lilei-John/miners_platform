<template>
  <div class="batch-ip-range">
    <el-row type="flex" style="margin-bottom: 10px;" align="middle">
      <div class="mr">
        <el-checkbox :indeterminate="isIndeterminate" v-model="checkAll" @change="handleCheckAllChange">待配置矿机列表:</el-checkbox>
      </div>
      <el-button v-if="false" size="mini" type="primary" plain @click="addips">+</el-button>
      <el-button v-if="false" size="mini" type="primary" plain @click="removeips">-</el-button>
    </el-row>
    <el-row v-if="false" type="flex" style=" margin-bottom: 10px;">
      <div>
        起始IP:
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP0')" @input="handleIpInput($event, 'startIP0')" v-model="startIP0" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP1')" @input="handleIpInput($event, 'startIP1')" v-model="startIP1" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP2')" @input="handleIpInput($event, 'startIP2')" v-model="startIP2" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP3')" @input="handleIpInput($event, 'startIP3')" v-model="startIP3" size="mini"></el-input>
      </div>
      <div style="margin-left: 20px">
        结束IP:
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP0')" @input="handleIpInput($event, 'endIP0')" v-model="endIP0" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP1')" @input="handleIpInput($event, 'endIP1')" v-model="endIP1" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP2')" @input="handleIpInput($event, 'endIP2')" v-model="endIP2" size="mini"></el-input> .
        <el-input class="input--ip" @change="handleIpChanged($event, 'startIP3')" @input="handleIpInput($event, 'endIP3')" v-model="endIP3" size="mini"></el-input>
      </div>
    </el-row>
    <el-checkbox-group class="batch-ip-range-list clearfix" v-model="checkedIps" @change="handleCheckedIpsChange">
      <div class="fl" v-for="(item, index) in ipList" :key="item+index" >
        <el-checkbox :label="item"></el-checkbox>
      </div>
    </el-checkbox-group>
  </div>
</template>

<script>
export default {
  name: 'IpRange',
  data () {
    return {
      startIP0: '192',
      startIP1: '168',
      startIP2: '0',
      startIP3: '0',
      endIP0: '192',
      endIP1: '168',
      endIP2: '0',
      endIP3: '0',
      ipList: [],
      checkedIps: [],
      checkAll: false,
      isIndeterminate: false
    }
  },
  computed: {
    ips () {
      let startip = this.startIP0 + '.' + this.startIP1 + '.' + this.startIP2 + '.' + this.startIP3
      let endip = this.endIP0 + '.' + this.endIP1 + '.' + this.endIP2 + '.' + this.endIP3
      let compareResult = this.$util.compareIP(startip, endip)
      let result = startip
      if (compareResult < 0) result += '~' + endip
      else if (compareResult > 0) result = endip + '~' + startip
      return result
    }
  },
  methods: {
    handleIpInput (e, key) {
      this.$nextTick(() => {
        let result = e.replace(/[^\d]/g, '')
        result = result.replace(/\s+/g, '')
        if (result.length >= 2) {
          result = result.replace(/^0+/, '')
          if (result === '') {
            result = '0'
          }
        }
        if (result.length > 3) {
          result = result.substr(0, 3)
        }
        if (Number(result) > 255) {
          result = '255'
        }
        this[key] = result
      })
    },
    handleIpChanged (e, key) {
      if (e === '') {
        this[key] = '0'
      }
    },
    addips () {
      if (!this.ipList.includes(this.ips)) {
        this.ipList.push(this.ips)
        this.checkedIps.push(this.ips)
        this.ipList.sort((a, b) => { return this.$util.compareIP(a, b) })
        this.checkedIps.sort((a, b) => { return this.$util.compareIP(a, b) })
      }
    },
    addSelectedIps (ipList) {
      let isChange = false

      ipList.forEach((value, index) => {
        if (!this.ipList.includes(value)) {
          this.ipList.push(value)
          this.checkedIps.push(value)
          isChange = true
        }
      })

      if (isChange) {
        this.ipList.sort((a, b) => { return this.$util.compareIP(a, b) })
        this.checkedIps.sort((a, b) => { return this.$util.compareIP(a, b) })
      }
    },
    removeips () {
      let ipDeleted = this.ipList.pop()
      let index = this.checkedIps.findIndex(v => v === ipDeleted)
      this.checkedIps.splice(index, 1)
    },
    handleCheckAllChange (val) {
      this.checkedIps = val ? this.ipList.concat() : []
      this.isIndeterminate = false
    },
    handleCheckedIpsChange (value) {
      let checkedCount = value.length
      this.checkAll = checkedCount === this.ipList.length
      this.isIndeterminate = checkedCount > 0 && checkedCount < this.ipList.length
    }
  }
}
</script>

<style>
  .batch-ip-range {
    width: 100%;
  }
  .batch-ip-range-list {
    height: 120px;
    border: 1px solid #dcdfe6;
    width: 100%;
    padding-left: 2px;
    overflow: scroll;
  }
  .batch-ip-range-list .fl {
    box-sizing: border-box;
    width: 25%;
  }
  .input--ip {
    width: 54px;
  }
  .input--ip .el-input__inner{
    text-align:  center;
  }
</style>
