// TCP Relay application to interact with server.
// used by the UI components

var TcpApp = function(socket) {
	this.socket = socket
};

TcpApp.prototype.processConfig = function(data) {
	//console.log(JSON.stringify(data));
	this.sendCommand(data);
	return true;
};

TcpApp.prototype.sendCommand = function(message) {
	this.socket.emit('command', message);
};

