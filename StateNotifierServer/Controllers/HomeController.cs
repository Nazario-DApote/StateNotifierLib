using System;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;

namespace Gvr.DispatcherProxy.Controllers
{
	public class HomeController : Controller
	{
		ILogger _logger;
		IServiceProvider _services;

		public HomeController(ILoggerFactory loggerFactory, IServiceProvider services)
		{
			this._logger = loggerFactory.CreateLogger("HomeController");
			this._services = services;
		}

		public IActionResult Index()
		{
			_logger.LogDebug("Index called");
			return View();
		}
		public IActionResult About()
		{
			ViewData["Message"] = "Your application description page.";
			_logger.LogDebug("About called");
			return View();
		}

		public IActionResult Contact()
		{
			ViewData["Message"] = "Your contact page.";
			_logger.LogDebug("Contact called");
			return View();
		}

		public IActionResult Error()
		{
			_logger.LogDebug("Error called");
			return View();
		}
	}
}