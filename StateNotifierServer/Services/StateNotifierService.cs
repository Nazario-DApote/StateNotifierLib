using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Net.WebSockets;
using System.Threading.Tasks;

namespace StateNotifierServer.Services
{
	interface IWsNotifier
	{
		WebSocketMiddleware WebSocketService { get; set; }
		Task SendBroadCast(Message msg);
		Dictionary<Guid, WebSocket> Clients { get; set; }
	}

	class StateNotifierService : IWsNotifier
	{
		public Dictionary<Guid, WebSocket> Clients { get; set; }

		public WebSocketMiddleware WebSocketService { get; set; }

		public async Task SendBroadCast(Message msg)
		{
			foreach (var cl in Clients)
			{
				await WebSocketService.SendStringAsync(cl.Value, JsonConvert.SerializeObject(msg, Formatting.Indented), WebSocketService.CancelToken);
			}
		}
	}
}
