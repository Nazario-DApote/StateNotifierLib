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

		public CancellationToken CancelToken { get { return _ct; } }

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

		private string GetJsonValue(JObject json, string key)
		{
			var jsonObj = json[key];
			if (jsonObj != null)
				return jsonObj.ToString();
			else
				return null;
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
				await ManageClientConnectionLoop(socket, _ct);
			}
		}

		private async Task ManageClientConnectionLoop(WebSocket socket, CancellationToken ct)
		{
			await Task.Yield();

			var clKey = Guid.NewGuid();
			_notifier.Clients.Add(clKey, socket);

			while (true)
			{
				try
				{
					var jsonString = await ReceiveStringAsync(socket, ct);
					_logger.LogInformation($"Received message: {jsonString}");

					_logger.LogInformation(jsonString);
				}
				catch (JsonException ex)
				{
					_logger.LogError(ex.Message);
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

		private async Task CloseConnection(WebSocket socket, string reason, CancellationToken ct)
		{
			await socket.CloseAsync(WebSocketCloseStatus.NormalClosure, reason, ct);
		}

		private static void CheckPayload(string payload)
		{
			if (payload == null)
				throw new InvalidDataException("invalid payload");
		}

		private void CheckMessageId(string msgId)
		{
			if (string.IsNullOrEmpty(msgId))
				throw new InvalidDataException("Invalid messageId");
		}

		public Task SendStringAsync(WebSocket socket, string data, CancellationToken ct = default(CancellationToken))
		{
			var buffer = Encoding.UTF8.GetBytes(data);
			var segment = new ArraySegment<byte>(buffer);
			return socket.SendAsync(segment, WebSocketMessageType.Text, true, ct);
		}

		private async Task<string> ReceiveStringAsync(WebSocket socket, CancellationToken ct = default(CancellationToken))
		{
			// Message can be sent by chunk.
			// We must read all chunks before decoding the content
			var buffer = new ArraySegment<byte>(new byte[8192]);
			using (var ms = new MemoryStream())
			{
				WebSocketReceiveResult result;
				do
				{
					ct.ThrowIfCancellationRequested();

					result = await socket.ReceiveAsync(buffer, ct);
					ms.Write(buffer.Array, buffer.Offset, result.Count);
				}
				while (!result.EndOfMessage);

				ms.Seek(0, SeekOrigin.Begin);

				if (result.MessageType == WebSocketMessageType.Close)
					return $"{{ \"messageId\":\"{Guid.NewGuid()}\", \"command\": \"CLOSE\" }}"; // close not gracefully

				if (result.MessageType != WebSocketMessageType.Text)
					throw new Exception("Unexpected message");

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