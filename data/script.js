var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
setInterval(sendMessage, 10000);

window.addEventListener('load', onload);
function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');   
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    var jsonReceived = JSON.parse(event.data);
    //set objects on webpage after receiving message from server
    console.log('Browser received message');
    console.log(jsonReceived);
    plotPMS10(jsonReceived.ReadingTime, jsonReceived.PMS10);
    plotPMS25(jsonReceived.ReadingTime, jsonReceived.PMS25);
    plotPMS100(jsonReceived.ReadingTime, jsonReceived.PMS100);
}
function sendMessage(event) 
{
    websocket.send(sendJSONstring());
    console.log('Browser sent message');
}
function sendJSONstring(){
    //set up JSON message before sending mesage to server
    var jsonSend = {"ReadingTime":(String)(new Date().getTime()) }; //send readings
    console.log(jsonSend);
    return(JSON.stringify(jsonSend));   
}

// Create PMS1.0 Chart
var chartPMS10 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS10' },
  series: [{
    name: 'PMS1.0'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: false }
    },
    series: { color: '#50b8b4' }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '1.0uM' }
  },
  credits: { enabled: false }
});
  
// Create PMS2.5 Chart
var chartPMS25 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS25' },
  series: [{
    name: 'PMS2.5'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: false }
    },
    series: { color: '#50b8b4' }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '2.5uM' }
  },
  credits: { enabled: false }
});

// Create PMS10 Chart
var chartPMS100 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS100' },
  series: [{
    name: 'PMS10'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: false }
    },
    series: { color: '#50b8b4' }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '10uM' }
  },
  credits: { enabled: false }
});

//Plot chart PMS10
function plotPMS10(date, value) {
  var x = Number(date);
  var y = Number(value);
  if(chartPMS10.series[0].data.length > 100) {
    chartPMS10.series[0].addPoint([x,y], true, true, true);
  } else {
    chartPMS10.series[0].addPoint([x,y], true, false, true);
  }
}

//Plot chart PMS25
function plotPMS25(date, value) {
  var x = Number(date);
  var y = Number(value);
  if(chartPMS25.series[0].data.length > 100) {
    chartPMS25.series[0].addPoint([x, y], true, true, true);
  } else {
    chartPMS25.series[0].addPoint([x, y], true, false, true);
  }
}

//Plot chart PMS100
function plotPMS100(date, value) {
  var x = Number(date);
  var y = Number(value);
  if(chartPMS100.series[0].data.length > 100) {
    chartPMS100.series[0].addPoint([x, y], true, true, true);
  } else {
    chartPMS100.series[0].addPoint([x, y], true, false, true);
  }
}
