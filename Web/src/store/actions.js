export const setBatchSelection = function ({commit, state}, {type, selection}) {
  commit('SET_BATCH_SELECTION', {type, selection})
}
