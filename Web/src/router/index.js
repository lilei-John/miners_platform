import Vue from 'vue'
import Router from 'vue-router'
import Home from '@/components/Home'
import MinerDetail from '@/components/MinerDetail'
import BatchConfiguration from '@/components/BatchConfiguration'
import BatchControl from '@/components/BatchControl'
import BatchUpgrade from '@/components/BatchUpgrade'
import PlatformConfiguration from '@/components/PlatformConfiguration'
import AdvancedConfiguration from '@/components/AdvancedConfiguration'

Vue.use(Router)

export default new Router({
  scrollBehavior (to, from, savedPosition) {
    return {x: 0, y: 0}
  },
  routes: [
    {
      path: '/',
      name: 'Home',
      component: Home
    },
    {
      path: '/miner-detail',
      name: 'MinerDtail',
      component: MinerDetail
    },
    {
      path: '/batch-configuration',
      name: 'BatchConfiguration',
      component: BatchConfiguration
    },
    {
      path: '/batch-control',
      name: 'BatchControl',
      component: BatchControl
    },
    {
      path: '/batch-upgrade',
      name: 'BatchUpgrade',
      component: BatchUpgrade
    },
    {
      path: '/platform-configuration',
      name: 'PlatformConfiguration',
      component: PlatformConfiguration
    },
    {
      path: '/advanced-configuration',
      name: 'AdvancedConfiguration',
      component: AdvancedConfiguration
    }
  ]
})
