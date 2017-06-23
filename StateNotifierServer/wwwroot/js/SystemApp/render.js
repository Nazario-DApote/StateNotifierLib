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
	this.name = "";
	this.instance = 0;
	this.States = []; // array of State objects
	this.Events = []; // array of Event objects
};

// System class definition
var Canvas = function (windowWidth) {
	this.windowWidth = windowWidth;
	this.colSize = 80;
	this.rowSize = 80;
	this.spaceBetweenRows = 15;
	this.spaceBetweenCols = 25;
	this.Devices = [];   // array of Device objects
};

Canvas.prototype.MaxSteps = function () { return Math.floor(this.windowWidth / (this.colSize + this.spaceBetweenCols) - 1); }

function initializeMove() {
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

				var canvas = data.canvas;
				if (!canvas) return;

				if (!renderedObjects)
					renderedObjects = {};

				var spaceBetweenRows = canvas.spaceBetweenRows;
				var spaceBetweenCols = canvas.spaceBetweenCols;
				var colSize = canvas.colSize;
				var rowSize = canvas.rowSize;
				var devPosX = 0;
				var devPosY = 5;
				var steps = canvas.Devices.length;
				var devStateHue = color('hsl(83, 100%, 50%, 1)'); // useful calculator: http://hslpicker.com/#9dff00
				var devTextColor = color('red').darker();
				var devFontSize = 15
				var circleRadius = rowSize / 2 + devPosY;

				canvas.Devices.forEach(function (dev) {

					if (!dev || !dev.name) return;

					var devKey = dev.name;
					if (!renderedObjects.hasOwnProperty(devKey)) {
						// Draw the deviceName
						renderedObjects[devKey] = new Text(dev.name).attr({
							fontFamily: 'Arial, sans-serif',
							fontSize: devFontSize,
							textStrokeColor: devTextColor,
							textFillColor: devTextColor,
							x: devPosX,
							y: devPosY + colSize / 2 - devFontSize
						});
						renderedObjects[devKey].addTo(stage);
					}

					var statePosX = devPosX + colSize; // start circle position
					var statePosY = devPosY;
					dev.States.forEach(function (state) {

						if (!state || !state.name) {
							statePosX += colSize + spaceBetweenCols; // increment state position
							return;
						}

						// Draw the circle
						if (!renderedObjects.hasOwnProperty(state.uuid)) {
							renderedObjects[state.uuid] = new Circle(statePosX + colSize / 2, statePosY + rowSize / 2, circleRadius)
								.attr('fillColor', devStateHue)
								.stroke(devStateHue.midpoint('black'), 0.5);

							renderedObjects[state.uuid].addTo(stage);
						}

						// Draw the state name
						var stateTextsPosY = statePosY;
						var stateTextKey = state.uuid + "_text";
						if (!renderedObjects.hasOwnProperty(stateTextKey)) {
							renderedObjects[stateTextKey] = new Text(state.name).attr({
								fontFamily: 'Arial, sans-serif',
								fontSize: '8',
								textStrokeColor: 'black',
								x: statePosX,
								y: stateTextsPosY
							});
							renderedObjects[stateTextKey].addTo(stage);
						}

						// Draw the sequence name
						stateTextsPosY += 10;
						var stateSeqTextKey = state.uuid + "_seq_text";
						if (!renderedObjects.hasOwnProperty(stateSeqTextKey)) {
							renderedObjects[stateSeqTextKey] = new Text(state.sequence).attr({
								fontFamily: 'Arial, sans-serif',
								fontSize: '8',
								textStrokeColor: 'black',
								x: statePosX,
								y: stateTextsPosY
							});
							renderedObjects[stateSeqTextKey].addTo(stage);
						}

						// Draw the statrttime
						stateTextsPosY += 10;
						var stateSeqTextKey = state.uuid + "_time_text";
						if (!renderedObjects.hasOwnProperty(stateSeqTextKey)) {
							renderedObjects[stateSeqTextKey] = new Text(state.startTime).attr({
								fontFamily: 'Arial, sans-serif',
								fontSize: '2',
								textStrokeColor: 'black',
								x: statePosX,
								y: stateTextsPosY
							});
							renderedObjects[stateSeqTextKey].addTo(stage);
						}

						// Draw the state parameters
						var paramStatePosY = stateTextsPosY;
						for (var property in state.parameters) {
							paramStatePosY += 10;
							var stateParamTextKey = state.uuid + "_" + property;
							if (!renderedObjects.hasOwnProperty(stateParamTextKey)) {
								var value = state.parameters[property];
								renderedObjects[stateParamTextKey] = new Text(property + ":" + value).attr({
									fontFamily: 'Arial, sans-serif',
									fontSize: '5',
									textStrokeColor: 'black',
									x: statePosX,
									y: paramStatePosY
								});
								renderedObjects[stateParamTextKey].addTo(stage);
							}
						} // forEach parameters

						statePosX += colSize + spaceBetweenCols; // increment state position
					}); // forEach state

					// increment device position
					devPosY += rowSize + spaceBetweenRows;
					devStateHue.hue(devStateHue.hue() + 1 / steps);

				}); // forEach device

			}); // end rendering

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