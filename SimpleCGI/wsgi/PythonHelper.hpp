#ifndef PYTHON_HELPER_HPP_
#define PYTHON_HELPER_HPP_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <Python.h>
#pragma GCC diagnostic pop

namespace fcgi { namespace bits {

class Decref {
public:
  Decref(PyObject* data = nullptr)
      : data(data)
  {}

  ~Decref()
  {
    if (data)
      Py_DECREF(data);
  }

  Decref&
  operator=(PyObject* rhs)
  {
    if (data)
      Py_DECREF(data);

    data = rhs;
    return *this;
  }

  operator PyObject*()
  { return data; }

  PyObject*
  operator->()
  { return data; }

private:
  PyObject* data;
};

class AutoGIL {
public:
  AutoGIL()
    : gil(PyGILState_Ensure()) {}

  ~AutoGIL()
  { PyGILState_Release(gil); }

private:
  PyGILState_STATE gil;
};
} // ns bits
} // ns fcgi
#endif
