// https://github.com/pebble-examples/slate-config-example/blob/master/src/js/pebble-js-app.js

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  var settings=localStorage.getItem("settings");
  settings='{"101":"Tremadol|1452850614|8","102":"Ibuprofin|1452841367|12","103":"Zopiclone|1452813635|24","104":"Omeprazole|1452841369|24","KEY_SORT":1,"KEY_MEDICATIONS":"Paracetamol|1452850611|6","KEY_MODE":2,"KEY_ALARM":1}';
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