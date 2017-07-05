using System;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using StateNotifierServer.Services;

namespace StateNotifierServer
{
	public class Startup
	{
		private ILoggerFactory _loggerFactory;

		private IHostingEnvironment _hostingEnvironment;

		public Startup(IHostingEnvironment env, ILoggerFactory loggerFactory)
		{
			var builder = new ConfigurationBuilder()
				.SetBasePath(env.ContentRootPath)
				.AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
				.AddJsonFile($"appsettings.{env.EnvironmentName}.json", optional: true)
				.AddEnvironmentVariables();
			Configuration = builder.Build();
			this._hostingEnvironment = env;
			this._loggerFactory = loggerFactory;
		}

		public IConfigurationRoot Configuration { get; }

		// This method gets called by the runtime. Use this method to add services to the container.
		public void ConfigureServices(IServiceCollection services)
		{
			// Add framework services.
			services.AddMvc();

			// Read services configuration
			services.Configure<WebSocketMiddlewareConfig>(Configuration.GetSection("WebSocket"));
			services.Configure<TcpListenerServiceConfig>(Configuration.GetSection("TcpSocket"));
			services.Configure<JsonDumpOptions>(Configuration.GetSection("Dump"));

			// Initialize Services
			services.AddSingleton<StateNotifierService>();
			services.AddSingleton<JsonDump>();
			services.AddSingleton<TcpListenerService>();
		}

		// This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
		public void Configure(IApplicationBuilder app, IHostingEnvironment env, ILoggerFactory loggerFactory, IApplicationLifetime lifetime)
		{

			if (env.IsDevelopment())
			{
				loggerFactory.AddConsole(Configuration.GetSection("Logging"));
				loggerFactory.AddDebug();
				app.UseDeveloperExceptionPage();
			}
			else
			{
				app.UseExceptionHandler("/Home/Error");
			}

			app.UseStaticFiles();
			app.UseMvc(routes =>
			{
				routes.MapRoute(
					name: "default",
					template: "{controller=Home}/{action=Index}/{id?}");
			});

			// Configure WebSocket
			var webSocketOptions = new WebSocketOptions()
			{
				KeepAliveInterval = TimeSpan.FromSeconds(120),
				ReceiveBufferSize = 4 * 1024
			};
			app.UseWebSockets(webSocketOptions);
			app.UseMiddleware<WebSocketMiddleware>();

			var tcpService = app.ApplicationServices.GetRequiredService<TcpListenerService>();
			tcpService.Listen();

			lifetime.ApplicationStopping.Register(() => {
				Console.WriteLine("StateNotifierServer terminated!");
			});
		}
	}
}
