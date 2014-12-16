// TCP Relay application to interact with server.
// used by the UI components

var SockApp = function(socket) {
	this.socket = socket
};

SockApp.prototype.sendConfig = function(data) {
	//console.log(JSON.stringify(data));
	this.sendCommand(data);
	return true;
};

SockApp.prototype.sendCommand = function(message) {
	this.socket.emit('configData', message);
};

