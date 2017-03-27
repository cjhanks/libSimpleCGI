#include "WsgiServer.hpp"

#include <csignal>

#include "SimpleCGI/common/Logging.hpp"
#include "WsgiReader.hpp"
#include "WsgiError.hpp"
#include "WsgiStartResponse.hpp"


using namespace fcgi;


WsgiApplication::WsgiApplication(Config config)
  : config(config)
  , module(nullptr)
  , application(nullptr)
{}

WsgiApplication::~WsgiApplication()
{
  if (application)
    Py_DECREF(application);

  if (module)
    Py_DECREF(module);

  Py_Finalize();
}

void
WsgiApplication::Initialize()
{
  LOG(INFO) << "Initialize: "
            << config.module
            << ":"
            << config.app;
  // 1.  Initialize the python library
  Py_Initialize();

  // Override the annoying Python SIGINT handler.
  // FIXME:  More properly free the memory allocated here.
  std::signal(SIGINT, SIG_DFL);

  // 2.  Load the provided module
  PyObject* moduleName = PyUnicode_FromString(config.module.c_str());
  module = PyImport_Import(moduleName);
  Py_DECREF(moduleName);
  if (module == nullptr) {
    PyErr_Print();
    throw std::runtime_error("Failed to load module");
  }

  // 3.  Load the application
  application = PyObject_GetAttrString(module, config.app.c_str());
  if (application == nullptr) {
    PyErr_Print();
    throw std::runtime_error("Failed to load application");
  }

  // 4.
  WsgiHandler::WsgiVersionTuple = PyTuple_Pack(
      2,
      PyLong_FromLong(1),
      PyLong_FromLong(0)
  );
}

bool
WsgiApplication::Serve(fcgi::HttpRequest& req, fcgi::HttpResponse& res)
{
  return WsgiHandler(req, res)(application);
}

////////////////////////////////////////////////////////////////////////////////
PyObject*
WsgiHandler::WsgiVersionTuple = nullptr;

WsgiHandler::WsgiHandler(fcgi::HttpRequest& req, fcgi::HttpResponse& res)
  : req(req)
  , res(res)
  , environment(PyDict_New())
{
}

WsgiHandler::~WsgiHandler()
{
  if (environment)
    Py_DECREF(environment);

  if (callback)
    Py_DECREF(callback);
}

bool
WsgiHandler::operator()(PyObject* wsgiFunctor)
{
  PyObject* arglist  = nullptr;
  PyObject* results  = nullptr;
  PyObject* iterator = nullptr;
  bool rc = false;

  PopulateEnvironmentCGI();
  PopulateEnvironmentWSGI();

  // -
  callback = New(&res);
  if (callback == nullptr)
    return false;

  // -
  arglist = Py_BuildValue("OO", environment, callback);
  if (arglist == nullptr)
    goto unwind;

  // -
  results = PyObject_CallObject(wsgiFunctor, arglist);
  if (results == nullptr)
    goto unwind;

  iterator = PyObject_GetIter(results);
  if (!PyIter_Check(iterator)) {
    goto unwind;
  }

  // -
  while (PyObject* elem = PyIter_Next(iterator)) {
    if (!PyBytes_Check(elem)) {
      LOG(WARNING) << "UNKNOWN TYPE";
    }

    Py_BEGIN_ALLOW_THREADS
    res.write(PyBytes_AsString(elem), PyBytes_GET_SIZE(elem));
    Py_END_ALLOW_THREADS

    Py_DECREF(elem);
  }

  rc = true;

unwind:
  if (arglist)
    Py_DECREF(arglist);

  if (results)
    Py_DECREF(results);

  if (iterator)
    Py_DECREF(iterator);

  return rc;
}

void
WsgiHandler::PopulateEnvironmentCGI()
{
  for (const auto& pair: req.headers()) {
    PyDict_SetItem(
        environment,
        PyUnicode_FromString(pair.first.c_str()),
        PyUnicode_FromString(pair.second.c_str())
    );
  }

  PyDict_SetItem(
      environment,
      PyUnicode_FromString("PATH_INFO"),
      PyUnicode_FromString(req.GetHeader("DOCUMENT_URI").c_str())
  );
}

void
WsgiHandler::PopulateEnvironmentWSGI()
{
  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.url_scheme"),
      PyUnicode_FromString(req.GetHeader("REQUEST_SCHEME").c_str())
  );

  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.version"),
      WsgiVersionTuple
  );

  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.multithread"),
      Py_False
  );

  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.multiprocess"),
      Py_False
  );

  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.input"),
      New(&req)
  );

#if 0
  PyDict_SetItem(
      environment,
      PyUnicode_FromString("wsgi.error"),
      New(&res)
  );
#endif
}
