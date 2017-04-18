#include "WsgiStartResponse.hpp"

#include <sstream>
#include <string>
#include <tuple>

#include "SimpleCGI/common/Logging.hpp"


using namespace fcgi;


namespace {
struct WsgiStartResponseObject {
  PyObject_HEAD
  HttpResponse* res = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
std::tuple<size_t, std::string>
ParseMessageLine(const std::string& message)
{
  // TODO: Error handling
  std::stringstream ss(message);
  size_t code;
  ss >> code;

  std::string msg;
  std::getline(ss, msg);

  return std::make_tuple(code, msg);
}

PyObject*
__call__(PyObject* self_, PyObject* args, PyObject**)
{
  PyObject* status_ = nullptr;
  PyObject* headers = nullptr;

  if (!PyArg_ParseTuple(args, "OO", &status_, &headers)) {
    return nullptr;
  }

  // - Extract out the status lines
  auto self = (WsgiStartResponseObject*)self_;
  std::string status(
      PyBytes_AS_STRING(PyUnicode_AsEncodedString(status_, "utf-8", "Error")));

  size_t code;
  std::string message;
  std::tie(code, message) = ParseMessageLine(status);

  // - Extract out the headers
  assert(PyList_CheckExact(headers));

  fcgi::HttpHeader header(code, message);
  for (Py_ssize_t i = 0; i < PyList_Size(headers); ++i) {
    PyObject* pair = PyList_GET_ITEM(headers, i);
    assert(PyTuple_CheckExact(pair));

    PyObject* nameObj  = PyTuple_GET_ITEM(pair, 0);
    PyObject* valueObj = PyTuple_GET_ITEM(pair, 1);

    std::string name(
      PyBytes_AS_STRING(PyUnicode_AsEncodedString(nameObj, "utf-8", "Error")));
    std::string value(
      PyBytes_AS_STRING(PyUnicode_AsEncodedString(valueObj, "utf-8", "Error")));

    Py_DECREF(pair);
    header.addHeader(name, value);
  }

  // - Send header response
  assert(nullptr != self);
  assert(nullptr != self->res);
  self->res->SetResponse(header);

  Py_RETURN_NONE;
}


PyTypeObject
WsgiStartResponseType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "wsgi.ResponseStarter",         /* tp_name */
  sizeof(WsgiStartResponseObject),/* tp_basicsize */
  0,                              /* tp_itemsize */
  0,                              /* tp_dealloc */
  0,                              /* tp_print */
  0,                              /* tp_getattr */
  0,                              /* tp_setattr */
  0,                              /* tp_reserved */
  0,                              /* tp_repr */
  0,                              /* tp_as_number */
  0,                              /* tp_as_sequence */
  0,                              /* tp_as_mapping */
  0,                              /* tp_hash  */
  (ternaryfunc)__call__,          /* tp_call */
  0,                              /* tp_str */
  0,                              /* tp_getattro */
  0,                              /* tp_setattro */
  0,                              /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,             /* tp_flags */
  "",                             /* tp_doc */
  0,                              /* tp_traverse */
  0,                              /* tp_clear */
  0,                              /* tp_richcompare */
  0,                              /* tp_weaklistoffset */
  0,                              /* tp_iter */
  0,                              /* tp_iternext */
  0,                              /* tp_methods */
  0,                              /* tp_members */
  0,                              /* tp_getset */
  0,                              /* tp_base */
  0,                              /* tp_dict */
  0,                              /* tp_descr_get */
  0,                              /* tp_descr_set */
  0,                              /* tp_dictoffset */
  (initproc)0,                    /* tp_init */
  0,                              /* tp_alloc */
  0,                              /* tp_new */
};
} // ns

////////////////////////////////////////////////////////////////////////////////

PyObject*
New(fcgi::HttpResponse* res)
{
  static bool Initialized = false;

  if (!Initialized) {
    WsgiStartResponseType.tp_new = PyType_GenericNew;
    PyType_Ready(&WsgiStartResponseType);
    Initialized = true;
  }

  // -
  WsgiStartResponseObject* self =
      (WsgiStartResponseObject*)WsgiStartResponseType.tp_alloc(
          (PyTypeObject*) &WsgiStartResponseType, 0);

  // FIXME
  if (self == nullptr)
    return nullptr;

  self->res = res;

  return (PyObject*) self;
}
