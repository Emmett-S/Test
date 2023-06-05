#include <python3.6m/Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <malloc.h>
#include <stddef.h>
//gcc buffer_show4.c -L/usr/lib/aarch64-linux-gnu -lpython3.6m -o buffer_show4
//获取文件夹下指定后缀的文件名
//尝试循环读取图像序列，但重复初始化python解释器和引入第三方库sys导致core dumped
int GetFileNamesInDir(char *DirPath,char *FileExtName,char FileNames[][128],int *FileNum,int MaxFileNum)
{
    DIR *dir;
    struct dirent *ptr;
 
    if ((dir=opendir(DirPath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
    
    char *CurFileExtName = NULL;
    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) ///current dir OR parrent dir
            continue;//跳过.和..目录
        else if(ptr->d_type == 8)    ///d_type=8对应file
        {  
            CurFileExtName = rindex(ptr->d_name, '.');//char *rindex(const char *s, int c);rindex()用来找出参数s 字符串中最后一个出现的参数c 地址，然后将该字符出现的地址返回。字符串结束字符(NULL)也视为字符串一部分。
            if(CurFileExtName!=NULL&& strcmp(CurFileExtName,FileExtName) == 0)
            {  
                if(*FileNum<MaxFileNum)
                {
                   memcpy(FileNames[(*FileNum)++],ptr->d_name,sizeof(ptr->d_name)); 
                   //printf("CurFilePath=%s/%s\n",DirPath,ptr->d_name);
                }
                
            }     
        }     
    }
    closedir(dir);
 
    return 1;
}

//gcc buffer_show3.c -L/usr/lib/aarch64-linux-gnu -lpython3.6m -o buffer_show3
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 图片二进制流编码成为base64传输给python脚本
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

//读取图像存入buffer，调用python脚本显示
//PyObject *pModule, *pDict;
int BufferShow(char *YUVPath, int i)
{

    /*以二进制方式读取YUV图像*/
    //printf("%s",YUVPath);
    FILE *yuv_info;
	  yuv_info = fopen(YUVPath,"rb");
 
	  if(!yuv_info){
		  printf("打开YUV文件失败！");
		  return -1;
	  }else{
      printf("打开YUV文件成功！\n");
    }
 
	  /*获取yuv_info大小*/

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
    }else
    {
        printf("memory_success\n");
    }
    ret1 = base64_encode(image_buff, imgbuffer_b64, size);
    //printf("%s\n", ret1);
    free(image_buff);
    length = strlen(imgbuffer_b64);
    
    
 
	  /*将buffer数据重新存放到图像*/
	  //write_file(buff_info,len);


/**********************************调用Python函数*******************************************/

    //PyObject *pModule, *pDict;

    //初始化python
    
    
    Py_Initialize();
    
    if (!Py_IsInitialized())
    {
        printf("python初始化失败！");
        return 0;
    }else{
        printf("python初始化成功！:%s\n",YUVPath);
        
    }

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");


    PyObject *pModule = PyImport_ImportModule("buffer_show4");
    printf("size%ld\n",sizeof(pModule));
    if ( !pModule ) {  
        printf("can't find py_tobeCalled.py");  
        getchar();  
        return -1;  
    }else{
        printf("pModule成功！:%s\n",YUVPath);
    }
    
    PyObject *pDict = PyModule_GetDict(pModule);
    if ( !pDict ) {  
        PyRun_SimpleString("print('no pDict')");
        return -1;  
    }else{
        printf("pDict成功！:%s\n",YUVPath);
    } 
    //PyObject* pFuncAdd = PyDict_GetItemString(pDict, "array3dispaly");
    PyObject* pFuncDisp = PyDict_GetItemString(pDict, "array2dispaly");
    if ( !pFuncDisp || !PyCallable_Check(pFuncDisp) ) {  
        printf("can't find function [array2dispaly]");  
        getchar();  
        return -1;  
    }else{
        printf("find function [array2dispaly]成功！:%s\n",YUVPath);
    }
    
    
    
    // 参数进栈  
    //创建元组
   	PyObject *args = PyTuple_New(1);
    //传入具体图像数据
    PyObject *pArgDisp = Py_BuildValue("(s)", (const char*)ret1);
    PyTuple_SetItem(args, 0, pArgDisp);
    
        
    PyObject *resultDisp = PyEval_CallObject(pFuncDisp, args);
    /*
    int c;
    PyArg_Parse(resultDisp, "i", &c);
    printf("call Disp result=%d\n", c);
    */
    Py_XDECREF(pArgDisp);
    Py_XDECREF(args);
    
    Py_XDECREF(pFuncDisp);
    
    Py_XDECREF(resultDisp);

 
    Py_XDECREF(pModule);
    Py_XDECREF(pDict);
    
    
    //PyRun_SimpleString("sys.path.remove('./')");

    Py_Finalize();

    
    return 0;
}


int main(int argc,char **argv)
{
    char *DirPath=".";
    char *FileExtName=".yuv";
    char FileNames[1000][128];
    int MaxFileNum=1000;
    int FileNum=0;
    GetFileNamesInDir(DirPath,FileExtName,FileNames,&FileNum,MaxFileNum);
    printf("########Find include %s suffix FileNum=%d########\n",FileExtName,FileNum);
    for(int i=0;i<FileNum;i++)
    {
        printf("CurFilePath=%s/%s\n",DirPath,FileNames[i]);
        printf("%s\n",FileNames[i]);
        //typeof(FileNames[i]);
        //char *FilePath = strcat(DirPath, FileNames[i]);
        //BufferShow("pico_dump_yuv420sp.yuv");
        BufferShow((char*)FileNames[i],i);
    } 
    
   
    return 0;
}