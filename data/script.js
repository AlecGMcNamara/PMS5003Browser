// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Create Temperature Chart
var chartPMS10 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS10' },
  series: [{
    name: 'PMS1.0'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    }
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
  
// Create Humidity Chart
var chartPMS25 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS25' },
  series: [{
    name: 'PMS2.5'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
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

// Create Humidity Chart
var chartPMS100 = new Highcharts.Chart({
  chart:{ renderTo:'chart-PMS100' },
  series: [{
    name: 'PMS10'
  }],
  title: { text: undefined},
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
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
function plotPMS10(value) {
  var x = (new Date()).getTime()
  var y = Number(value);
  if(chartPMS10.series[0].data.length > 40) {
    chartPMS10.series[0].addPoint([x, y], true, true, true);
  } else {
    chartPMS10.series[0].addPoint([x, y], true, false, true);
  }
}

//Plot chart PMS25
function plotPMS25(value) {
  var x = (new Date()).getTime()
  var y = Number(value);
  if(chartPMS25.series[0].data.length > 40) {
    chartPMS25.series[0].addPoint([x, y], true, true, true);
  } else {
    chartPMS25.series[0].addPoint([x, y], true, false, true);
  }
}

//Plot chart PMS100
function plotPMS100(value) {
  var x = (new Date()).getTime()
  var y = Number(value);
  if(chartPMS100.series[0].data.length > 40) {
    chartPMS100.series[0].addPoint([x, y], true, true, true);
  } else {
    chartPMS100.series[0].addPoint([x, y], true, false, true);
  }
}

// Function to get current readings on the webpage when it loads for the first time
function getReadings() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var PMS10 = myObj.PMS10;
      var PMS25 = myObj.PMS25;
      var PMS100 = myObj.PMS100;
      plotPMS10(PMS10);
      plotPMS25(PMS25);
      plotPMS100(PMS100);
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    plotPMS10(myObj.PMS10);
    plotPMS25(myObj.PMS25);
    plotPMS100(myObj.PMS100);
  }, false);
}