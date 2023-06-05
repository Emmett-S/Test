#include <python3.6m/Python.h>
int main(int argc,char **argv)
{
    PyObject *pModule, *pDict, *pClass, *pInstance;
    PyObject *result, *result1;
    //初始化python
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        printf("python初始化失败！");
        return 0;
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    pModule = PyImport_ImportModule("test_cpp_call_py");
    assert(pModule != NULL);
    
    PyObject *pFuncHello = PyObject_GetAttrString(pModule, "Hello");
    PyObject *pArgHello = Py_BuildValue("(s)", "Hello Charity");
    PyObject *resultHello = PyEval_CallObject(pFuncHello, pArgHello);
    char *hello = NULL;
    PyArg_Parse(resultHello,"s",&hello);
    printf("call Hello result=%s\n", hello);
    Py_DECREF(pFuncHello);
    Py_DECREF(pArgHello);
    Py_DECREF(resultHello);
 
    pDict = PyModule_GetDict(pModule);
    assert(pDict != NULL);
    PyObject* pFuncAdd = PyDict_GetItemString(pDict, "Add");
    PyObject* pArgAdd = Py_BuildValue("(i, i)", 1, 2);
    PyObject *resultAdd = PyEval_CallObject(pFuncAdd, pArgAdd);
    int c;
    PyArg_Parse(resultAdd, "i", &c);
    printf("call Add result=%d\n", c);
    Py_DECREF(pFuncAdd);
    Py_DECREF(pArgAdd);
    Py_DECREF(resultAdd);
 
    //通过字典属性获取模块中的类
    pClass = PyDict_GetItemString(pDict, "Person");
    assert(pClass != NULL);
 
    pInstance = PyObject_CallObject(pClass, NULL);
    assert(pInstance != NULL);
    
    PyRun_SimpleString("print('-'*10, 'Python start', '-'*10)");
    result = PyObject_CallMethod(pInstance, "getInfo", "");
    PyObject_CallMethod(pInstance, "setInfo", "si", "tyl", 24);
    result1 = PyObject_CallMethod(pInstance, "getInfo", "");
    char* name;
    int age;
    PyArg_ParseTuple(result, "si", &name, &age);
    printf("result:%s-%d\n", name, age);        
    PyArg_ParseTuple(result1, "si", &name, &age);
    printf("result1:%s-%d\n", name, age);
    PyRun_SimpleString("print('-'*10, 'Python end', '-'*10)");
    Py_DECREF(result);
    Py_DECREF(result1);
 
    Py_DECREF(pModule);
    Py_DECREF(pDict);
    Py_DECREF(pClass);
    Py_DECREF(pInstance);
    PyRun_SimpleString("print('-'*10, 'decref end', '-'*10)");
    Py_Finalize();
}