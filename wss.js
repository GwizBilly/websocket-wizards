const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });

var msg = '1';
var counter = 0;

console.log("waiting for websocket connection (Port:8080)");

wss.on('connection', function connection(ws) {
    console.log("connection event!");
    ws.on('message', function incoming(message) {
      msg = message + " sent back #: " + counter;
      counter++;
        console.log('received: %s', message);
    })
    console.log("now sending");
      ws.send(msg);
});
