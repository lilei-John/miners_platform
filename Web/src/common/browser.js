export function getBrowser () {
  let sys = {}
  let ua = navigator.userAgent.toLowerCase()
  let regs = {
    edge: /edge\/([\d.]+)/,
    rv: /rv:([\d.]+)\) like gecko/,
    msie: /msie ([\d.]+)/,
    firefox: /firefox\/([\d.]+)/,
    chrome: /chrome\/([\d.]+)/,
    opera: /opera.([\d.]+)/,
    safari: /version\/([\d.]+).*safari/
  }

  if (regs.edge.test(ua)) sys.edge = ua.match(regs.edge)[1]
  else if (regs.rv.test(ua)) sys.ie = ua.match(regs.rv)[1]
  else if (regs.msie.test(ua)) sys.ie = ua.match(regs.msie)[1]
  else if (regs.firefox.test(ua)) sys.firefox = ua.match(regs.firefox)[1]
  else if (regs.chrome.test(ua)) sys.chrome = ua.match(regs.chrome)[1]
  else if (regs.opera.test(ua)) sys.opera = ua.match(regs.opera)[1]
  else if (regs.safari.test(ua)) sys.safari = ua.match(regs.safari)[1]

  if (sys.edge) return { broswer: 'Edge', version: sys.edge }
  if (sys.ie) return { broswer: 'IE', version: sys.ie }
  if (sys.firefox) return { broswer: 'Firefox', version: sys.firefox }
  if (sys.chrome) return { broswer: 'Chrome', version: sys.chrome }
  if (sys.opera) return { broswer: 'Opera', version: sys.opera }
  if (sys.safari) return { broswer: 'Safari', version: sys.safari }
  return { broswer: '', version: '0' }
}
