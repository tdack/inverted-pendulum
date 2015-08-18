$(document).ready(function() {

  var ws;
  var interval = [];

  var WS_CONNECTING = 0;
  var WS_OPEN = 1;
  var WS_CLOSING = 2;
  var WS_CLOSED = 3;

  function sendWSMsg(msg) {
    var msgJSON = JSON.stringify(msg);
    if (ws.readyState  == WS_OPEN) {
      ws.send(msgJSON);
    }
  }

  var param = $(".param");
  param.slider();
  param.slider('disable', true);
  param.on('slideStart', function(ev){
    this.dataset.origValue = this.value;
  });
  param.on('slideStop', function(ev){
    if (this.dataset.origValue != this.value) {
      sendWSMsg({ "action": "set", "param": this.id, "value": this.value });
    }
  });
  $("#switch-state").bootstrapSwitch();

  $('#pendulumGauge').jqxGauge({
      min: -60,
      max: 60,
      startAngle: 0,
      endAngle: 180,
      ranges: [ { startValue: -60, endValue: -45, style: { fill: '#FC6A6A', stroke: '#FC6A6A' }, endWidth: 15, startWidth: 20},
                { startValue: -45, endValue: -25, style: { fill: '#FCA76A', stroke: '#FCA76A' }, endWidth: 10, startWidth: 15 },
                { startValue: -25, endValue: -5, style: { fill: '#e8fc6a', stroke: '#e8fc6a' }, endWidth: 5, startWidth: 10 },
                { startValue: -5, endValue: 5, style: { fill: '#7cf47d', stroke: '#7cf47d' }, endWidth: 5, startWidth: 5 },
                { startValue: 5, endValue: 25, style: { fill: '#e8fc6a', stroke: '#e8fc6a' }, endWidth: 10, startWidth: 5 },
                { startValue: 25, endValue: 45, style: { fill: '#FCA76A', stroke: '#FCA76A' }, endWidth: 15, startWidth: 10 },
                { startValue: 45, endValue: 60, style: { fill: '#FC6A6A', stroke: '#FC6A6A' }, endWidth: 20, startWidth: 15}],
      ticksMinor: { interval: 5, size: '5%' },
      ticksMajor: { interval: 10, size: '9%' },
      border: { visible: false, showGradient: false },
      caption: { value: '0°' },
      value: 0,
      colorScheme: 'scheme03',
      animationDuration: 700
  });

  var btnConnect = $("#connect");
  btnConnect.bootstrapSwitch();
  btnConnect.on('switchChange.bootstrapSwitch', function(event, state) {
    if (state) {
      // console.log("Opening new websocket");
      ws = new WebSocket("ws://beaglebone.local:9980");
      // console.log("btnConnect=true - ws: ", ws.readyState);

      ws.onopen = function(evt) {
        // console.log("onopen - ws: ", ws.readyState);
        // Get pendulum position every 500ms
        interval.push(setInterval(function() {
            param.slider('enable', true);
            sendWSMsg({ "action": "get", "param": "pendulum" });
        }, 300));
        // get PID parameters every 5 seconds
        params = ["kp", "ki", "kd"];
        for (var i =0; i < params.length; i++) {
          sendWSMsg({ "action": "get", "param": params[i] });
        }
        param.slider('enable', true);
        interval.push(setInterval(function() {
          params = ["kp", "ki", "kd"];
          for (var i =0; i < params.length; i++) {
            sendWSMsg({ "action": "get", "param": params[i] });
          }
        }, 5000))
      };

      ws.onmessage = function(evt) {
        // console.log("onmessage: ", evt);
        var msg = JSON.stringify(evt.data);
        msg = JSON.parse(JSON.parse(msg));
        if (msg["param"] == "pendulum") {
          //  $("#" + msg["param"]).text(msg["value"]);
          $("#pendulumGauge").jqxGauge({caption: {value: msg["value"] + '°'}});
          $('#pendulumGauge').jqxGauge('value', msg["value"]);
        } else {
          console.log(msg);
          $("#" + msg["param"]).slider('setValue', parseFloat(msg["value"]));
        }
      };

      ws.onerror = function(err) {
        // console.log("onerror - ws: ", ws.readyState);
        console.log(err)
        if (ws.readyState == WS_CONNECTING || ws.readyState == WS_CLOSED) {
          console.log('Urghh!! Something happened');
          btnConnect.bootstrapSwitch('toggleState');
        }
      };
      ws.onclose = function(evt) {
        // console.log("onclose - ws: ", ws.readyState);
        if (evt.type == 'close' && ws.readyState == WS_CLOSED) {
          for (var i = 0; i < interval.length; i++) {
            clearInterval(interval[i]);
          }
          if (btnConnect.bootstrapSwitch('state')) {
              btnConnect.bootstrapSwitch('toggleState');
          }
          param.slider('disable', true);
        }
      }
    } else {
      // console.log("btnConnect=false - ws: ", ws.readyState);
      if (ws.readyState == WS_OPEN) {
        ws.close();
      }
    }
  });

});
