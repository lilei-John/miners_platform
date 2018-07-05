function pick(array, amount) {
  let len = array.length
  if (len <= amount) {
    return array
  }

  let result = []

  let start = 0
  let end = len - 1

  result.push(array[start])

  let count = 2
  let interval = len / amount

  for (let i = interval; i < len; i += interval) {
    let index = Math.floor(i)
    result.push(array[index])
    count++
    if (count === amount) {
      break
    }
  }

  result.push(array[end])

  return result
}

const dbPeriodMap = {
  day: '1 DAY',
  week: '1 WEEK',
  month: '1 MONTH',
  season: '1 QUARTER',
  half: '2 QUARTER',
  year: '1 YEAR',
}

function pad0(num) {
  let len = num.toString().length
  if (len === 1) {
    num = '0' + num
  }
  return num
}

function fmtDate(dateString) {
  let date = new Date(dateString)
  let y = pad0(date.getFullYear())
  let M = pad0(date.getMonth() + 1)
  let d = pad0(date.getDate())
  let h = pad0(date.getHours())
  let m = pad0(date.getMinutes())
  let s = pad0(date.getSeconds())
  return `${y}-${M}-${d} ${h}:${m}:${s}`
}

function formatDuration(seconds = 0) {
  if (seconds < 60) {
    return seconds + '秒'
  }

  let s = seconds % 60
  let mins = (seconds - s) / 60
  if (mins < 60) {
    return mins + '分' + s + '秒'
  }

  let m = mins % 60
  let hours = (mins - m) / 60
  if (hours < 24) {
    return hours + '小时' + m + '分' + s + '秒'
  }

  let h = hours % 24
  let days = (hours - h) / 24
  return days + '天' + h + '小时' + m + '分' + s + '秒'
}

const pool = {
  rate: 1000,
}

function formatHashrate(hashrate) {
  let i = -1
  var byteUnits = ['KH/S', 'MH/S', 'GH/S', 'TH/S', 'PH/S']
  do {
    hashrate = hashrate / pool.rate
    i++
  } while (hashrate > pool.rate)

  let value = hashrate.toFixed(2)
  let unit = byteUnits[i]
  let text = value + ' ' + unit
  let totalRate = Math.pow(pool.rate, i + 1)
  return {
    value,
    unit,
    text,
    totalRate,
  }
}

module.exports = {
  pick,
  dbPeriodMap,
  formatDate: fmtDate,
  formatHashrate,
  formatDuration,
}