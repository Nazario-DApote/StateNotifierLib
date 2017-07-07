﻿using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Newtonsoft.Json;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace StateNotifierServer.Services
{
	internal class TcpListenerService : IDisposable
	{
		private const int BUFFER_SIZE = 500;

		private readonly TcpListenerServiceConfig _config;

		private readonly ILogger _logger;

		private TcpListener _listener;

		private CancellationTokenSource _cancellationTokenSource;

		private StateNotifierService _notifier;

		private Dictionary<TcpClient, Task> _connections;

		private JsonDump _dump;

		private readonly object _lock = new Object(); // sync lock

		public TcpListenerService(
			IOptions<TcpListenerServiceConfig> options,
			ILogger<TcpListenerService> logger,
			StateNotifierService notifier,
			JsonDump dump)
		{
			_config = options.Value;
			_logger = logger;
			_notifier = notifier;
			_dump = dump;
			_cancellationTokenSource = new CancellationTokenSource();
			_connections = new Dictionary<TcpClient, Task>(); // pending connections
			_listener = new TcpListener(IPAddress.Any, _config.Port);
		}

		public void Dispose()
		{
			_cancellationTokenSource.Cancel();

			foreach (var c in _connections)
				c.Key.Client.Shutdown(SocketShutdown.Both);

			_listener.Stop();
			_listener.Server.Dispose();
		}

		public async void Listen()
		{
			if (_listener != null)
			{
				_listener.Start();
				_logger.LogInformation($"StateNotifier server service started at {IPAddress.Any}:{_config.Port}");

				var ct = _cancellationTokenSource.Token;

				// Continue listening.
				while (!ct.IsCancellationRequested)
				{
					_logger.LogInformation("Waiting for client...");
					ct.ThrowIfCancellationRequested();

					try
					{
						var client = await _listener.AcceptTcpClientAsync();
						_logger.LogInformation("[Server] Client has connected");
						var task = StartHandleConnectionAsync(client, ct);
						// if already faulted, re-throw any error on the calling context
						if (task.IsFaulted)
							task.Wait();
					}
					catch (ObjectDisposedException)
					{
						// server shutdown
						break;
					}
				}
			}
		}

		// Register and handle the connection
		private async Task StartHandleConnectionAsync(TcpClient tcpClient, CancellationToken ct)
		{
			// start the new connection task
			var connectionTask = HandleConnectionAsync(tcpClient, ct);

			// add it to the list of pending task
			lock (_lock)
				_connections.Add(tcpClient, connectionTask);

			// catch all errors of HandleConnectionAsync
			try
			{
				await connectionTask;
				// we may be on another thread after "await"
			}
			catch (Exception ex)
			{
				// log the error
				_logger.LogError(ex.Message);
				tcpClient.Dispose();
			}
			finally
			{
				// remove pending task
				lock (_lock)
					_connections.Remove(tcpClient);
			}
		}

		private async Task HandleConnectionAsync(TcpClient client, CancellationToken ct)
		{
			await Task.Yield();
			// continue asynchronously on another threads

			_logger.LogInformation("Client connected. Waiting for data.");
			while (!ct.IsCancellationRequested)
			{
				ct.ThrowIfCancellationRequested();

				using (var msResponse = new MemoryStream())
				{
					var networkStream = client.GetStream();
					var bufferForSize = new Byte[sizeof(int)];

					int numberOfBytesRead = await networkStream.ReadAsync(bufferForSize, 0, bufferForSize.Length, ct);
					var length = BitConverter.ToInt32(bufferForSize, 0);

					while (length > 0)
					{
						var bufferData = new byte[BUFFER_SIZE];
						ct.ThrowIfCancellationRequested();

						numberOfBytesRead = networkStream.Read(bufferData, 0, Math.Min(bufferData.Length, length));
						if (numberOfBytesRead <= 0)
							throw new Exception("No data received");

						await msResponse.WriteAsync(bufferData, 0, numberOfBytesRead, ct);
						length -= numberOfBytesRead; // decrement length from total bytes to read
					}

					ManageMessage(Encoding.ASCII.GetString(msResponse.ToArray()));
				}
			}
		}

		private async void ManageMessage(string msg)
		{
			if (_notifier != null)
			{
				var serializer = new JsonSerializer();
				var msgObj = JsonConvert.DeserializeObject<Message>(msg);
				if (msgObj != null)
				{
					await _notifier.SendBroadCast(msgObj);
					_dump.Write(msgObj);
				}
			}
		}
	}

	public enum Commands
	{
		ENTERSTATE,
		EXITSTATE,
		EVENT_EMIT,
		EVENT_RECV,
	}

	public class Message
	{
		[JsonProperty("process", Required = Required.Always)]
		public string ProcessName { get; set; }

		[JsonProperty("instance", Required = Required.Always)]
		public int Instance { get; set; }

		[JsonProperty("type", Required = Required.Always)]
		public Commands Command { get; set; }

		[JsonProperty("startTime", Required = Required.Always)]
		public DateTime StartTime { get; set; }

		[JsonProperty("sequence", NullValueHandling = NullValueHandling.Ignore)]
		public string Sequence { get; set; }

		[JsonProperty("name", Required = Required.Always)]
		public string Eventname { get; set; }

		[JsonProperty("from", NullValueHandling = NullValueHandling.Ignore)]
		public string From { get; set; }

		[JsonProperty("to", NullValueHandling = NullValueHandling.Ignore)]
		public string To { get; set; }

		[JsonProperty("parameters", NullValueHandling = NullValueHandling.Ignore)]
		public Dictionary<string, string> Parameters { get; set; }
	}

	public class TcpListenerServiceConfig
	{
		public int Port { get; set; }
	}
}