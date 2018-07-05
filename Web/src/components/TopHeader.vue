<template>
  <div class="clearfix"  style="height: 52px;">
    <a class="fl logo">
      <img src="static/logo.png" alt="">
    </a>
    <nav class="fr">
      <ul class="clearfix">
        <li class="fl nav-item">
          <a @click="home" class="link-btn" :class="{'active': isActive('Home') }">首页</a>
        </li>
        <li class="fl nav-item">
          <a @click="batchConfiguration" class="link-btn" :class="{'active': isActive('BatchConfiguration')}">批量配置</a>
        </li>
        <li class="fl nav-item">
          <a @click="batchControl" class="link-btn" :class="{'active': isActive('BatchControl')}">批量控制</a>
        </li>
        <li class="fl nav-item">
          <a @click="batchUpgrade" class="link-btn" :class="{'active': isActive('BatchUpgrade')}">批量升级</a>
        </li>
        <li class="fl nav-item">
          <a @click="platformConfiguration" class="link-btn" :class="{'active': isActive('PlatformConfiguration')}">平台配置</a>
        </li>
        <li class="fl nav-item">
          <a @click="advancedConfiguration" class="link-btn" :class="{'active': isActive('AdvancedConfiguration')}">其他</a>
        </li>
        <!-- <li class="fl nav-item">
          <a class="link-btn">登录</a>
        </li> -->
        <!-- <li class="fl nav-item">
          <a class="link-btn">注册</a>
        </li> -->
      </ul>
    </nav>
  </div>
</template>

<script>
export default {
  methods: {
    isActive (routeName) {
      if (this.$route.name === routeName) {
        return true
      }
      return false
    },
    home () {
      this.$router.push('/')
    },
    batchConfiguration () {
      this.$router.push('/batch-configuration')
    },
    batchControl () {
      this.$router.push('/batch-control')
    },
    batchUpgrade () {
      this.$router.push('/batch-upgrade')
    },
    platformConfiguration () {
      this.$router.push('/platform-configuration')
    },
    advancedConfiguration () {
      this.$router.push('/advanced-configuration')
    }
  }
}
</script>

<style scoped>
  .logo {
    margin-top: 13px;
  }
  .nav-item {
    line-height: 52px;
  }
  .link-btn {
    color: #fff;
    cursor: pointer;
    line-height: 1;
    padding: 8px 16px;
    border-radius: 3px;
    transition: .3s;
  }
  .link-btn:hover {
    text-decoration: none;
  }

  .link-btn.active {
    background: #df2639;
  }

</style>
