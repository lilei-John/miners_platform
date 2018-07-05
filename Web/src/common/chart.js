import {getBrowser} from './browser.js'

function exportPic (myChart, title = 'spline') {
  let picInfo = myChart.getConnectedDataURL({
    type: 'png',
    backgroundColor: '#fff',
    pixelRatio: 1,
    excludeComponents: [
      'toolbox'
    ]
  })

  let a = document.createElement('a')
  a.download = title + '.png'
  a.target = '_blank'
  a.href = picInfo

  let browserInfo = getBrowser()

  if (typeof MouseEvent === 'function' && browserInfo.browser !== 'IE' && browserInfo.browser !== 'Edge') {
    console.log(browserInfo)
    let evt = new MouseEvent('click', {
      view: window,
      bubbles: true,
      cancelable: false
    })
    a.dispatchEvent(evt)
  } else { // IE
    if (window.navigator.msSaveOrOpenBlob) {
      var bstr = atob(picInfo.split(',')[1])
      var n = bstr.length
      var u8arr = new Uint8Array(n)

      while (n--) {
        u8arr[n] = bstr.charCodeAt(n)
      }

      var blob = new Blob([u8arr])
      window.navigator.msSaveOrOpenBlob(blob, 'hashrate-spline.png')
    } else {
      var lang = '右键另存为图片'
      var html = '' + '<body style="margin:0;">' + '<img src="' + picInfo + '" style="max-width:100%;" title="' + lang + '" />' + '</body>'
      var tab = window.open()
      tab.document.write(html)
    }
  }
}

export default {
  exportPic: exportPic
}
