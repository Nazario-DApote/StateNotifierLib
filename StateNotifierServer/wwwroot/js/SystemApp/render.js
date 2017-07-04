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
};

// System class definition
var Canvas = function (windowWidth) {
	this.windowWidth = windowWidth;
	this.colSize = 80;
	this.rowSize = 80;
	this.spaceBetweenRows = 35;
	this.spaceBetweenCols = 25;
	this.Devices = [];   // array of Device objects
	this.Events = [];   // array of Events objects
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
				var devPosY = 20;
				var steps = canvas.Devices.length;
				var devStateHue = color('hsl(83, 100%, 50%, 1)'); // useful calculator: http://hslpicker.com/#9dff00
				var devTextColor = color('red').darker();
				var devFontSize = 15
				var circleRadius = rowSize / 2;

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
						}).addTo(stage);
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
								.stroke(devStateHue.midpoint('black'), 0.5)
								.addTo(stage);
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
							}).addTo(stage);
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
							}).addTo(stage);
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
							}).addTo(stage);
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
								}).addTo(stage);
							}
						} // forEach parameters

						statePosX += colSize + spaceBetweenCols; // increment state position
					}); // forEach state

					// increment device position
					devPosY += rowSize + spaceBetweenRows;
					devStateHue.hue(devStateHue.hue() + 1 / steps);

				}); // forEach device

				Math.radians = function (degrees) {
					return degrees * Math.PI / 180;
				};

				Math.deg = function (radiants) {
					return radiants * 180 / Math.PI;
				};

				canvas.Events.forEach(function (ev) {
					var c1 = renderedObjects[ev.from];  // get circle
					var c2 = renderedObjects[ev.to];	// get circle

					if (c1 && c2)
					{
						var x1 = c1.attr('x');
						var y1 = c1.attr('y');
						var x2 = c2.attr('x');
						var y2 = c2.attr('y');

						if (c1 === c2)
						{
							var R = colSize;
							var d = (colSize + spaceBetweenCols / 2) / 2;
							var pwd = Math.pow(d, 2);
							var g = Math.acos((2 * pwd - Math.pow(R, 2)) / (2 * pwd));
							var sticaz = Math.radians(6); // radiant corrections
							renderedObjects[ev.uuid] = new Arc(x1 + d, y1, d, 2 * Math.PI - g - sticaz, Math.PI + g + sticaz)
								.stroke('red', 1)
								.addTo(stage);

							renderedObjects[ev.uuid + "_text"] = new Text(ev.name).attr({
								fontFamily: 'Arial, sans-serif',
								fontSize: '8',
								textFillColor: 'red',
								textStrokeColor: 'red',
								x: x1 + d - 15,
								y: y1 - d - 15
							}).addTo(stage);
						}
						else
						{
							// Draw Bezier Curve
							var R = circleRadius;
							var offbez = 20;
							var p = null;
							if (y1 < y2)
								p = ['M', x1, y1 + R, 'C', x1, y1 + R + offbez, x2, y2 - R - offbez, x2, y2 - R].join(' ');
							else
								p = ['M', x1, y1 - R, 'C', x1, y1 - R - offbez, x2, y2 + R + offbez, x2, y2 + R].join(' ');
							renderedObjects[ev.uuid] = new Path(p)
								.stroke('red', 1)
								.addTo(stage);

							//renderedObjects[ev.uuid] = new Path()
							//	.moveTo(x1, y1)
							//	.lineTo(x2, y2)
							//	.stroke('red', 1)
							//	.addTo(stage);
						}
					}
				});

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