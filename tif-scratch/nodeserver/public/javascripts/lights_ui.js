// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {
	var tcpApp = new TcpApp(socket);

	$("#slider-low").slider({
		//orientation: "horizontal",
		range: "min",
		min: 200,
		max: 400,
		value: 200,
		step: 10,
		slide: function(event, ui) {
			$("#lo-amount").val(ui.value);
		}
	});
	$("#lo-amount").val($("#slider-low").slider("value"));
	
	$("#slider-hi").slider({
		range: "min",
		min: 300,
		max: 1000,
		value: 300,
		step: 10,
		slide: function(event, ui) {
			$("#hi-amount").val(ui.value);
		}
	});
	$("#hi-amount").val($("#slider-hi").slider("value"));

	$( "#slider-gain" ).slider({
		range: true,
		max: 1.001,
		min: 0.001,
		step: 0.001,
		values: [ 0.001, 1.000 ],
		slide: function( event, ui ) {
			$( "#gain-amount" ).val(ui.values[ 0 ] + " - " + ui.values[ 1 ] );
		}
	});

	$("#use-default").click(function(){
		$('#logLevel').val("SLLL_WARNING");

		$('#audioSource').val("SLAIS_FILE");

		$("#slider-low").slider("value", 200);
		$("#lo-amount").val($("#slider-low").slider("value"));
		$("#slider-hi").slider("value", 300);
		$("#hi-amount").val($("#slider-hi").slider("value"));

		$("#slider-gain").slider("values", 1, 1);
		$("#slider-gain").slider("values", 0, 0.001);
		$( "#gain-amount" ).val($("#slider-gain").slider("values", 0) + " - " + $("#slider-gain").slider("values", 1));
		$("#gain-strength").val(0.5);
	});

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

	var lowdata = $("#slider-low").slider("value");
	var hidata = $("#slider-hi").slider("value");

	var maxacg = $("#slider-gain").slider("values", 1);
	var minacg = $("#slider-gain").slider("values", 0);
	var strengthacg = parseFloat($("#gain-strength").val());

	var data = {
			logLevel: logdata,
			audioSource: audiodata,
			lowCutoff : lowdata,
			hiCutoff : hidata,
			acgMax : maxacg,
			acgMin : minacg,
			acgStrength : strengthacg

	}

	var systemMessage = tcpApp.processConfig(data);
	if(systemMessage){
		console.log('tcp ok');
	}
}

function divMessage(inString) {
	return $('<div></div>').text(inString);
}

