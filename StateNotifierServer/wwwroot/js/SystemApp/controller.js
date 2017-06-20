(function () {
	'use strict';

	angular
		.module('system.module')
		.controller('connectWebsocket', connectWebsocket);

	connectWebsocket.$inject = ['$scope', '$rootScope', '$websocket'];

	function connectWebsocket($scope, $rootScope, $websocket) {

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
					var resp = angular.fromJson(event.data);
					$scope.$apply();
				})
				.onError(function (event) {
					console.log("WEBSOCKET ERROR" + event.data);
				});
		}
	}
})();