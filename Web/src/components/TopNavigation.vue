<template>
  <el-breadcrumb separator="/" v-if="notHome">
    <el-breadcrumb-item :to="{ path: '/' }" v-if="notHome">首页</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('MinerDtail')">矿机详情</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('BatchConfiguration')">批量配置</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('BatchControl')">批量控制</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('BatchUpgrade')">批量升级</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('PlatformConfiguration')">平台配置</el-breadcrumb-item>
    <el-breadcrumb-item v-if="isVisible('AdvancedConfiguration')">其他</el-breadcrumb-item>
  </el-breadcrumb>
</template>

<script>
export default {
  computed: {
    notHome () {
      if (this.$route.name === 'Home') {
        return false
      }
      return true
    }
  },
  methods: {
    isVisible (routeName) {
      if (this.$route.name === routeName) {
        return true
      }
      return false
    }
  }
}
</script>

<style>
</style>
