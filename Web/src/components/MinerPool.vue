<template>
  <div v-loading="loading" element-loading-text="保存中">
    <el-table :data="tableDataView" stripe style="width: 100%">
      <el-table-column type="index"></el-table-column>
      <el-table-column width="100" prop="name" label="名称">
        <template slot-scope="scope">
          <span v-text="scope.row.name"></span>
        </template>
      </el-table-column>
      <el-table-column width="400" prop="addr" label="矿池地址">
        <template slot-scope="scope">
          <span v-show="!isEdit" v-text="scope.row.addr"></span>
          <el-input v-show="isEdit" placeholder="请输入矿池地址" v-model.trim="scope.row.addr" size="small"></el-input>
        </template>
      </el-table-column>
      <el-table-column width="200" prop="miner_addr" label="矿工名">
        <template slot-scope="scope">
          <span v-show="!isEdit" v-text="scope.row.miner_addr"></span>
          <el-input v-show="isEdit" placeholder="请输入矿工名" v-model.trim="scope.row.miner_addr" size="small"></el-input>
        </template>
      </el-table-column>
      <el-table-column width="200" prop="password" label="密码">
        <template slot-scope="scope">
          <span v-show="!isEdit" v-text="scope.row.password"></span>
          <el-input v-show="isEdit" placeholder="请输入密码" v-model.trim="scope.row.password" size="small"></el-input>
        </template>
      </el-table-column>
      <el-table-column prop="status" label="状态"></el-table-column>
      <!-- <el-table-column prop="accept" label="接收"></el-table-column>
      <el-table-column prop="reject" label="拒绝"></el-table-column> -->
    </el-table>
    <div style="text-align: center; margin-top: 15px;">
      <el-button v-show="!isEdit" type="primary" @click="edit">编辑</el-button>
      <el-button v-show="isEdit" @click="cancel">取消</el-button>
      <el-button v-show="isEdit" type="primary" @click="save">保存</el-button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'MinerPool',
  props: {
    tableData: {
      type: Array
    }
  },
  data () {
    return {
      isEdit: false,
      loading: false,
      copy: null
    }
  },
  computed: {
    tableDataView () {
      return this.isEdit ? this.copy : this.tableData
    }
  },
  methods: {
    edit: function () {
      this.copy = JSON.parse(JSON.stringify(this.tableData))
      this.isEdit = true
    },
    cancel: function () {
      this.isEdit = false
    },
    save: function () {
      console.log('save')
      if (JSON.stringify(this.tableData) === JSON.stringify(this.copy)) {
        this.cancel()
        return
      }
      this.loading = true
      let vm = this
      this.$ajax.patch('/v1/miner', {
        patchAction: 'setPool',
        pool1_addr: this.copy[0].addr,
        pool1_miner_addr: this.copy[0].miner_addr,
        pool1_password: this.copy[0].password,
        pool2_addr: this.copy[1].addr,
        pool2_miner_addr: this.copy[1].miner_addr,
        pool2_password: this.copy[1].password,
        pool3_addr: this.copy[2].addr,
        pool3_miner_addr: this.copy[2].miner_addr,
        pool3_password: this.copy[2].password,
        mac: this.copy[0].mac
      })
        .then((response) => {
          if (response.data.result === 'success') {
            this.$emit('refresh', function () {
              vm.isEdit = false
              vm.loading = false
            })
          } else {
            this.$alert('保存失败')
            vm.loading = false
          }
        })
        .catch((error) => {
          this.$alert(error.message)
          vm.loading = false
        })
    }
  },
  mounted () {
  }
}
</script>
