// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
var gvalues = [0.0001, 0.001, 0.01, 0.1, 1]; //agc min/max
$(document).ready(function() {
	var sockApp = new SockApp(socket);

	//$('#audioInputParam').attr('placeholder', 'plughw:1,0');
	$('#audioInput').change(function(){
		if($('#audioInput').val() == 'alsa'){
			$('#audioInputParam').attr('placeholder', 'plughw:1,0');
		} else {
			$('#audioInputParam').val('');
			$('#audioInputParam').attr('placeholder', '');
		}
	});

	$('#lightOutput').change(function(){
		if($('#lightOutput').val() == 'alsa'){
			$('#lightOutputParam').attr('placeholder', '192.168.0.4');
		} else {
			$('#lightOutputParam').val('');
			$('#lightOutputParam').attr('placeholder', '');
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
		value: 500,
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

	$('#slider-bright').slider({
		range: 'min',
		min: 0.01,
		max: 1.0,
		value: 0.5,
		step: 0.01,
		slide: function(event, ui) {
			$('#brightness').val(ui.value);
		}
	});
	$('#brightness').val($('#slider-bright').slider('value'));
	
	$.get( "/config.json", function( config ) {
		setConfig(config);
	}, "json");

	socket.on('commandReply', function(result) {
		$('#messages').append(divMessage(result));
		$('#messages').scrollTop($('#messages').prop('scrollHeight'));
	});

	$('#select-submit').click(function() {
		sendConfig(sockApp, socket);
		// Return false to show we have handleded it.
		return false;
	});
});

function getConfig() {
	var config = {};
	config.logLevel = $('#logLevel').val();
	config.audioInput = $('#audioInput').val();
	config.audioInputParam = $('#audioInputParam').val();
	config.controlInput = $('#controlInput').val();
	config.lightOutput = $('#lightOutput').val();
	config.lightOutputParam = $('#lightOutputParam').val();
	config.lowCutoff = $('#slider-low').slider('value');
	config.hiCutoff = $('#slider-hi').slider('value');
	config.agcMax = gvalues[$('#slider-gain').slider('values', 1)];
	config.agcMin = gvalues[$('#slider-gain').slider('values', 0)];
	config.agcStrength = parseFloat($('#gain-strength').val());
	config.brightness = $('#slider-bright').slider('value');
	return config;
}
function sendConfig(sockApp, socket){
	var data = getConfig();

	var systemMessage = sockApp.sendConfig(data);
	if(systemMessage){
		console.log('sock ok');
	}
}

function divMessage(inString) {
	return $('<div></div>').text(inString);
}

function agcRangeToSliderValue(value) {
	for(i=0;i<gvalues.length;i++) {
		if(value<=gvalues[i])
			return i;
	}
	return 1;
}

function setConfig(config) {
	$('#logLevel').val(config.logLevel);

	$('#audioInput').val(config.audioInput);
	$('#audioInputParam').val(config.audioInputParam);
	$('#controlInput').val(config.controlInput);
	$('#lightOutput').val(config.lightOutput);
	$('#lightOutputParam').val(config.lightOutputParam);

	$('#slider-low').slider('value', config.lowCutoff);
	$('#lo-amount').val($('#slider-low').slider('value'));
	$('#slider-hi').slider('value', config.hiCutoff);
	$('#hi-amount').val($('#slider-hi').slider('value'));

	$('#slider-gain').slider('values', 1, agcRangeToSliderValue(config.agcMax));
	$('#slider-gain').slider('values', 0, agcRangeToSliderValue(config.agcMin));
	$('#gain-amount').val(gvalues[$('#slider-gain').slider('values', 0)] + ' - ' + gvalues[$('#slider-gain').slider('values', 1)]);
	$('#gain-strength').val(config.agcStrength);
	
	$('#slider-bright').slider('value', config.brightness);
	$('#brightness').val($('#slider-bright').slider('value'));
}

