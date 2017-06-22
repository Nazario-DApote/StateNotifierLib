// State class definition
var State = function () {
	this.name = "";
	this.sequence = "";
	this.startTime = "";
	this.parameters = {}; // map of key/value
};

// State class definition
var Event = function () {
	this.name = "";
	this.from = "";
	this.to = "";
	this.startTime = "";
	this.parameters = {}; // map of key/value
};

// Device class definition
var Device = function () {
	this.process = "";
	this.instance = 0;
	this.States = new Array(); // array of State objects
	this.MaxStates = 10;
	this.Events = new Array(); // array of Event objects
};

// System class definition
var Canvas = function () {
	this.Devices = new Array();   // array of Device objects
};

function initCanvas() {
	var movie = bonsai.run(document.getElementById('movie'), {
		code: function () {
			var renderedObjects = {};

			// receive data from the other side
			stage.on('message:clear', function (canvas) {
				console.log("destroy message received");
				for (var prop in renderedObjects) {
					renderedObjects[prop].remove(stage);
					delete renderedObjects[prop];
				}
				delete renderedObjects;
				stage.clear();
			});
			stage.on('message', function (data) {

				if (!renderedObjects)
					renderedObjects = {};

				var canvas = data.canvas;
				if (canvas === null || canvas === 'undefined')
					return;

				var rowI = defSize = 40;
				var colI = 40;
				var rowIncr = rowI * 2;
				var colIncr = colI * 2;

				canvas.Devices.forEach(function (dev) {
					var devKey = dev.name;
					if (!renderedObjects.hasOwnProperty(devKey)) {
						// Draw the processName
						renderedObjects[devKey] = new Text(dev.name).attr({
							fontFamily: 'Arial, sans-serif',
							fontSize: '20',
							textStrokeColor: 'red',
							textFillColor: 'red',
							x: colI - defSize,
							y: rowI - defSize + rowI / 2
						});
						renderedObjects[devKey].addTo(stage);
					}

					colI += colIncr;

					dev.States.forEach(function (state) {
						// Draw the circle
						if (!renderedObjects.hasOwnProperty(state.uuid)) {
							renderedObjects[state.uuid] = new Circle(colI, rowI, defSize).attr('fillColor', 'green');
							renderedObjects[state.uuid].addTo(stage);
						}

						// Draw the state name
						var stateTextKey = state.uuid + "_text";
						if (!renderedObjects.hasOwnProperty(stateTextKey)) {
							renderedObjects[stateTextKey] = new Text(state.name).attr({
								fontFamily: 'Arial, sans-serif',
								fontSize: '8',
								textStrokeColor: 'black',
								x: colI - defSize,
								y: rowI - defSize
							});
							renderedObjects[stateTextKey].addTo(stage);
						}

						// Draw the state parameters
						var colParam = colI - defSize;
						var rowParam = rowI - defSize;
						for (var property in state.parameters) {
							var stateParamTextKey = state.uuid + "_" + property;
							if (!renderedObjects.hasOwnProperty(stateParamTextKey)) {
								var value = state.parameters[property];
								renderedObjects[stateParamTextKey] = new Text(property + ":" + value).attr({
									fontFamily: 'Arial, sans-serif',
									fontSize: '5',
									textStrokeColor: 'black',
									x: colParam,
									y: rowParam + 10
								});
								renderedObjects[stateParamTextKey].addTo(stage);
								rowParam += 10;
							}
						}

						colI += colIncr;
					});

					rowI += rowIncr;
				});
			});
			stage.sendMessage('ready', {});

			//stage.on('mouseover pointerdown', function (e) {
			//	t.addTo(stage);
			//});

			//stage.on('mouseout pointerup', function (e) {
			//	t.remove(t);
			//});
		}
		//,
		//width: width,
		//height: height
	});

	return movie;
}