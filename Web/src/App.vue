<template>
  <el-container id="app">
    <el-header class="wrap bg-header" style="height: 52px;">
      <top-header class="innerwrap"></top-header>
    </el-header>
    <div class="wrap">
      <top-navigation class="innerwrap top-navigation"></top-navigation>
    </div>
    <el-main class="wrap">
      <div class="innerwrap">
        <router-view/>
      </div>
    </el-main>
    <el-footer class="wrap">
     <bottom-footer class="innerwrap"></bottom-footer>
    </el-footer>
  </el-container>
</template>

<script>
import TopHeader from '@/components/TopHeader.vue'
import TopNavigation from '@/components/TopNavigation.vue'
import BottomFooter from '@/components/BottomFooter.vue'

export default {
  name: 'App',
  components: {
    TopHeader,
    BottomFooter,
    TopNavigation
  },
  created () {
    this.$ajax.get(`/v1/system/mineName`)
      .then((response) => {
        let mineName = response.data.mineName
        document.getElementsByTagName('title')[0].innerHTML = mineName + '管理系统'
      })
      .catch((error) => {
        console.log(error)
        this.$alert('获取矿场名字失败')
      })
  }
}
</script>

<style scoped>
  .wrap {
    width: 100%;
    min-width: 1220px;
  }
  .innerwrap {
    width: 1180px;
    margin: 0 auto;
  }
  .bg-header {
    background: #111;
  }
  .top-navigation {
    padding-top: 10px;
  }
</style>

<style>
  #app .el-tabs--border-card>.el-tabs__header {
    background-color: #5e5e5e;
    border-bottom: 1px solid #e4e7ed;
    margin: 0;
  }
  #app .el-tabs--border-card>.el-tabs__header .el-tabs__item {
    color: #fff;
  }

  #app .el-tabs--border-card>.el-tabs__header .el-tabs__item.is-active {
    color: #fff;
    background-color: #df2639;
    border-right-color: transparent;
    border-left-color: transparent;
  }

  #app .el-tabs--border-card>.el-tabs__header .el-tabs__item:not(.is-disabled):hover {
    color: rgba(255, 255, 255, 0.8);
  }

  #app .el-tabs--border-card>.el-tabs__header .el-tabs__item.is-active:hover {
    color: #fff;
  }

  #app .el-tabs__nav-wrap {
    margin-bottom: 0;
  }
</style>
