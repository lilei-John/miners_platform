// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import ElementUI from 'element-ui'
import './assets/css/base.css'
import 'element-ui/lib/theme-chalk/index.css'
import App from './App'
import router from './router'
import config from './common/config.js'
import axios from 'axios'
import util from './common/util.js'
import store from './store'
import VueCookies from 'vue-cookies'

Vue.config.productionTip = false
Vue.use(ElementUI)

Vue.use(VueCookies)

Vue.prototype.$ajax = axios
axios.defaults.xsrfCookieName = 'csrfToken'
axios.defaults.xsrfHeaderName = 'x-csrf-token'
Vue.prototype.$util = util
window.PublicKeys = config.keys

// router.afterEach((to, from) => {
//   window.scrollTo(0, 0)
// })

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  store,
  components: { App },
  template: '<App/>'
})
