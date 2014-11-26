// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
var gvalues = [0, 0.001, 0.01, 0.1, 1]; //agc min/max
$(document).ready(function() {
	var sockApp = new SockApp(socket);

	//$('#audioSourceParam').attr('placeholder', 'plughw:1,0');
	$('#audioSource').change(function(){
		if($('#audioSource').val() == 'SLAIS_ALSA'){
			$('#audioSourceParam').attr('placeholder', 'plughw:1,0');
		} else {
			$('#audioSourceParam').val('');
			$('#audioSourceParam').attr('placeholder', '../test/clips/909Tom X1.wav');
		}
	});

	$('#slider-low').slider({
		range: 'min',
		min: 200,
		max: 400,
		value: 200,
		step: 10,
		slide: function(event, ui) {
			$('#lo-amount').val(ui.value);
		}
	});
	$('#lo-amount').val($('#slider-low').slider('value'));
	
	$('#slider-hi').slider({
		range: 'min',
		min: 300,
		max: 1000,
		value: 300,
		step: 10,
		slide: function(event, ui) {
			$('#hi-amount').val(ui.value);
		}
	});
	$('#hi-amount').val($('#slider-hi').slider('value'));

	$('#slider-gain').slider({
		range: true,
		max: 4,
		min: 0,
		step: 1,
		values: [0, 5],
		slide: function(event, ui) {
			$('#gain-amount').val(gvalues[ui.values[0]] + ' - ' + gvalues[ui.values[1]]);
		}
	});

	setDefault();

	socket.on('commandReply', function(result) {
		$('#messages').append(divMessage(result));
		$('#messages').scrollTop($('#messages').prop('scrollHeight'));
	});

	$('#select-submit').click(function() {
		processSelect(sockApp, socket);
		// Return false to show we have handleded it.
		return false;
	});
});

function processSelect(sockApp, socket){
	var logdata = $('#logLevel').val();

	var audiodata = $('#audioSource').val();
	var audioSPdata = $('#audioSourceParam').val();
	var monitorAud = $('#monitorAudio').prop('checked');

	var lowdata = $('#slider-low').slider('value');
	var hidata = $('#slider-hi').slider('value');

	var maxagc = gvalues[$('#slider-gain').slider('values', 1)];
	var minagc = gvalues[$('#slider-gain').slider('values', 0)];
	var strengthagc = parseFloat($('#gain-strength').val());

	var data = {
			logLevel: logdata,
			audioSource: audiodata,
			audioSourceParam: audioSPdata,
			monitorAudio: monitorAud,
			lowCutoff : lowdata,
			hiCutoff : hidata,
			agcMax : maxagc,
			agcMin : minagc,
			agcStrength : strengthagc

	}

	var systemMessage = sockApp.processConfig(data);
	if(systemMessage){
		console.log('sock ok');
	}
}

function divMessage(inString) {
	return $('<div></div>').text(inString);
}

function setDefault() {
	$('#logLevel').val('SLLL_WARNING');

	$('#audioSource').val('SLAIS_ALSA');
	$('#audioSourceParam').val('plughw:1,0');
	$('#monitorAudio').prop('checked', false);

	$('#slider-low').slider('value', 200);
	$('#lo-amount').val($('#slider-low').slider('value'));
	$('#slider-hi').slider('value', 300);
	$('#hi-amount').val($('#slider-hi').slider('value'));

	$('#slider-gain').slider('values', 1, 4);
	$('#slider-gain').slider('values', 0, 1);
	$('#gain-amount').val(gvalues[$('#slider-gain').slider('values', 0)] + ' - ' + gvalues[$('#slider-gain').slider('values', 1)]);
	$('#gain-strength').val(0.5);
}

