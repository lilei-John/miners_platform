<template>
  <div class="miner-talbe-list">
    <div class="clearfix">
      <div class="fl">
        <el-input :placeholder="searchTextPlaceholder" v-model.trim="searchText" @keyup.enter.native="search" class="input-with-select" >
          <el-select v-model="select" slot="prepend" :class="{'search-select': !isMhsSelected, 'search-select-number':isNumberSelected, 'search-select-mhs':isMhsSelected}" @change="selectChange">
            <el-option label="IP" value="ip"></el-option>
            <el-option label="位置" value="position"></el-option>
            <el-option label="设备条码" value="number"></el-option>
            <el-option label="状态" value="status"></el-option>
            <el-option label="版本" value="version"></el-option>
            <el-option label="平均算力(mhs) >" value="mhsg"></el-option>
            <el-option label="平均算力(mhs) <" value="mhsl"></el-option>
          </el-select>
          <div slot="append" style="display: flex;">
            <span v-if="multipleSelect && isIpSelected" v-show="!showSearchTextNext" class="el-icon-d-arrow-right clr-danger" @click="arrowRight"></span>
            <span v-if="multipleSelect && isIpSelected" v-show="showSearchTextNext" class="el-icon-d-arrow-left clr-danger" @click="arrowLeft"></span>
            <el-input v-if="isIpSelected" placeholder="192.168.6.241" v-show="showSearchTextNext" class="search-text-next" v-model.trim="searchTextNext"></el-input>
            <el-button v-show="!(isIpSelected && showSearchTextNext)" class="search-btn" icon="el-icon-search" @click="search"></el-button>
            <el-button v-show="isIpSelected && showSearchTextNext" class="search-btn" icon="el-icon-plus" @click="addIpRange"></el-button>
          </div>
        </el-input>
      </div>
      <div class="fr">
        <slot name="operation"></slot>
        <el-button type="primary" @click="reset">重置</el-button>
        <el-button v-if="exportVisible" type="primary" @click="exportTable">导出</el-button>
      </div>
    </div>

    <el-table :fit="true" :data="tableData" stripe class="mtl-table-container" @selection-change="handleSelectionChange">
      <el-table-column v-if="multipleSelect" type="selection" width="30"></el-table-column>
      <el-table-column type="index" width="38px"></el-table-column>
      <el-table-column prop="type" label="型号" width="76"></el-table-column>
      <el-table-column prop="version" label="版本" width="102"></el-table-column>
      <el-table-column prop="ip" label="IP" width="116"></el-table-column>
      <el-table-column prop="status" label="状态" width="82">
        <template slot-scope="scope">
          <span :class="{'clr-danger': scope.row.status !== 'running'}" v-text="scope.row.status"></span>
        </template>
      </el-table-column>
       <el-table-column prop="mhs" label="当前算力" width="106">
        <template slot-scope="scope">
          <span v-text="$util.formatHashrate(scope.row.mhs * 1024 * 1024).text"></span>
        </template>
      </el-table-column>
      <el-table-column prop="avg_mhs" label="平均算力" width="106">
        <template slot-scope="scope">
          <span v-text="$util.formatHashrate(scope.row.avg_mhs * 1024 * 1024).text"></span>
        </template>
      </el-table-column>
      <el-table-column prop="number" label="设备条码"></el-table-column>
      <el-table-column prop="position" label="位置"></el-table-column>
      <el-table-column label="运行时间">
        <template slot-scope="scope">
          <span v-text="$util.formatDuration(scope.row.duration)"></span>
        </template>
      </el-table-column>
      <el-table-column label="操作" width="50">
        <template slot-scope="scope">
          <el-button @click="viewMiner(scope.row.ip, scope.row.mac)" type="text" size="small">查看</el-button>
        </template>
      </el-table-column>
      <el-table-column v-if="showProgress" prop="progress" label="进度">
        <template slot-scope="scope">
          <el-progress v-show="scope.row.percentage !== undefined" :text-inside="true" :stroke-width="18" :percentage="scope.row.percentage"></el-progress>
        </template>
      </el-table-column>
    </el-table>
    <div class="mtl-pagination-container" v-show="total > 0">
      <el-pagination :current-page="currentPage" @size-change="handleSizeChange" @current-change="handleCurrentChange" :page-size="pageSize" :page-count="11" layout="prev, pager, next, jumper" :total="total"></el-pagination>
    </div>
  </div>
</template>

<script>
import {mapActions} from 'vuex'

