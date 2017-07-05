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
	this.spaceBetweenRows = 100;
	this.spaceBetweenCols = 100;
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
				var devPosY = 60;
				var steps = canvas.Devices.length;
				var devStateHue = color('hsl(83, 100%, 50%, 1)'); // useful calculator: http://hslpicker.com/#9dff00
				var devTextColor = color('red').darker();
				var devFontSize = 15
				var R = colSize / 2;

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
					var lastStateRendered = null;
					dev.States.forEach(function (state) {

						if (!state || !state.name) {
							statePosX += 2 * R + spaceBetweenCols; // increment state position
							return;
						}

						if (lastStateRendered && !renderedObjects[state.uuid + "_gap"]) {
							var c1 = renderedObjects[lastStateRendered.uuid];  // get circle
							var x1 = c1.attr('x');
							var y1 = c1.attr('y');
							var d = R + spaceBetweenCols / 2;
							var pwd = Math.pow(d, 2);
							var g = Math.acos((2 * pwd - Math.pow(R, 2)) / (2 * pwd));
							var arcEndX = x1 + d + d * Math.cos(g), y1;
							console.log(arcEndX);
							var arcEndY = y1 - d * Math.sin(g);
							console.log(arcEndY);

							p = ['M', arcEndX, arcEndY, 'Q', arcEndX + 50, arcEndY + 50, statePosX + colSize / 2, statePosY + rowSize / 2].join(' ');

							renderedObjects[state.uuid + "_gap"] = new Path(p)
								.stroke('red', 1)
								.addTo(stage);
						}

						// Draw the circle
						if (!renderedObjects.hasOwnProperty(state.uuid)) {
							renderedObjects[state.uuid] = new Circle(statePosX + colSize / 2, statePosY + rowSize / 2, R)
								.attr('fillColor', devStateHue)
								.stroke(devStateHue.midpoint('black'), 0.5)
								.addTo(stage);
						}
						lastStateRendered = state;

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
								fontSize: '8',
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
									fontSize: '8',
									textStrokeColor: 'black',
									x: statePosX,
									y: paramStatePosY
								}).addTo(stage);
							}
						} // forEach parameters

						statePosX += 2 * R + spaceBetweenCols; // increment state position
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

						if (c1 === c2) // save device
						{
							var d = R + spaceBetweenCols / 2;
							var pwd = Math.pow(d, 2);
							var g = Math.acos((2 * pwd - Math.pow(R, 2)) / (2 * pwd));
							if (!renderedObjects[ev.uuid])
							{
								renderedObjects[ev.uuid] = new Arc(x1 + d, y1, d, Math.PI + g, 2 * Math.PI - g)
									.stroke('red', 1)
									.addTo(stage);
							}

							if (!renderedObjects[ev.uuid + "_text"])
							{
								renderedObjects[ev.uuid + "_text"] = new Text(ev.name).attr({
									fontFamily: 'Arial, sans-serif',
									fontSize: '8',
									textFillColor: 'red',
									textStrokeColor: 'red',
									x: x1 + d - 15,
									y: y1 - d - 15
								}).addTo(stage);
							}

							// Add Triangle
							if (!renderedObjects[ev.uuid + "_arrow"])
							{
								renderedObjects[ev.uuid + "_arrow"] = new Polygon(x1 + d + d * Math.cos(g), y1 - d * Math.sin(g), 5, 3)
									.attr({
										fillColor: 'red',
										rotation: Math.PI / 180 * 30
									}).addTo(stage);
							}
						}
						else // different devices
						{
							// Draw Bezier Curve
							var offbez = 20;
							var p = null;
							if (y1 < y2) {

								// Add Triangle
								if (!renderedObjects[ev.uuid + "_arrow"])
								{
									renderedObjects[ev.uuid + "_arrow"] = new Polygon(x2, y2 - R, 5, 3)
										.attr({
											fillColor: 'blue',
											rotation: Math.PI / 180 * 180
										}).addTo(stage);
								}

								p = ['M', x1, y1 + R, 'C', x1, y1 + R + offbez, x2, y2 - R - offbez, x2, y2 - R].join(' ');
							}
							else {
								// Add Triangle
								if (!renderedObjects[ev.uuid + "_arrow"])
								{
									renderedObjects[ev.uuid + "_arrow"] = new Polygon(x2, y2 + R, 5, 3)
										.attr({
											fillColor: 'blue',
											rotation: Math.PI / 180
										}).addTo(stage);
								}

								p = ['M', x1, y1 - R, 'C', x1, y1 - R - offbez, x2, y2 + R + offbez, x2, y2 + R].join(' ');
							}

							if (!renderedObjects[ev.uuid])
							{
								renderedObjects[ev.uuid] = new Path(p)
									.stroke('blue', 1)
									.addTo(stage);
							}

							if (!renderedObjects[ev.uuid + "_bezText"])
							{
								renderedObjects[ev.uuid + "_bezText"] = new Text(ev.name).attr({
									fontFamily: 'Arial, sans-serif',
									fontSize: '8',
									textFillColor: 'blue',
									textStrokeColor: 'blue',
									x: (x1 + x2) / 2,
									y: (y1 - R - offbez + (y2 + R + offbez)) / 2
								}).addTo(stage);
							}
						}
					}

				}); // canvas.Events.forEach

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