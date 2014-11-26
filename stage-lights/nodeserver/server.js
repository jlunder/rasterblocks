// After creating package.json, install modules:
//   $ npm install
//
// Launch server with:
//   $ node server.js
var PORT_NUMBER = 3007;


var http = require('http');
var fs   = require('fs');
var path = require('path');
var mime = require('mime');

/* 
 * Create the static web server
 */
var server = http.createServer(function(request, response) {
	var filePath = false;
	
	if (request.url == '/') {
		filePath = './public/index.html';
	} else if (request.url == '/config.json') {
		filePath = process.argv.slice(2)[0];
	} else {
		filePath = './public' + request.url;
	}
	
	serveStatic(response, filePath);
});

server.listen(PORT_NUMBER, function() {
	console.log('Server listening on port ' + PORT_NUMBER);
});

function serveStatic(response, filePath) {
	fs.exists(filePath, function(exists) {
		if (exists) {
			fs.readFile(filePath, function(err, data) {
				if (err) {
					send404(response);
					console.log(err);
				} else {
					sendFile(response, filePath, data);
				}
			});
		} else {
			send404(response);
			console.log(filePath+" does not exist");
		}
	});
}

function send404(response) {
	response.writeHead(404, {'Content-Type': 'text/plain'});
	response.write('Error 404: resource not found.');
	response.end();
}

function sendFile(response, filePath, fileContents) {
	response.writeHead(
			200,
			{'Content-Type': mime.lookup(path.basename(filePath))}
		);
	response.end(fileContents);
}


/*
 * Create the Userver to listen for the websocket
 */
var sockServer = require('./lib/lightserver');
sockServer.listen(server);


