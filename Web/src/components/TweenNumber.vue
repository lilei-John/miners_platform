<template>
  <span class="fz-number-large" v-text="animatedNumber"></span>
</template>

<script>
import TWEEN from 'tween.js'
export default {
  props: {
    number: {
      type: Number,
      default: 0
    },
    precision: 0
  },
  data () {
    return {
      tweenedNumber: 0,
      tween: null
    }
  },
  computed: {
    animatedNumber () {
      return this.tweenedNumber.toFixed(this.precision)
    }
  },
  watch: {
    number (newValue, oldValue) {
      let vm = this
      this.tween = new TWEEN.Tween({number: oldValue})
      this.tween.to({number: newValue}, 500).onUpdate(function () {
        vm.tweenedNumber = this.number
      })
      this.tween.start()
    }
  },
  created () {
    function animate (time) {
      requestAnimationFrame(animate)
      TWEEN.update(time)
    }
    requestAnimationFrame(animate)
  }
}
</script>
