// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  var settings=localStorage.getItem("settings");
  //settings='{"101":"Ibuprofen|1454302849|6|1","102":"Tremadol|1454306400|8|1","103":"Omeprazole|1454265012|24|1","KEY_APP_VERSION":"1.4","KEY_SORT":1,"KEY_MEDICATIONS":"Paracetamol|1454292053|6|1","KEY_VERSION":3,"KEY_MODE":2,"KEY_TIMESTAMP":1454311036,"KEY_ALARM":0}';
  //settings='{"KEY_MEDICATIONS":"Paracetamol|1454292053|6|1","101":"Ibuprofen|1454302849|6|1","102":"Tremadol|1454306400|8|1","103":"Omeprazole|1454265012|24|1","KEY_APP_VERSION":"1.4","KEY_SORT":1,"KEY_VERSION":3,"KEY_MODE":2,"KEY_TIMESTAMP":1454311036,"KEY_ALARM":0}';
  var dict=settings ? JSON.parse(settings) : {};
  if (!dict.KEY_TIMESTAMP) {
    var d=new Date();$
    dict.KEY_TIMESTAMP = Math.floor(d.getTime()/1000 - d.getTimezoneOffset()*60);
  }
  sendDict(dict);
});

Pebble.addEventListener("appmessage", function(e) {
  console.log("Received message: " + JSON.stringify(e.payload));
  if (e.payload.KEY_EXPORT) {
    delete e.payload.KEY_EXPORT;
    localStorage.setItem("settings",JSON.stringify(e.payload));
    showConfiguration();
  } else {
    localStorage.setItem("settings",JSON.stringify(e.payload));
  }
  createAllPins(e.payload,0);
});

function sendDict(dict) {
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
}

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
    var d;
    var secs;
    while (setting=(med===0) ? settings.KEY_MEDICATIONS : settings[100+med]) {
      setting=setting.split("|");
      d = new Date(0); // The 0 there is the key, which sets the date to the epoch
      d.setUTCSeconds(setting[1]*1.0 +setting[2]*3600);  // setting[1] = last time, setting[2] = repeat_hrs
      secs=d.getSeconds();
      /*d.setUTCSeconds(d.getTimezoneOffset()*60); // this sets seconds to zero */
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
  var d=new Date();
  dict.KEY_TIMESTAMP = Math.floor(d.getTime()/1000 /*- d.getTimezoneOffset()*60 */);
  
  var med=0;
  var data;
  var hhmm;
  var secs;
  while (configData["med_"+med]) {
    data=decodeURIComponent(configData["med_"+med]).split("|");
    hhmm=data[1].split(":");
    d = new Date(); // The 0 there is the key, which sets the date to the epoch
    d.setHours(hhmm[0]);
    d.setMinutes(hhmm[1]);
    d.setSeconds(data[4]);
    d.setMilliseconds(0);
    if (d < new Date()) d.setHours(hhmm[0]*1.0 + 24);
    secs=d.getTime()/1000;
    data[1]=secs /*- d.getTimezoneOffset()*60 */- data[2]*3600;
    dict[100+med]=data.join("|");
    med++;
  }

  // Send to watchapp
  sendDict(dict);
});

/****************************** Custom Timeline Stuff ************************/

// Pin template
var pin = {
//  "id": "example-pin-0",
//  "time": date.toISOString(),
  "layout": {
    "type": "genericPin",
//    "title": "Take Ibuprofen",
    "tinyIcon": "system://images/NOTIFICATION_REMINDER"
  },
  "reminders": [
    {
//      "time": date.toISOString(),
      "layout": {
      "type": "genericReminder",
        "tinyIcon": "system://images/NOTIFICATION_REMINDER",
//        "title": "Take Ibuprofen"
      }
    }
  ],
  "actions": [
    {
      "title": "Med Taken",
      "type": "openWatchApp",
      "launchCode": 10 /*+med number*/
    },
    {
      "title": "Open Meds Timer",
      "type": "openWatchApp",
      "launchCode": 0
    }
  ],
};

var last_pin=0;
var total_pins=0;
var last_total_pins=0;
var dict_copy=null;
var pin_prefix="MedTimer-pin-";

function pinInsertedCallback(responseText) { 
  console.log('insertUserPin Result: ' + responseText);
  createAllPins(dict_copy,last_pin+1);
}

