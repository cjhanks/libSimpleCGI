#ifndef PYTHON_HELPER_HPP_
#define PYTHON_HELPER_HPP_

#include <Python.h>

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

} // ns bits
} // ns fcgi
#endif
