<template>
<div>
  <el-table :data="pools" stripe style="width: 100%">
    <el-table-column type="index"></el-table-column>
    <el-table-column min-width="10%" prop="name" label="名称">
      <template slot-scope="scope">
        <span v-text="scope.row.name"></span>
      </template>
    </el-table-column>
    <el-table-column min-width="40%" prop="addr" label="矿池地址">
      <template slot-scope="scope">
        <el-input v-show="isEdit" placeholder="请输入矿池地址" v-model.trim="scope.row.addr" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column min-width="20%" prop="minerAddr" label="矿工名">
      <template slot-scope="scope">
        <el-input v-show="isEdit" placeholder="请输入矿工名" v-model.trim="scope.row.minerAddr" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column min-width="20%" prop="password" label="密码">
      <template slot-scope="scope">
        <el-input v-show="isEdit" placeholder="请输入密码" v-model.trim="scope.row.password" size="small"></el-input>
      </template>
    </el-table-column>
  </el-table>
  <div style="text-align: center; margin-top: 15px;">
    <el-button v-if="_option.empty.visible" v-show="isEdit" @click="empty">清空</el-button>
    <el-button v-if="_option.cancel.visible" v-show="isEdit" @click="cancel">取消</el-button>
    <el-button v-if="_option.save.visible" v-show="isEdit" type="primary" @click="save">保存</el-button>
  </div>
</div>
</template>

<script>
export default {
  props: {
    option: {
      type: Object
    }
  },
  data () {
    return {
      isEdit: true,
      pools: [
        {
          name: 'pool1',
          addr: '',
          minerAddr: '',
          password: ''
        },
        {
          name: 'pool2',
          addr: '',
          minerAddr: '',
          password: ''
        },
        {
          name: 'pool3',
          addr: '',
          minerAddr: '',
          password: ''
        }
      ]
    }
  },
  computed: {
    _option () {
      let __option = {
        cancel: {
          visible: true
        },
        empty: {
          visible: true
        },
        save: {
          visible: true
        }
      }
      return Object.assign(__option, this.option)
    }
  },
  methods: {
    cancel () {
      this.$emit('cancel')
    },
    save () {
      if (this.pools[0].addr === '') {
        this.$alert('pool1的矿池地址不能为空!')
        return
      } else if (this.pools[0].minerAddr === '') {
        this.$alert('pool1的矿工名不能为空!')
        return
      }
      const h = this.$createElement
      let pool1 = this.pools[0]
      let pool2 = this.pools[1]
      let pool3 = this.pools[2]
      let confirmMessage = h('div', null, [
        h('p', null, `pool1 地址: ${pool1.addr} 矿工名: ${pool1.minerAddr} 密码: ${pool1.password}`),
        h('p', null, `pool2 地址: ${pool2.addr} 矿工名: ${pool2.minerAddr} 密码: ${pool2.password}`),
        h('p', null, `pool3 地址: ${pool3.addr} 矿工名: ${pool3.minerAddr} 密码: ${pool3.password}`)
      ])
      this.$emit('save', this.pools, confirmMessage)
    },
    empty () {
      this.pools.forEach((pool) => {
        pool.addr = ''
        pool.minerAddr = ''
        pool.password = ''
      })
    }
  }
}
</script>
