#include "WsgiReader.hpp"

using namespace fcgi;


namespace {
struct WsgiReaderObject {
  PyObject_HEAD
  HttpRequest* req = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

PyMethodDef
WsgiReaderMethods[] = {
  {
    "read",
    (PyCFunction)nullptr,
    METH_VARARGS,
    ""
  },
  {
    "readline",
    (PyCFunction)nullptr,
    METH_VARARGS,
    ""
  },
  {
    "readlines",
    (PyCFunction)nullptr,
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
  0,                            /* tp_iter */
  0,                            /* tp_iternext */
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
