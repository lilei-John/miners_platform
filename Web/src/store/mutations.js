const mutations = {
  SET_BATCH_SELECTION (state, {type, selection}) {
    if (type === 'alert') {
      state.batchAlertSelection = selection
    }
    if (type === 'all') {
      state.batchAllSelection = selection
    }
  }
}

export default mutations
