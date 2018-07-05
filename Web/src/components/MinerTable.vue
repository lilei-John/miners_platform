<template>
  <el-table :data="tableDataView" stripe style="width: 100%" v-loading="loading" element-loading-text="保存中">
    <!-- <el-table-column prop="num" label="序号"></el-table-column> -->
    <el-table-column type="index"></el-table-column>
    <el-table-column prop="type" label="型号"></el-table-column>
    <el-table-column prop="version" label="版本"></el-table-column>
    <el-table-column prop="ip" label="IP"></el-table-column>
    <el-table-column prop="status" label="状态"></el-table-column>
    <el-table-column prop="number" label="编号">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.number }}</span>
        <el-input v-show="isEdit" placeholder="请输入矿机编号" v-model.trim="scope.row.number" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column prop="position" label="位置">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.position }}</span>
        <el-input v-show="isEdit" placeholder="请输入矿机位置" v-model.trim="scope.row.position" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column label="操作">
      <template slot-scope="scope">
        <el-button v-show="!isEdit" @click="editMiner(scope.row.id)" type="text" size="small">编辑</el-button>
        <el-button v-show="isEdit" @click="cancel" type="text" size="small">取消</el-button>
        <el-button v-show="isEdit" @click="save(scope.row.id)" type="text" size="small">保存</el-button>
      </template>
    </el-table-column>
  </el-table>
</template>

<script>
export default {
  props: {
    tableData: {
      type: Array
    }
  },
  data () {
    return {
      minerStatusMap: window.PublicKeys.minerStatus,
      isEdit: false,
      copy: null,
      loading: false
    }
  },
  computed: {
    tableDataView () {
      return this.isEdit ? [this.copy] : this.tableData
    }
  },
  methods: {
    editMiner (id) {
      this.copy = Object.assign({}, this.tableData[0])
      this.isEdit = true
    },
    cancel () {
      this.isEdit = false
    },
    save (id) {
      let isPositionNotChange = this.tableData[0].position && (this.copy.position.trim() === this.tableData[0].position.trim())
      let isNumberNotChange = this.tableData[0].number && (this.copy.number.trim() === this.tableData[0].number.trim())

      if (isPositionNotChange && isNumberNotChange) {
        this.cancel()
        return
      }

      this.loading = true
      let vm = this
      let params = {
        patchAction: 'setPosition',
        id: this.copy.id,
        ip: this.copy.ip,
        mac: this.copy.mac
      }
      if (!isPositionNotChange) {
        params.position = this.copy.position.trim()
      }
      if (!isNumberNotChange) {
        params.number = this.copy.number.trim()
      }

      if (!isPositionNotChange && isNumberNotChange) { // 只修改了位置
        this.$ajax.patch('/v1/miner', params)
          .then((response) => {
            this.$emit('refresh', function () {
              vm.isEdit = false
              vm.loading = false
            })
          })
          .catch(function (error) {
            alert(error)
            vm.loading = false
          })
      }

      if (!isNumberNotChange && isPositionNotChange) { // 只修改了编号
        params.patchAction = 'setNumber'
        this.$ajax.patch('/v1/miner', params)
          .then((response) => {
            this.$emit('refresh', function () {
              vm.isEdit = false
              vm.loading = false
            })
          })
          .catch(function (error) {
            alert(error)
            vm.loading = false
          })
      }

      if (!isNumberNotChange && !isPositionNotChange) { // 都改了
        let vm = this
        this.$ajax.all([this.$ajax.patch('/v1/miner', params), this.$ajax.patch('/v1/miner', Object.assign({}, params, {patchAction: 'setNumber'}))])
          .then(this.$ajax.spread((positionResp, numberResp) => {
            vm.$emit('refresh', function () {
              vm.isEdit = false
              vm.loading = false
            })
          }))
          .catch(function (error) {
            alert(error)
            vm.loading = false
          })
      }

    }
  }
}
</script>

<style scoped>

</style>
