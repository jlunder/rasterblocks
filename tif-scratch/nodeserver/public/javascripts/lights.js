// UDP Relay application to interact with server.
// used by the UI components

var UdpApp = function(socket) {
	this.socket = socket
};

UdpApp.prototype.processCommand = function(command) {
	var words = command.split(' ');
	var operation = words[0].toLowerCase();
	var message = false;
	
	switch(operation) {
	case 'test':
		words.shift(); // Strip first one
		var slCommand = words.join(' ');
		this.sendCommand(slCommand);
		break;
		
	default:
		message = 'Unrecognized command.'
	}
	return message;
};

UdpApp.prototype.sendCommand = function(message) {
	this.socket.emit('command', message);
};

