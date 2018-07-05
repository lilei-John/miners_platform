<template>
  <el-dialog :visible.sync="dialogVisible" :width="width">
    <batch-pools-table @cancel="cancel" @save="save"></batch-pools-table>
  </el-dialog>
</template>

<script>
import BatchPoolsTable from '@/components/BatchPoolsTable.vue'

export default {
  props: {
    visible: {
      type: Boolean,
      default: false
    }
  },
  components: {
    BatchPoolsTable
  },
  computed: {
    dialogVisible: {
      get () {
        return this.visible
      },
      set () {
        this.$emit('cancel')
      }
    }
  },
  data () {
    return {
      width: '1008px'
    }
  },
  methods: {
    cancel () {
      this.$emit('cancel')
    },
    save (pools) {
      this.$emit('save', pools)
    }
  }
}
</script>
