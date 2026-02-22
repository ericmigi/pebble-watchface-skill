var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

const moddableProxy = require("@moddable/pebbleproxy");
Pebble.addEventListener('ready', moddableProxy.readyReceived);
Pebble.addEventListener('appmessage', moddableProxy.appMessageReceived);
