const { Client, Server } = require('node-osc');
var socket = require('socket.io-client')('https://gatekeeper.davidjonas.art');

socket.on('connect', function(){console.log("Connected to SocketIO");});
socket.on('disconnect', function(){console.log("Disconnected from SocketIO");});

var server = new Server(3333, '0.0.0.0');

server.on('listening', () => {
  console.log('OSC Server is listening.');
})

server.on('message', (msg) => {
  var blobs = JSON.parse(msg[1]);
  socket.emit("touch", blobs);
});
