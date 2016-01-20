// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  var settings=localStorage.getItem("settings");
  //settings='{"1":2,"2":1,"3":1,"4":3,"5":"1.2","100":"Ibuprofin|1453318208|12|1","101":"Omeprazole|1453275012|24|1","102":"Paracetamol|1453347465|6|0","103":"Tremadol|1453347472|8|0","104":"Zopiclone|1453330855|24|1","KEY_MODE":2,"KEY_ALARM":1,"KEY_SORT":1,"KEY_VERSION":3,"KEY_APP_VERSION":"1.2","KEY_MEDICATIONS":"Ibuprofin|1453318208|12|1"}';
  var dict=settings ? JSON.parse(settings) : {};
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

Pebble.addEventListener("appmessage",	function(e) {
	console.log("Received message: " + JSON.stringify(e.payload));
  if (e.payload.KEY_EXPORT) {
    delete e.payload.KEY_EXPORT;
    localStorage.setItem("settings",JSON.stringify(e.payload));
    showConfiguration();
  } else {
    localStorage.setItem("settings",JSON.stringify(e.payload));
  }
});

function padZero(i) {
  return (i<10 ? "0":"")+i;
}

Pebble.addEventListener('showConfiguration', showConfiguration);

function showConfiguration() {
  var url = 'https://wildhart.github.io/med-timer-config/index.html';
  var settings=localStorage.getItem("settings");
  var config="";
  if (settings) {
    settings=JSON.parse(settings);
    config+="?app_version="+(settings.KEY_APP_VERSION ? settings.KEY_APP_VERSION : "1.2");
    config+="&data_version="+settings.KEY_VERSION;
    config+="&mode="+settings.KEY_MODE;
    config+="&alarm="+settings.KEY_ALARM;
    config+="&sort="+settings.KEY_SORT;
    
    var med=0;
    var setting;
    while (setting=(med===0) ? settings.KEY_MEDICATIONS : settings[100+med]) {
      setting=setting.split("|");
      var d = new Date(0); // The 0 there is the key, which sets the date to the epoch
      d.setUTCSeconds(setting[1]*1.0 +setting[2]*3600);
      var secs=d.getSeconds();
      d.setUTCSeconds(d.getTimezoneOffset()*60); // this sets seconds to zero
      setting[1]=padZero(d.getHours())+":"+padZero(d.getMinutes());
      setting.push(secs);
      config+="&med_"+med+"="+encodeURIComponent(setting.join("|"));
      med++;
    }
  }
  console.log('Showing configuration page: ' + url + config);

  Pebble.openURL(url+config);
}

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var dict = {};
  dict.KEY_CONFIG_DATA  = 1;
  dict.KEY_MODE    = configData.mode;
  dict.KEY_ALARM   = configData.alarm ? 1 : 0;  // Send a boolean as an integer
  dict.KEY_SORT    = configData.sort ? 1 : 0;   // Send a boolean as an integer
  dict.KEY_VERSION = configData.data_version;
  
  var med=0;
  while (configData["med_"+med]) {
    var data=decodeURIComponent(configData["med_"+med]).split("|");
    var hhmm=data[1].split(":");
    var d = new Date(); // The 0 there is the key, which sets the date to the epoch
    d.setHours(hhmm[0]);
    d.setMinutes(hhmm[1]);
    d.setSeconds(data[4]);
    d.setMilliseconds(0);
    if (d < new Date()) d.setHours(hhmm[0]*1.0 + 24);
    var secs=d.getTime()/1000;
    data[1]=secs - d.getTimezoneOffset()*60 - data[2]*3600;
    dict[100+med]=data.join("|");
    med++;
  }

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});