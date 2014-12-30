/*
 * Respond to commands over a websocket to relay TCP commands to a local program
 */

 //configure colour palette?

var socketio = require('socket.io');
var io;

var net = require('net');
var fs = require('fs');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	//data from client
	socket.on('configData', function(config) {
		console.log('received config data: ' + config);

		//var outputFile = '/var/lib/rasterblocks/config.json';
		var outputFile = process.argv.slice(2)[0];
		fs.writeFile(outputFile + "~", JSON.stringify(config, undefined, 2), function(err){
			fs.renameSync(outputFile + "~", outputFile);
			if(err){
				console.log(err);
			} else {
				console.log('JSON saved to ' + outputFile);
				var reply = 'Config file changed\n';
				socket.emit('commandReply', reply);
			}
		});
	});
};

