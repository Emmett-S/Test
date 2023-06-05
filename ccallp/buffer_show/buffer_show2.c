//我在实现Python函数简单的调用后，测试了一下将图片路径作为字符串传参给torch项目让他找到图片后读取、模型运算、输出结果后返回结果给C并且在C这端输出运算结果，这么操作没有问题完全可实现。但是我们的要求是不落盘，那就不会存在一个图片的路径，图片只会存在在内存空间里，为了实现图片的传参先后尝试了三种方法：二进制流、opencv的Mat数组、Base64编码

/*
三种方式的尝试结果。
  1-二进制流：二进制流对于C语言端来说实现最为简单，只要通过rb的格式去fopen一个图片就能获取到图片的二进制流，但是怎么把二进制流转变成string字符串这个点真是结结实实的难到了我，搜遍百度谷歌后，得出一个普遍的答案是创建一个结构体后把二进制流放在结构体内再进行传参，但是由于本人C水平有限，试错成本高时间又比较紧，放弃。
  
  2-opencv的Mat数组：本以为opencv是最有希望的，毕竟是一个在C端和Python端都存在的一个库，对于图片解析的风格应该都一致啊，在网上也搜到绝大多数C调用Python并传参图片的教程都是以opencv Mat转numpy.array完成的。就在我装完opencv准备ctrlC+V尝试的时候，发现编译C文件一堆报错，再一细看网上的代码，居然全是C++的相关代码，可是人家C根本就没有Mat这个概念啊，人家只有Iplmage格式啊，我要强行要用只能自己去解析一遍Iplmage结构体内容并想办法把这个格式转成对应大小的三维数组，再次因为C水平不到家，放弃。
  
  3-Base64编码：经过和擅长C的同事讨论，他认为二进制流在转换成Python可理解的string格式时出现了转换函数无法理解的字符导致了转换失败。既然二进制流转string费劲，那我何不直接转Python完全可以理解的base64编码呢，遂在网上找了个C语言将二进制流转换为base64编码的教程并且照做了一下，生成后转换成Python的string也很顺利，在Python端对base64进行解码后成功获取到了图片并运算返回了结果，至此完成了图片在内存中的交互，成功！
  
  Base64编码见buffer_show2文件
*/

#include <python3.6m/Python.h>
//#include <numpy/ndarrayobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 图片二进制流编码成为base64方便python理解并提取内容
char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int main(int argc,char **argv)
//int main()
{

    /*以二进制方式读取YUV图像*/
    FILE *yuv_info;
	  yuv_info = fopen("pico_dump_yuv420sp.yuv","rb");
 
	  if(!yuv_info){
		  printf("打开YUV文件失败！");
		  return -1;
	  }else{
      printf("打开YUV文件成功！\n");
    }
 
	  /*将YUV图像保存到指定大小的buffer中*/
    /*
	  int len = 504*378*3/2;
    char* image_buff = (char *)malloc(len* sizeof(char));
	  fread(image_buff, sizeof(char), len, yuv_info);
    printf("buff_info大小为：%ld\n",sizeof(image_buff));
    fclose(yuv_info);
    int size = len;
    */
    
    
  	fseek(yuv_info, 0, SEEK_END);
    long int size = ftell(yuv_info);
    rewind(yuv_info);
    
    //根据图像数据长度分配内存buffer
	  char* image_buff = (char*)malloc(size* sizeof(char));
	  fread(image_buff, size, 1, yuv_info);
    fclose(yuv_info);
    

    //创建图像base64编码buffer
    char* imgbuffer_b64;
	  char *ret1; 
	  unsigned int length;
	  imgbuffer_b64 = (char *)malloc((size/4+1)*16/3);
	  if (NULL == imgbuffer_b64)
    {
	      printf("memory_error");
	      exit(2);
    }
    ret1 = base64_encode(image_buff, imgbuffer_b64, size);
    printf("%s\n", ret1);
    free(image_buff);
    length = strlen(imgbuffer_b64);
    
 
	  /*将buffer数据重新存放到图像*/
	  //write_file(buff_info,len);


/**********************************调用Python函数*******************************************/
    PyObject *pModule, *pDict, *pArgs;
    //PyObject *result, *result1;
    
    /*
    if (argc < 3) 
    {
        printf("Usage: exe_name python_source function_name\n");
        return 1;
    }
    */
    
    //初始化python
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        printf("python初始化失败！");
        return 0;
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    /*
    PyRun_SimpleString("from PIL import Image");
    PyRun_SimpleString("import numpy as np");
    PyRun_SimpleString("import matplotlib.pyplot as plt");
    */
    
    
    /*
    //main命令行参数
   	int argc = 1;
    wchar_t * argv[] = { L" " };
    PySys_SetArgv(argc, argv);   //加入argv参数 否则出错
    */

    

    
    pModule = PyImport_ImportModule("buffer_show2");
    //assert(pModule != NULL);
    if ( !pModule ) {  
        printf("can't find py_tobeCalled.py");  
        getchar();  
        return -1;  
    }
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
    //assert(pDict != NULL);
    if ( !pDict ) {  
        PyRun_SimpleString("print('no pDict')");
        return -1;  
    } 
    //PyObject* pFuncAdd = PyDict_GetItemString(pDict, "array3dispaly");
    PyObject* pFuncDisp = PyDict_GetItemString(pDict, "array2dispaly");
    if ( !pFuncDisp || !PyCallable_Check(pFuncDisp) ) {  
        printf("can't find function [array2dispaly]");  
        getchar();  
        return -1;  
    }
    
    // 参数进栈  
    //创建元组
    pArgs = PyTuple_New(1);
      
   	PyObject *args = PyTuple_New(1);
    //PyObject *pArgDisp = Py_BuildValue("(s)", (const char*)imgbuffer_b64);
    //这里传入的应该是具体图像数据，而不是buffer地址
    PyObject *pArgDisp = Py_BuildValue("(s)", (const char*)ret1);
    PyTuple_SetItem(args, 0, pArgDisp);
    
    //不建立元组直接传输
    //PyObject *pArgDisp = Py_BuildValue("(z)", (const char*)imgbuffer_b64);
    
    
    //PyObject* pArgAdd = Py_BuildValue("s", buff_info);
    PyObject *resultDisp = PyEval_CallObject(pFuncDisp, args);
    
    int c;
    PyArg_Parse(resultDisp, "i", &c);
    printf("call Disp result=%d\n", c);
    
    Py_DECREF(pFuncDisp);
    Py_DECREF(args);
    Py_DECREF(resultDisp);
/* 
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
    */
 
    Py_DECREF(pModule);
    Py_DECREF(pDict);
    //Py_DECREF(pClass);
    //Py_DECREF(pInstance);
    //PyRun_SimpleString("print('-'*10, 'decref end', '-'*10)");
    Py_Finalize();
    return 0;
}