function createAllPins(dict,next_pin) {
  if (!dict) return;
  dict_copy=dict;
  if (next_pin===0) {
    last_total_pins=localStorage.getItem("pins")*1.0;
    last_total_pins=8;
    total_pins=0;
  }
  last_pin=next_pin;
  var setting;
  setting=(last_pin===0) ? dict.KEY_MEDICATIONS : dict[100+last_pin];
  if (setting) {
    // get the time
    setting=setting.split("|");
    var d = new Date(0); // The 0 there is the key, which sets the date to the epoch
    d.setUTCSeconds(setting[1]*1.0 +setting[2]*3600); // setting[1] = last time, setting[2] = repeat_hrs
    d.setSeconds(0);
    d.setMilliseconds(0);
    //var secs=d.getSeconds();
    /*d.setUTCSeconds(d.getTimezoneOffset()*60); // this sets seconds to zero */
    //setting[1]=padZero(d.getHours())+":"+padZero(d.getMinutes());
    //setting.push(secs);
    
    // configure the pin
    pin.id=pin_prefix+last_pin;
    pin.time=pin.reminders[0].time=d.toISOString();
    pin.layout.title=pin.reminders[0].layout.title="Take "+setting[0];
    pin.actions[0].launchCode=10+last_pin;
    total_pins++;
    // Push the pin
    console.log('Inserting pin in the future: ' + JSON.stringify(pin));
    insertUserPin(pin, pinInsertedCallback);
  } else {
    // no more meds to send
    localStorage.setItem("pins",total_pins);
    console.log("remembering pins: "+total_pins);
    // delete any old pins
    deleteAllPins(last_pin);
  }
}

function pinDeletedCallback(responseText) { 
  console.log('deleteUserPin Result: ' + responseText);
  deleteAllPins(last_pin+1);
} 

function deleteAllPins(next_pin) {
  last_pin=next_pin;
  if (last_pin<last_total_pins) {
    console.log("Deleting pin: "+pin_prefix+last_pin);
    deleteUserPin(pin_prefix+last_pin, pinDeletedCallback);
  } else {
    sendDict({KEY_PINS_DONE:1});
  }
}


// https://gist.github.com/pebble-gists/6a4082ef12e625d23455
/******************************* timeline lib *********************************/

// The timeline public URL root
var API_URL_ROOT = 'https://timeline-api.getpebble.com/';
var usertoken=null;

/**
 * Send a request to the Pebble public web timeline API.
 * @param pin The JSON pin to insert. Must contain 'id' field.
 * @param type The type of request, either PUT or DELETE.
 * @param topics Array of topics if a shared pin, 'null' otherwise.
 * @param apiKey Timeline API key for this app, available from dev-portal.getpebble.com
 * @param callback The callback to receive the responseText after the request has completed.
 */
function timelineRequest(pin, type, topics, apiKey, callback) {
  // User or shared?
  var url = API_URL_ROOT + 'v1/' + ((topics !== null) ? 'shared/' : 'user/') + 'pins/' + pin.id;

  // Create XHR
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    console.log('timeline: response received: ' + this.responseText);
    callback(this.responseText);
  };
  xhr.onerror=function(error) {
    console.log('timeline: error: ' +JSON.stringify(error));
    callback("error");
  };
  xhr.open(type, url);

  // Set headers
  xhr.setRequestHeader('Content-Type', 'application/json');
  if(topics !== null) {
    xhr.setRequestHeader('X-Pin-Topics', '' + topics.join(','));
    xhr.setRequestHeader('X-API-Key', '' + apiKey);
  }

  // Get token
  if (usertoken) {
    send(xhr,pin);
  } else {
    Pebble.getTimelineToken(function(token) {
      usertoken=token;
      send(xhr,pin);
    }, function(error) { console.log('timeline: error getting timeline token: ' + error); });
  }
}

function send(xhr,pin)  {
  // Add headers
  xhr.setRequestHeader('X-User-Token', '' + usertoken);
  // Send
  xhr.send(JSON.stringify(pin));
  console.log('timeline: request sent. ' + usertoken);
}

/**
 * Insert a pin into the timeline for this user.
 * @param pin The JSON pin to insert.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function insertUserPin(pin, callback) {
  timelineRequest(pin, 'PUT', null, null, callback);
}

/**
 * Delete a pin from the timeline for this user.
 * @param pin The JSON pin to delete.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function deleteUserPin(pin, callback) {
  timelineRequest(pin, 'DELETE', null, null, callback);
}

/**
 * Insert a pin into the timeline for these topics.
 * @param pin The JSON pin to insert.
 * @param topics Array of topics to insert pin to.
 * @param apiKey Timeline API key for this app, available from dev-portal.getpebble.com
 * @param callback The callback to receive the responseText after the request has completed.
 */
function insertSharedPin(pin, topics, apiKey, callback) {
  timelineRequest(pin, 'PUT', topics, apiKey, callback);
}

/**
 * Delete a pin from the timeline for these topics.
 * @param pin The JSON pin to delete.
 * @param topics Array of topics to delete pin from.
 * @param apiKey Timeline API key for this app, available from dev-portal.getpebble.com
 * @param callback The callback to receive the responseText after the request has completed.
 */
function deleteSharedPin(pin, topics, apiKey, callback) {
  timelineRequest(pin, 'DELETE', topics, apiKey, callback);
}

/****************************** end timeline lib ******************************/