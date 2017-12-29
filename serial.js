var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
var port = new SerialPort('COM3', {
	baudRate: 9600,
	dataBits: 8
});

var Readline = SerialPort.parsers.Readline;
var parser = new Readline();

var buffer = '';
port.on('open', function() {
	console.log('Serial Port Opened');
	
	port.on('data', function(data) {	
		buffer += data;
		var answers = buffer.split(/\r?\n/);
		buffer = answers.pop();
		
		if (answers.length > 0) {
			console.log(answers[0]);
		}
	});
});

port.on('error', function(e) {
	
});