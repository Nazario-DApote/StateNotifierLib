using Newtonsoft.Json;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.WebSockets;
using System.Threading.Tasks;

namespace StateNotifierServer.Services
{
	class StateNotifierService
	{
		private ConcurrentDictionary<Guid, WebSocket> _dict;

		public StateNotifierService()
		{
			_dict = new ConcurrentDictionary<Guid, WebSocket>();
		}

		public WebSocketMiddleware WebSocketService { get; set; }
		public IDictionary<Guid, WebSocket> Clients { get => _dict; }

		public async Task SendBroadCast(Message msg)
		{
			foreach (var cl in Clients.Values)
			{
				await WebSocketService.SendStringAsync(cl, JsonConvert.SerializeObject(msg, Formatting.Indented), WebSocketService.CancelToken);
			}
		}
	}
}
