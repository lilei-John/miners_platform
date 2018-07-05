<template>
  <el-table :data="tableData" stripe style="width: 100%">
    <el-table-column type="index"></el-table-column>
    <el-table-column prop="name" label="名称"></el-table-column>
    <el-table-column prop="mhs" label="算力">
      <template slot-scope="scope">
        <span v-text="$util.formatHashrate((scope.row.mhs || 0) * 1024 * 1024).text"></span>
      </template>
    </el-table-column>
    <el-table-column prop="frequency" label="频率"></el-table-column>
    <el-table-column prop="temperature" label="温度"></el-table-column>
    <el-table-column prop="accept" label="接收"></el-table-column>
    <el-table-column prop="reject" label="拒绝"></el-table-column>
  </el-table>
</template>

<script>
export default {
  name: 'MinerBoard',
  props: {
    tableData: {
      type: Array
    }
  },
  data () {
    return {
    }
  },
  mounted () {
  }
}
</script>

<style>
</style>
