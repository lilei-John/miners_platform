<template>
 <div>
  <el-table :data="[freqs]" style="width: 100%">
    <el-table-column prop="frequency" label="全部" :render-header="renderHeaderAll">
      <template slot-scope="scope">
        <el-select :disabled="!isAllChecked" v-model.trim="scope.row.frequency" size="small">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </template>
    </el-table-column>
    <el-table-column prop="frequency1" label="算力板1">
      <template slot-scope="scope">
        <el-select :disabled="isAllChecked" v-model.trim="scope.row.frequency1" size="small">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </template>
    </el-table-column>
    <el-table-column prop="frequency2" label="算力板2">
      <template slot-scope="scope">
        <el-select :disabled="isAllChecked" v-model.trim="scope.row.frequency2" size="small">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </template>
    </el-table-column>
    <el-table-column prop="frequency3" label="算力板3">
      <template slot-scope="scope">
        <el-select :disabled="isAllChecked" v-model.trim="scope.row.frequency3" size="small">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </template>
    </el-table-column>
    <el-table-column prop="frequency4" label="算力板4">
      <template slot-scope="scope">
        <el-select :disabled="isAllChecked" v-model.trim="scope.row.frequency4" size="small">
          <el-option v-for="item in options" :key="item" :label="item" :value="item"></el-option>
        </el-select>
      </template>
    </el-table-column>
  </el-table>
  <div style="text-align: center; margin-top: 15px;">
    <el-button type="primary" @click="save">保存</el-button>
  </div>
 </div>
</template>

<script>
import config from '@/common/config.js'

export default {
  name: 'BatchFrequency',
  data () {
    return {
      freqs: {
        frequency: 684,
        frequency1: 684,
        frequency2: 684,
        frequency3: 684,
        frequency4: 684
      },
      options: config.frequencySet,
      isAllChecked: true
    }
  },
  methods: {
    save () {
      let payload = {}
      if (this.isAllChecked) {
        payload = {
          use_frequency_all: this.isAllChecked,
          frequency: this.freqs.frequency
        }
      } else {
        payload = {
          use_frequency_all: this.isAllChecked,
          frequency1: this.freqs.frequency1,
          frequency2: this.freqs.frequency2,
          frequency3: this.freqs.frequency3,
          frequency4: this.freqs.frequency4
        }
      }
      this.$emit('save', payload)
    },
    renderHeaderAll (createElement, { column }) {
      let vm = this
      return createElement(
        'div',
        [
          createElement('span', column.label),
          createElement('el-checkbox', {
            style: {
              marginLeft: '4px'
            },
            attrs: {
              'checked': this.isAllChecked
            },
            on: {
              'change': function (v) {
                vm.isAllChecked = v
              }
            }
          })
        ]
      )
    }
  }
}
</script>