export default {
  props: {
    minerType: { // alert all
      type: String,
      default: ''
    },
    tableData: {
      type: Array
    },
    pageSize: {
      type: Number,
      default: 10
    },
    total: {
      type: Number,
      default: 0
    },
    multipleSelect: {
      type: Boolean,
      default: false
    },
    showProgress: {
      type: Boolean,
      default: false
    },
    exportVisible: {
      type: Boolean,
      default: true
    }
  },
  data () {
    return {
      searchText: '',
      searchTextNext: '',
      showSearchTextNext: false,
      minerStatusMap: window.PublicKeys.minerStatus,
      select: 'ip',
      currentPage: 1,
      multipleSelection: [],
      isIpSelected: true
    }
  },
  computed: {
    searchQueryText () {
      if (this.searchText) {
        return this.select + '=' + this.searchText
      }
      return ''
    },
    searchTextPlaceholder () {
      // ip position number status version mhsg mhsl
      let text = '请输入搜索内容'
      if (this.showSearchTextNext) {
        text = '请输入起始IP'
      }
      console.log(this.select)
      switch (this.select) {
        case 'position':
          text = '1-10-1'
          break
        case 'number':
          text = '1234567'
          break
        case 'status':
          text = `offline (状态有'offline', 'running', 'error', 'low mhs', 'ip conflict')`
          break
        case 'version':
          text = 'L21'
          break
        case 'mhsg':
          text = '123'
          break
        case 'mhsl':
          text = '123'
          break
        case 'ip':
          text = '192.168.6.42'
          break
      }
      return text
    },
    isMhsSelected () {
      return this.select === 'mhsg' || this.select === 'mhsl'
    },
    isNumberSelected () {
      return this.select === 'number'
    }
  },
  methods: {
    ...mapActions({
      setBatchSelection: 'setBatchSelection'
    }),
    viewMiner (ip, mac) {
      this.$router.push(`/miner-detail?mac=${mac}`)
    },
    selectChange (val) {
      console.log(val)
      if (val !== 'ip') {
        this.isIpSelected = false
      } else {
        this.isIpSelected = true
      }
    },
    search () {
      if (this.isIpSelected && this.showSearchTextNext) { // 如果是选择IP 且点开了ip范围 则不搜索
        return
      }
      this.currentPage = 1
      this.$emit('search', this.searchQueryText)
    },
    arrowRight () {
      this.showSearchTextNext = true
    },
    arrowLeft () {
      this.showSearchTextNext = false
    },
    addIpRange () {
      let startIp = this.searchText
      let endIp = this.searchTextNext
      let startIpLegal = this.$util.checkip(startIp)
      let endIpLegal = this.$util.checkip(endIp)
      if (!startIpLegal || !endIpLegal) {
        let msg1 = !startIpLegal ? '起始IP' : ''
        let msg2 = !endIpLegal ? '结束IP' : ''
        let msg = msg1 + (msg1 === '' ? '' : '和') + msg2 + '格式错误!'

        this.$alert(msg, '提示')
        return
      }

      let compareResult = this.$util.compareIP(startIp, endIp)
      let result = startIp
      if (compareResult < 0) result += '~' + endIp
      else if (compareResult > 0) result = endIp + '~' + startIp

      this.$emit('addIpRange', result)
    },
    reset () {
      this.searchText = ''
      this.search()
    },
    exportTable () {
      let type = this.minerType
      window.open(`/v1/excel/miners/${type}/`)
    },
    handleSizeChange (val) {
      console.log(`每页 ${val} 条`)
    },
    handleCurrentChange (val) {
      console.log(`当前页: ${val}`)
      this.currentPage = val
      this.$emit('handleCurrentChange', val, this.searchQueryText)
    },
    handleSelectionChange (val) {
      this.multipleSelection = val
      this.setBatchSelection({type: this.minerType, selection: this.multipleSelection})
    }
  },
  beforeDestroy () {
    if (this.multipleSelect) {
      this.setBatchSelection({type: this.minerType, selection: []})
    }
  }
}
</script>

<style>
  .mtl-table-container {
    margin-bottom: 10px;
  }
  .mtl-pagination-container {
    text-align: center;
  }
  .input-with-select > .el-input__inner {
    width: 440px;
  }
  .search-text-next .el-input__inner {
    width: 200px;
  }
  .input-with-select .el-input-group__prepend {
    background: #fff;
  }
  .input-with-select .el-input-group__append {
    padding: 0;
  }
  .input-with-select .el-input-group__append .search-btn {
    margin: 0 0 0 -1px;
    border-left: 1px solid #dcdfe6;
    border-radius: 0;
  }
  .input-with-select .el-input-group__append .el-icon-d-arrow-right,
  .input-with-select .el-input-group__append .el-icon-d-arrow-left {
    padding: 12px 5px;
    cursor: pointer;
  }
  .search-select {
    width: 80px;
  }
  .search-select-mhs {
    width: 160px;
  }
  .search-select-number {
    width: 120px;
  }
  .input-with-select .el-input-group__append .search-text-next {
    margin: -1px;
  }

  .miner-talbe-list .el-table .cell,
  .miner-talbe-list .el-table th div,
  .miner-talbe-list .el-table--border td:first-child .cell,
  .miner-talbe-list .el-table--border th:first-child .cell {
    padding-left: 5px;
  }

  .miner-talbe-list .el-table .cell,
  .miner-talbe-list .el-table th div{
    padding-right: 5px;
  }
</style>
