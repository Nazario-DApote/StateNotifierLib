using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace StateNotifierServer.Services
{
	public class JsonDump
    {
		private readonly JsonDumpOptions _config;

		private readonly ILogger _logger;

		public JsonDump(
			IOptions<JsonDumpOptions> options,
			ILogger<JsonDump> logger)
		{
			_config = options.Value;
			_logger = logger;
		}

		public async Task Write(Message msg)
		{
			if (_config.Enabled)
			{
				await Task.Yield(); // Make us async right away

				var fname = string.Concat(
					DateTime.Now.ToLocalTime().ToString("yyyy-MM-dd_HHmmss."),
					msg.ProcessName + msg.Instance.ToString(), ".",
					msg.Command.ToString(),
					".json");

				if (!Directory.Exists(_config.Folder))
					Directory.CreateDirectory(_config.Folder);

				using (var fs = new FileStream(Path.Combine(_config.Folder, fname), FileMode.CreateNew, FileAccess.Write, FileShare.None))
				using (var ws = new StreamWriter(fs, Encoding.UTF8))
				using (var writer = new JsonTextWriter(ws))
				{
					writer.Formatting = Formatting.Indented;
					JsonSerializer serializer = new JsonSerializer();
					serializer.Serialize(writer, msg);
				}
			}
		}
	}


	public class JsonDumpOptions
	{
		public JsonDumpOptions()
		{
			Folder = @"C:\StateNotifierDumps";
			Enabled = false;
		}
		public string Folder { get; set; }
		public bool Enabled { get; set; }
	}
}
