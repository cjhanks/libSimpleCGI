#include <Python.h>

#include <boost/program_options.hpp>

#include "SimpleCGI/SimpleCGI.hpp"


namespace {
struct Settings {
  std::string domainSocketPath;
  std::string pythonModule;
  std::string pythonName;
};

Settings
ParseSettings(int argc, char** argv);
} // ns

int
main(int argc, char* argv[])
{
  Settings settings = ParseSettings(argc, argv);

  // {
  // Configure the WSGI application
  WsgiApplication::Config wsgiConfig;
  wsgiConfig.module = settings.pythonModule;
  wsgiConfig.app = settings.pythonName;

  WsgiApplication app(wsgiConfig);
  // }

  // {
  // Configure the fastcgi server.
  fcgi::LOG::SetLogLevel(fcgi::DEBUG);
  fcgi::ServerConfig config;
  //config.concurrencyModel = fcgi::ServerConfig::ConcurrencyModel::SYNCHRONOUS;
  config.concurrencyModel = fcgi::ServerConfig::ConcurrencyModel::PREFORKED;
  config.childCount = 4;
  config.callBack = std::bind(&WsgiApplication::Initialize, &app);
  config.catchAll = std::bind(&WsgiApplication::Serve,
                              &app,
                              std::placeholders::_1,
                              std::placeholders::_2);

  // -
  fcgi::MasterServer server(
      config, fcgi::DomainSocket(settings.domainSocketPath));

  server.ServeForever();
  // }

  return 0;
}

namespace {
Settings
ParseSettings(int argc, char** argv)
{
	namespace po = boost::program_options;

  Settings settings;

	// {
	po::options_description desc;
	desc.add_options()
	(
	 	"domain",
		po::value(&settings.domainSocketPath)->required(),
		""
	)
	(
	 	"module",
		po::value(&settings.pythonModule)->required(),
		""
	)
	(
	 	"name",
		po::value(&settings.pythonName)->required(),
		""
	)
	(
	 	"help",
	 	"Print for help!"
	);


	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help") == 1) {
			std::cerr << desc;
			std::exit(0);
		}

		po::notify(vm);
	} catch (const std::exception& e) {
		std::cerr << desc << "\n" << e.what()
			  << std::endl;
    std::exit(1);
	}

}
} // ns
