#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


#include "WsgiReader.hpp"
#include "PythonHelper.hpp"

using namespace fcgi;


namespace {
struct WsgiReaderObject {
  PyObject_HEAD
  HttpRequest* req = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
PyObject*
Read(PyObject* self_, PyObject* args, PyObject**)
{
  Py_ssize_t size;
  if (!PyArg_ParseTuple(args, "n", &size)) {
    return nullptr;
  }

  std::vector<uint8_t> data(size);
  auto self = (WsgiReaderObject*)self_;
  self->req->recv(data.data(), data.size());

  return PyBytes_FromStringAndSize((char*)data.data(), data.size());
}

PyObject*
ReadLine(PyObject* self_, PyObject*, PyObject**)
{
  auto self = (WsgiReaderObject*)self_;
  Istream istr = self->req->ToStream();

  std::string line;
  std::getline(self->req->ToStream(), line);

  return PyBytes_FromStringAndSize(line.c_str(), line.size());
}

PyObject*
ReadLines(PyObject* self_, PyObject*, PyObject**)
{
  auto self = (WsgiReaderObject*)self_;

  Istream istr = self->req->ToStream();
  PyObject* dataList = PyList_New(0);
  std::string line;

  while (std::getline(istr, line)) {
    PyObject* data = PyBytes_FromStringAndSize(line.c_str(), line.size());
    PyList_Append(dataList, data);
  }

  return dataList;
}

PyObject*
SelfIterator(PyObject* self_)
{
  auto self = (WsgiReaderObject*)self_;
  std::vector<uint8_t> buffer(4096);

  auto read = self->req->recv(buffer.data(), buffer.size());
  if (0 == read)
    return nullptr;
  else
    return PyBytes_FromStringAndSize((char*)buffer.data(), buffer.size());
}

PyMethodDef
WsgiReaderMethods[] = {
  {
    "read",
    (PyCFunction)Read,
    METH_VARARGS,
    ""
  },
  {
    "readline",
    (PyCFunction)ReadLine,
    METH_VARARGS,
    ""
  },
  {
    "readlines",
    (PyCFunction)ReadLines,
    METH_VARARGS,
    ""
  },
  {nullptr}
};

//----------------------------------------------------------------------------//

PyTypeObject
WsgiReaderType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "wsgi.reader",                /* tp_name */
  sizeof(WsgiReaderObject),     /* tp_basicsize */
  0,                            /* tp_itemsize */
  0,                            /* tp_dealloc */
  0,                            /* tp_print */
  0,                            /* tp_getattr */
  0,                            /* tp_setattr */
  0,                            /* tp_reserved */
  0,                            /* tp_repr */
  0,                            /* tp_as_number */
  0,                            /* tp_as_sequence */
  0,                            /* tp_as_mapping */
  0,                            /* tp_hash  */
  0,                            /* tp_call */
  0,                            /* tp_str */
  0,                            /* tp_getattro */
  0,                            /* tp_setattro */
  0,                            /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,           /* tp_flags */
  "",                           /* tp_doc */
  0,                            /* tp_traverse */
  0,                            /* tp_clear */
  0,                            /* tp_richcompare */
  0,                            /* tp_weaklistoffset */
  PyObject_SelfIter,            /* tp_iter */
  SelfIterator,                 /* tp_iternext */
  WsgiReaderMethods,            /* tp_methods */
  0,                            /* tp_members */
  0,                            /* tp_getset */
  0,                            /* tp_base */
  0,                            /* tp_dict */
  0,                            /* tp_descr_get */
  0,                            /* tp_descr_set */
  0,                            /* tp_dictoffset */
  (initproc)0,                  /* tp_init */
  0,                            /* tp_alloc */
  0,                            /* tp_new */
};
} // ns

////////////////////////////////////////////////////////////////////////////////

PyObject*
New(fcgi::HttpRequest* req)
{
  static bool Initialized = false;

  bits::AutoGIL gil;

  if (!Initialized) {
    WsgiReaderType.tp_new = PyType_GenericNew;
    PyType_Ready(&WsgiReaderType);
    Initialized = true;
  }

  // -
  WsgiReaderObject* self =
      (WsgiReaderObject*)WsgiReaderType.tp_alloc(
          (PyTypeObject*) &WsgiReaderType, 0);

  // FIXME
  if (self == nullptr)
    return nullptr;

  self->req = req;

  return (PyObject*) self;
}
