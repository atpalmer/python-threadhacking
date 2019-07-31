#include <pthread.h>
#include <unistd.h>
#include <Python.h>

struct thread_proc_args {
    PyObject *func;
    PyObject *func_args;
};

static void *thread_proc(void *_args) {
    struct thread_proc_args *args = _args;
    PyObject *result = PyEval_CallObject(args->func, args->func_args);
    return result;
}

typedef struct {
    PyObject_HEAD
    pthread_t ob_thread;
    struct thread_proc_args ob_args;
} MyThreadObject;

static PyObject *MyThreadObject_New(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    PyObject *new = (PyObject *)type->tp_alloc(type, 0);
    ((MyThreadObject *)new)->ob_thread = 0;
    ((MyThreadObject *)new)->ob_args.func = Py_None;
    ((MyThreadObject *)new)->ob_args.func_args = Py_None;
    return new;
}

static int MyThreadObject_Init(MyThreadObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *func;
    PyObject *func_args;

    static char *kwlist[] = {"func", "args", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kwlist, &func, &func_args))
        return -1;
    if(!PyFunction_Check(func))
        return -1;
    if(!PyTuple_Check(func_args))
        return -1;

    self->ob_args.func = func;
    self->ob_args.func_args = func_args;
    Py_INCREF(self->ob_args.func);
    Py_INCREF(self->ob_args.func_args);

    pthread_create(&self->ob_thread, NULL, thread_proc, &self->ob_args);
    return 0;
}

static void MyThreadObject_Dealloc(MyThreadObject *self) {
    Py_DECREF(self->ob_args.func);
    Py_DECREF(self->ob_args.func_args);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *MyThreadObject_join(PyObject *self, PyObject *args) {
    void *thread_result;
    pthread_join(((MyThreadObject *)self)->ob_thread, &thread_result);
    Py_INCREF(thread_result);
    return thread_result;
}

static PyMethodDef MyThreadObject_Methods[] = {
    {"join", MyThreadObject_join, METH_NOARGS, ""},
    {0},
};

static PyTypeObject pthread_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Thread",
    .tp_doc = "",
    .tp_basicsize = sizeof(MyThreadObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = MyThreadObject_New,
    .tp_init = MyThreadObject_Init,
    .tp_dealloc = MyThreadObject_Dealloc,
    .tp_methods = MyThreadObject_Methods,
};

static PyModuleDef moddef = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "pthread",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_pthread(void) {
    PyObject *mod = PyModule_Create(&moddef);

    if(PyType_Ready(&pthread_type) < 0)
        return NULL;
    Py_INCREF(&pthread_type);
    PyModule_AddObject(mod, "Thread", (PyObject *)&pthread_type);

    return mod;
}
