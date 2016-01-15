// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  var settings=localStorage.getItem("settings");
  //settings='{"101":"Paracetamol|1452934053|6|0","102":"Ibuprofin|1452934051|12|0","103":"Zopiclone|1452898015|24|1","104":"Omeprazole|1452931452|24|1","KEY_SORT":1,"KEY_MEDICATIONS":"Tremadol|1452913171|8|0","KEY_VERSION":2,"KEY_MODE":2,"KEY_ALARM":1}';
  var dict=settings ? JSON.parse(settings) : {};
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

Pebble.addEventListener("appmessage",	function(e) {
	console.log("Received message: " + JSON.stringify(e.payload));
  localStorage.setItem("settings",JSON.stringify(e.payload));
});