using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Text;
using System.Threading;
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

		public void Write(Message msg)
		{
			if (_config.Enabled)
			{
				if (!Directory.Exists(_config.Folder))
					Directory.CreateDirectory(_config.Folder);

				string fPath;
				var index = 0;
				do
				{
					var fname = string.Concat(
						DateTime.Now.ToLocalTime().ToString("yyyy-MM-dd_HHmmss.fff."),
						msg.ProcessName + msg.Instance.ToString(), ".",
						msg.Command.ToString(),
						".",
						index,
						".json");

					index++;
					fPath = Path.Combine(_config.Folder, fname);

				}
				while (File.Exists(fPath)) ;

				using (var fs = new FileStream(fPath, FileMode.CreateNew, FileAccess.Write, FileShare.None))
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
