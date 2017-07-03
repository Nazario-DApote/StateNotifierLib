(function () {
	'use strict';

	Array.prototype.max = function () {
		return Math.max.apply(null, this);
	};

	Array.prototype.min = function () {
		return Math.min.apply(null, this);
	};

	angular
		.module('system.module')
		.controller('connectWebsocket', connectWebsocket);

	connectWebsocket.$inject = ['$scope', '$rootScope', '$websocket', "uuid", '$window'];

	function connectWebsocket($scope, $rootScope, $websocket, uuid, $window) {

		if (typeof Canvas === 'function') // check class defined
		{
			$scope.canvas = new Canvas($window.innerWidth);
			$scope.movie = initializeMove();
			$scope.movie.on('message:ready', function () {
				$scope.movieReady = true;
			});
		}

		$scope.connected = !angular.isUndefined($rootScope.ws) && $rootScope.ws !== null && !$rootScope.ws.readyState === 'OPEN';
		function string2Bin(str) {
			return str.split("").map(function (val) {
				return val.charCodeAt(0);
			});
		}

		function bin2String(array) {
			return String.fromCharCode.apply(String, array);
		}

		function appendText(data) {
			var valElem = angular.element(document.querySelector('#data'));
			valElem.append("<br>" + "<br>" + data);
		}

		if (!$scope.connected)
			{
				// instance of ngWebsocket, handled by $websocket service
				$rootScope.ws = $websocket('ws://localhost:5245/ws');
				$rootScope.ws.reconnectIfNotNormalClose = true;

				$rootScope.ws.onOpen(function () {
					console.log("WEBSOCKET CONNECTED");
					$scope.connected = true;
					// set operative
					if (typeof Canvas === 'function' && $scope.movieReady) {
						$scope.movie.sendMessage({
							canvas: $scope.canvas
						});
					}
					$scope.$apply();
				})
				.onClose(function () {
					console.log("WEBSOCKET DISCONNECTED");
					$scope.connected = false;
					$scope.$apply();
				})
				.onMessage(function (event) {
					console.log("NEW EVENT ARRIVED");
					appendText(event.data);
					console.log(event.data);

					if (typeof Canvas !== 'function') // check class defined
					{
						$scope.$apply();
						return;
					}

					var data = angular.fromJson(event.data);

					console.log("Window width:" + $window.innerWidth);
					console.log("Max states per view:" + $scope.canvas.MaxSteps());

					var dev;
					if (!$scope.canvas.Devices.find(function (elem) { var res = elem.name === data.process; if (res) dev = elem; return res; }))
					{
						dev = new Device();
						dev.uuid = uuid.v4();
						dev.name = data.process;
						dev.instance = data.instance;
						$scope.canvas.Devices.push(dev);
					}

					var currentTimeLine = 0;
					$scope.canvas.Devices.forEach(function (d) {
						if(d.name !== dev.name)
							currentTimeLine = Math.max(currentTimeLine, d.States.length-1);
						else
							currentTimeLine = Math.max(currentTimeLine, d.States.length);
					});
					if (currentTimeLine >= $scope.canvas.MaxSteps()) {
						$scope.movie.sendMessage("clear", $scope.canvas);
						$scope.canvas = new Canvas($window.innerWidth);
						dev.States = [];
						$scope.canvas.Devices.push(dev);
						currentTimeLine = 0;
					}

					if (data.type <= 2) // is a STATE change message
					{
						var newState = new State();
						newState.name = data.name;
						newState.uuid = uuid.v4();
						newState.sequence = data.sequence;
						newState.startTime = data.startTime;
						newState.parameters = data.parameters;
						for (var i = 0; i < currentTimeLine; i++) {
							if (!dev.States[i])
								dev.States[i] = {};
						}
						dev.States[currentTimeLine] = newState;
					}
					else // is EVENT message
					{
						var newEvent = new Event();
						newEvent.uuid = uuid.v4();
						newEvent.name = data.name;
						$scope.canvas.Devices.forEach(function (d) {
							if (d.name === data.from)
								newEvent.from = d.States[d.States.length-1].uuid;

							if (d.name === data.to)
							{
								d.States.forEach(function (s) {
									if (s.name)
										newEvent.to = s.uuid;
								});
							}
						});
						newEvent.startTime = data.startTime;
						newEvent.parameters = data.parameters;
						$scope.canvas.Events.push(newEvent);
					}
					if ($scope.movieReady)
					{
						$scope.movie.sendMessage({
							canvas: $scope.canvas
						});
					}
					$scope.$apply();
				})
				.onError(function (event) {
					console.log("WEBSOCKET ERROR" + event.data);
				});
		}
	}
})();