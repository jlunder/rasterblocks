// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {
	var udpApp = new UdpApp(socket);
	
	socket.on('commandReply', function(result) {
		$('#messages').append(divMessage(result));
		$('#messages').scrollTop($('#messages').prop('scrollHeight'));
	});
	
	// Make the text-entry box have focus.
	$('#send-command').focus();
	
	// Allow sending the form.
	$('#send-form').submit(function() {
		processUserInput(udpApp, socket);
		
		// Return false to show we have handleded it.
		return false;
	});
});

function processUserInput(udpApp, socket) {
	// Get the user's input from the browser.
	var message = $('#send-command').val();
	
	// Display the command in the message list.
	$('#messages').append(divMessage(message));

	// Process the command
	var systemMessage = udpApp.processCommand(message);
	if (systemMessage) {
		$('#messages').append(divMessage(systemMessage));
	}
	
	// Scroll window.
	$('#messages').scrollTop($('#messages').prop('scrollHeight'));
	
	// Clear the user's command (ready for next command).
	$('#send-command').val('');
}

function divMessage(inString) {
	return $('<div></div>').text(inString);
}

