// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {
	var tcpApp = new TcpApp(socket);
	
	socket.on('commandReply', function(result) {
		$('#messages').append(divMessage(result));
		$('#messages').scrollTop($('#messages').prop('scrollHeight'));
	});

	$('#select-submit').click(function() {
		processSelect(tcpApp, socket);
		// Return false to show we have handleded it.
		return false;
	});
});

function processSelect(tcpApp, socket){
	var logdata = $('#logLevel').val();

	var audiodata = $('#audioSource').val();

	var data = {
			logLevel: logdata,
			audioSource: audiodata
	}
	//console.log(data);

	var systemMessage = tcpApp.processConfig(data);
	if(systemMessage){
		console.log('tcp ok');
	}
}

function divMessage(inString) {
	return $('<div></div>').text(inString);
}

