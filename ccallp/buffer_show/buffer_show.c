#include <python3.6m/Python.h>
//gcc buffer_show.c -L/usr/lib/aarch64-linux-gnu -lpython3.6m -o buffer_show
/* 
void write_file(char *data, int len)
{
    static int		m = 0;
    FILE 			*fp = nullptr;
    char			filename[1024] = {0};
 
    snprintf(filename, sizeof(filename), "image%02d.yuv", m++);
 
    if( (fp=fopen(filename, "wb+")) == NULL ){  //以二进制方式打开
        return;
    }
 
    fwrite(data, len, 1, fp);
    fclose(fp);
}
*/

int main()
{
	/*读取YUV图像*/
	FILE *yuv_info;
	yuv_info = fopen("pico_dump_yuv420sp.yuv","rb");
 
	if(!yuv_info){
		printf("打开YUV文件失败！");
		return -1;
	}else{
    printf("打开YUV文件成功！");
  }
 
	/*将YUV图像保存到指定大小的buffer中*/
	int len = 504*378*3/2;
	char *buff_info = (char *)malloc(len*sizeof(char));
	fread(buff_info, sizeof(char), len, yuv_info);
  //printf("%s",buff_info);
 
 
	/*将buffer数据重新存放到图像*/
	//write_file(buff_info,len);
 
	
	
	/**********************************调用Python函数*******************************************/
	//初始化
	Py_Initialize();
	if (!Py_IsInitialized())
	{
		printf("python初始化失败！");
		return -1;
    }
	//相当于在python中的import sys语句。这个函数是宏，相当于直接运行python语句
	PyRun_SimpleString("import sys");
 	//PyRun_SimpleString("from PIL import Image");
 	//PyRun_SimpleString("from numpy import *");
  //PyRun_SimpleString("import matplotlib.pyplot as plt");
  //PyRun_SimpleString("sys.path.append('/home/yucheng/anaconda3/lib/python3.10/site-packages')");
	PyRun_SimpleString("sys.path.append('./')");//是将搜索路径设置为当前目录。
  PyRun_SimpleString("print(sys.path)");

 /*
	PyRun_SimpleString("import numpy as np");
	PyRun_SimpleString("import matplotlib.pyplot as plt");
 */
	PyObject *pModule = PyImport_ImportModule("buffer_show"); //导入hello.py
	if (!pModule)
	{
		printf("cannot find buffer_show.py\n");
		return -1;
	}
	else
	{
		printf("PyImport_ImportModule success\n");
	}
	
	PyObject *pDict = PyModule_GetDict(pModule);
	if (!pModule)
	{
		printf("cannot find buffer_show.py\n");
		return -1;
	}
	else
	{
		printf("PyModule_GetDict success\n");
	}
    //assert(pDict != NULL);
	
	//char* yuv
  //PyObject* pFuncDispaly = PyDict_GetItemString(pDict, "array2dispaly");
  PyObject* pFuncDispaly = PyDict_GetItemString(pDict, "array3dispaly");
  PyObject* pArgDispaly = Py_BuildValue("(s)", buff_info);
  PyObject* resultDispaly = PyEval_CallObject(pFuncDispaly, pArgDispaly);
  /* 
  int c;
  PyArg_Parse(resultDispaly, "i", &c);
  printf("call yuv frame result=%d\n", c);
  */
  
  char* yuvlen;
  PyArg_Parse(resultDispaly, "i", &yuvlen);
  printf("call yuv len result=%s\n", yuvlen);
  
	

  Py_DECREF(pFuncDispaly);
  Py_DECREF(pArgDispaly);
  Py_DECREF(resultDispaly);
	
	
	/*释放上面存储图像buffer的内存*/
  free(buff_info);
  Py_Finalize();
  return 0;
}