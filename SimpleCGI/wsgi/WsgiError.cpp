#include "WsgiError.hpp"

#if 0
using namespace fcgi;


namespace {
struct WsgiWriterObject {
  PyObject_HEAD
  HttpResponse* res = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

PyMethodDef
WsgiWriterMethods[] = {
  {
    "flush",
    (PyCFunction)nullptr,
    METH_NOARGS,
    ""
  },
  {
    "write",
    (PyCFunction)nullptr,
    METH_VARARGS,
    ""
  },
  {
    "writelines",
    (PyCFunction)nullptr,
    METH_VARARGS,
    ""
  },
  {nullptr}
};

//----------------------------------------------------------------------------//

PyTypeObject
WsgiWriterType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "wsgi.error",                 /* tp_name */
  sizeof(WsgiWriterObject),     /* tp_basicsize */
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
  WsgiWriterMethods,            /* tp_methods */
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
New(fcgi::HttpResponse* res)
{
  static bool Initialized = false;

  if (!Initialized) {
    WsgiWriterType.tp_new = PyType_GenericNew;
    PyType_Ready(&WsgiWriterType);
    Initialized = true;
  }

  // -
  WsgiWriterObject* self =
      (WsgiWriterObject*)WsgiWriterType.tp_alloc(
          (PyTypeObject*) &WsgiWriterType, 0);

  // FIXME
  if (self == nullptr)
    return nullptr;

  self->res = res;

  return (PyObject*) self;
}
#endif
