<template>
  <el-table :data="tableDataView" stripe style="width: 100%" v-loading="loading" element-loading-text="保存中">
    <el-table-column type="index"></el-table-column>
    <el-table-column width="140" prop="ip_type" label="Protocol">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.ip_type }}</span>
        <el-select v-show="isEdit" v-model.trim="scope.row.ip_type">
          <el-option v-for="item in options" :key="item.value" :label="item.label" :value="item.value"></el-option>
        </el-select>
      </template>
    </el-table-column>
    <el-table-column prop="ip" label="IP">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.ip }}</span>
        <el-input v-show="isEdit" placeholder="请输入ip地址" v-model.trim="scope.row.ip" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column prop="netmask" label="Netmask">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.netmask }}</span>
        <el-input v-show="isEdit" placeholder="请输入netmask" v-model.trim="scope.row.netmask" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column prop="gateway" label="Gateway">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.gateway }}</span>
        <el-input v-show="isEdit" placeholder="请输入gateway" v-model.trim="scope.row.gateway" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column prop="dns" label="DNS">
      <template slot-scope="scope">
        <span v-show="!isEdit">{{ scope.row.dns }}</span>
        <el-input v-show="isEdit" placeholder="请输入DNS server" v-model.trim="scope.row.dns" size="small"></el-input>
      </template>
    </el-table-column>
    <el-table-column label="操作">
      <template slot-scope="scope">
        <el-button v-show="!isEdit" @click="edit" type="text" size="small">编辑</el-button>
        <el-button v-show="isEdit" @click="cancel" type="text" size="small">取消</el-button>
        <el-button v-show="isEdit" @click="save" type="text" size="small">保存</el-button>
      </template>
    </el-table-column>
  </el-table>
</template>

<script>
export default {
  name: 'MinerNetwork',
  props: {
    tableData: {
      type: Array
    }
  },
  data () {
    return {
      isEdit: false,
      copy: null,
      loading: false,
      options: [
        { value: 'dhcp', label: 'dhcp' },
        { value: 'static', label: 'static' }
      ]
    }
  },
  computed: {
    tableDataView () {
      return this.isEdit ? [this.copy] : this.tableData
    }
  },
  methods: {
    edit () {
      this.copy = Object.assign({}, this.tableData[0])
      this.isEdit = true
    },
    cancel () {
      this.isEdit = false
    },
    save () {
      console.log('save')
      const checkip = this.$util.checkip
      if (this.copy.ip_type === 'static') {
        if (!checkip(this.copy.ip)) {
          this.$alert('请输入正确的 IP 地址')
          return
        }
        if (!checkip(this.copy.netmask)) {
          this.$alert('请输入正确的 netmask')
          return
        }
        if (!checkip(this.copy.gateway)) {
          this.$alert('请输入正确的 gateway')
          return
        }
        if (!checkip(this.copy.dns)) {
          this.$alert('请输入正确的 dns')
          return
        }
      }
      this.loading = true
      let vm = this
      this.$ajax.patch('/v1/miner', {
        patchAction: 'setNetwork',
        mac: this.copy.mac,
        ip_type: this.copy.ip_type,
        ip: this.copy.ip,
        netmask: this.copy.netmask,
        gateway: this.copy.gateway,
        dns: this.copy.dns
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
