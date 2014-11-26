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
	socket.on('configData', function(data) {
		console.log('received config data: ' + data);

		var configData = {
			logLevel : data.logLevel,
			audioSource : data.audioSource,
			audioSourceParam : data.audioSourceParam,
			monitorAudio : data.monitorAudio,
			lowCutoff : data.lowCutoff,
			hiCutoff : data.hiCutoff,
			agcMax : data.agcMax,
			agcMin : data.agcMin,
			agcStrength : data.agcStrength
		}

		var args = process.argv.slice(2);
		//console.log(args[0]);

		//var outputFile = '/var/lib/stage-lights/config.json';
		var outputFile = args[0];
		fs.writeFile(outputFile, JSON.stringify(configData, undefined, 2), function(err){
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

