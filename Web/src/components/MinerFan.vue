<template>
  <el-table :data="tableData" stripe style="width: 100%">
    <el-table-column type="index"></el-table-column>
    <el-table-column prop="name" label="名称"></el-table-column>
    <el-table-column prop="speed" label="转速"></el-table-column>
    <el-table-column prop="status" label="状态"></el-table-column>
  </el-table>
</template>

<script>
export default {
  name: 'MinerFan',
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
