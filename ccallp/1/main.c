#include <python3.6/Python.h>
 
int main()
{
    Py_Initialize(); //初始化
    if (!Py_IsInitialized())
    {
        return -1; //init python failed
    }
    //相当于在python中的import sys语句。这个函数是宏，相当于直接运行python语句
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");//是将搜索路径设置为当前目录。
    PyObject *pmodule = PyImport_ImportModule("hello"); //导入hello.py
    if (!pmodule)
    {
        printf("cannot find hello.py\n");
        return -1;
    }
    else
    {
        printf("PyImport_ImportModule success\n");
    }
 
    PyObject *pfunc = PyObject_GetAttrString(pmodule, "func1"); //导入func1函数
    if (!pfunc)
    {
        printf("cannot find func\n");
        Py_XDECREF(pmodule);
        return -1;
    }
    else
    {
        printf("PyObject_GetAttrString success\n");
    }
    
    // 向Python传参数是以元组（tuple）的方式传过去的，
    // 因此我们实际上就是构造一个合适的Python元组就可以了
    // 要用到PyTuple_New，Py_BuildValue，PyTuple_SetItem等几个函数
    /*这个元组其实只是传参的载体
    创建几个元素python函数就是几个参数,这里创建的元组是作为1个参数传递的*/
    PyObject *pArgs = PyTuple_New(3);
    PyObject *pVender = Py_BuildValue("i", 2);     //构建参数1
    PyObject *pDataID = Py_BuildValue("i", 10001); //构建参数2
    PyObject *pyTupleList = PyTuple_New(2);        //构建参数3,这里创建的元组是作为c的数组
    float arr_f[2];
    arr_f[0] = 78;
    arr_f[1] = 3.41;
    for (int i = 0; i < 2; i++)
    {
        //这里是把c数组构建成python的元组
        PyTuple_SetItem(pyTupleList, i, Py_BuildValue("f", arr_f[i])); 
    }
 
    //参数入栈
    PyTuple_SetItem(pArgs, 0, pVender);
    PyTuple_SetItem(pArgs, 1, pDataID);
    PyTuple_SetItem(pArgs, 2, pyTupleList);
    
    //调用python脚本函数
    PyObject *pResult = PyObject_CallObject(pfunc, pArgs);
    int a;
    float b;
    // PyArg_Parse(pResult, "i", &a);
    PyArg_ParseTuple(pResult,"if",&a,&b);
    printf("%d %f\n", a,b);
 
    //释放资源
    Py_XDECREF(pmodule);
    Py_XDECREF(pfunc);
    Py_XDECREF(pArgs);
    Py_XDECREF(pResult);
 
    Py_Finalize();
    
    return 0;
}