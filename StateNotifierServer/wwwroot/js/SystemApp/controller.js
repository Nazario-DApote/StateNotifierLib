(function () {
	'use strict';

	angular
		.module('system.module')
		.controller('connectWebsocket', connectWebsocket);

	connectWebsocket.$inject = ['$scope', '$rootScope', '$websocket', "uuid", '$window'];

	function connectWebsocket($scope, $rootScope, $websocket, uuid, $window) {

		$scope.canvas = new Canvas();
		$scope.movie = initCanvas();
		$scope.movie.on('message:ready', function () {
			$scope.movieReady = true;
		});

		$scope.first = 0;

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

				$rootScope.ws.onOpen(function () {
					console.log("WEBSOCKET CONNECTED");
					// set operative
					$scope.connected = true;
					if ($scope.movieReady) {
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
					var data = angular.fromJson(event.data);

					console.log("Window width:" + $window.innerWidth);
					var maxSteps = $window.innerWidth / 80 - 3;
					console.log("Max states per view:" + maxSteps);

					if ($scope.first >= maxSteps) {
						$scope.movie.sendMessage("clear", $scope.canvas);
						$scope.canvas = new Canvas();
						$scope.first = 0;
					}
					else
						$scope.first += 1;

					var dev;
					if (!$scope.canvas.Devices.find(function (elem) { var res = elem.name === data.process; if (res) dev = elem; return res; }))
					{
						dev = new Device();
						dev.uuid = uuid.v4();
						dev.name = data.process;
						dev.instance = data.instance;
						$scope.canvas.Devices.push(dev);
					}
					if (data.type <= 2)
					{
						var newState = new State();
						newState.name = data.name;
						newState.uuid = uuid.v4();
						newState.sequence = data.sequence;
						newState.startTime = data.startTime;
						newState.parameters = data.parameters;
						dev.States.push(newState);
					}
					else
					{
						var newEvent = new Event();
						newEvent.uuid = uuid.v4();
						newEvent.name = data.name;
						newEvent.startTime = data.startTime;
						newEvent.parameters = data.parameters;
						dev.Events.push(newEvent);
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