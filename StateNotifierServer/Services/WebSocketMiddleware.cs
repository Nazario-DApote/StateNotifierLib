using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.IO;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace StateNotifierServer.Services
{
	class WebSocketMiddleware
	{
		private readonly RequestDelegate _next;

		private readonly WebSocketMiddlewareConfig _config;

		private readonly ILogger _logger;

		private CancellationToken _ct;

		private StateNotifierService _notifier;

		public WebSocketMiddleware(RequestDelegate next,
			IOptions<WebSocketMiddlewareConfig> options,
			ILogger<WebSocketMiddleware> logger,
			StateNotifierService notifier)
		{
			_next = next;
			_config = options.Value;
			_logger = logger;
			_notifier = notifier;
			_notifier.WebSocketService = this;
		}

		public async Task Invoke(HttpContext context)
		{
			if (context.Request.Path == _config.Url)
			{
				if (!context.WebSockets.IsWebSocketRequest)
				{
					// Not a web socket request
					await _next.Invoke(context);
					return;
				}

				_ct = context.RequestAborted;
				 var socket = await context.WebSockets.AcceptWebSocketAsync();
				await HandleConnectionAsync(socket);
			}
		}

		private async Task HandleConnectionAsync(WebSocket socket)
		{
			await Task.Yield();

			var clKey = Guid.NewGuid();
			_notifier.Clients.Add(clKey, socket);

			while (!_ct.IsCancellationRequested)
			{
				_ct.ThrowIfCancellationRequested();

				try
				{
					var jsonString = await ReceiveStringAsync(socket);
					var json = JObject.Parse(jsonString);
					if (json["command"] != null && json["command"].Value<string>().Equals("CLOSE"))
					{
						_logger.LogInformation($"Connection closed by peer: {jsonString}");
						_notifier.Clients.Remove(clKey);
						break;
					}
					else
					{
						// TODO: Client messages are not used
						_logger.LogInformation($"Received message: {jsonString}");
					}
				}
				catch (JsonException ex)
				{
					_logger.LogError(ex.Message);
				}
				catch (WebSocketException ex)
				{
					_logger.LogError(ex.Message);
					_notifier.Clients.Remove(clKey);
					break;
				}
				catch (Exception ex)
				{
					_logger.LogCritical(ex.Message);
					_logger.LogCritical(ex.StackTrace);
					_notifier.Clients.Remove(clKey);
					break;
				}
			}
		}

		public async Task SendStringAsync(WebSocket socket, string data)
		{
			var buffer = Encoding.UTF8.GetBytes(data);
			var segment = new ArraySegment<byte>(buffer);
			await socket.SendAsync(segment, WebSocketMessageType.Text, true, _ct);
		}

		private async Task<string> ReceiveStringAsync(WebSocket socket)
		{
			using (var ms = new MemoryStream())
			{
				// Message can be sent by chunk.
				// We must read all chunks before decoding the content
				var buffer = new ArraySegment<byte>(new byte[8192]);
				WebSocketReceiveResult result;
				do
				{
					_ct.ThrowIfCancellationRequested();

					result = await socket.ReceiveAsync(buffer, _ct);
					if (result.MessageType == WebSocketMessageType.Close)
					{
						_logger.LogInformation($"WS Connection closed: ${result.CloseStatusDescription}");
						return $"{{\"messageId\":\"{Guid.NewGuid()}\",\"command\":\"CLOSE\"}}"; // close gracefully
					}
					else if (result.MessageType == WebSocketMessageType.Text)
						ms.Write(buffer.Array, buffer.Offset, result.Count);
					else
						throw new WebSocketException("Unexpected message");
				}
				while (!result.EndOfMessage);

				ms.Seek(0, SeekOrigin.Begin);

				// Encoding UTF8: https://tools.ietf.org/html/rfc6455#section-5.6
				using (var reader = new StreamReader(ms, Encoding.UTF8))
				{
					return await reader.ReadToEndAsync();
				}
			}
		}
	}

	public class WebSocketMiddlewareConfig
	{
		public WebSocketMiddlewareConfig()
		{
			// set default
			Url = "/ws";
		}

		public string Url { get; set; }
	}
}