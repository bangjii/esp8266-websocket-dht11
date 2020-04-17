const WebSocket = require('ws');
const wss = new WebSocket.Server({port:8788});
wss.on('connection', function connection(ws){
  console.log("Websocket ready!");
	ws.on('message', function incoming(message){
    var d = Date(Date.now());
		console.log('received: %s', message);
    ws.send(message +" at: " + d.toString());
	});	
});	
