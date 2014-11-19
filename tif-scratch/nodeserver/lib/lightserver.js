/*
 * Respond to commands over a websocket to relay TCP commands to a local program
 */

 //configure colour palette?

var socketio = require('socket.io');
var io;

var net = require('net');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	// Pased string of comamnd to relay
	socket.on('command', function(data) {
		console.log('received command: ' + data);
		
		// Info for connecting to the local process via TCP
		var PORT = 22110;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(data);

		var client = new net.Socket();
		client.connect(PORT, HOST, function() {
			console.log('Connected');
		});

		client.write(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		    if (err) 
		    	throw err;
		    console.log('TCP message sent to ' + HOST +':'+ PORT);
		});
		
		client.on('listening', function () {
		    var address = client.address();
		    console.log('TCP Client: listening on ' + address.address + ":" + address.port);
		});
		// Handle an incoming message over the TCP from the local application.
		client.on('data', function (message, remote) {
		    //console.log("TCP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
		    console.log("TCP CLient: message Rx: " + message)
		    
		    var reply = message.toString('utf8')
		    socket.emit('commandReply', reply);
		    
		    client.destroy();

		});

		client.on('close', function() {
		    console.log("closed");
		});

		client.on('error', function(err) {
		    console.log("error: ",err);
		});
	});
};

