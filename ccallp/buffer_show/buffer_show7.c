#include <python3.6m/Python.h>
//#include <python2.7/Python.h>
//如果替换成python2.7版本，除了头文件和编译命令替换外，还需要将引入py文件的代码进行替换，但是由于py中引入了PIL，现在pip安装的都是python3的PIL也就是pillow，pip已停止支持python2，所以python2中没有PIL模块，导致出错，C文件无法建立py文件对象
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <malloc.h>
#include <stddef.h>
//gcc buffer_show5.c -L/usr/lib/aarch64-linux-gnu -lpython3.6m -o buffer_show5
//buffer_show5将初始化python解释器的部分放在main函数中，替换dom时，需要把所有功能写在函数中
//buffer_show6将initializing和finalize以及引入第三方库sys部分分别写在函数中，避免多次初始化
//获取文件夹下指定后缀的文件名
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

//读取图像存入buffer,返回base64编码buffer
//PyObject *pModule, *pDict;
char* img2base64(char *YUVPath, int i)
{

    /*以二进制方式读取YUV图像*/
    //printf("%s",YUVPath);
    FILE *yuv_info;
	  yuv_info = fopen(YUVPath,"rb");
 
	  if(!yuv_info){
		  printf("打开YUV文件失败！");
		  return NULL;
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
    if(length == ((size/4+1)*16/3)){
        printf("长度一样\n");
    }else{
        printf("长度不一样\n");
    }
    return ret1;
    
    
 
	  /*将buffer数据重新存放到图像*/
	  //write_file(buff_info,len);

    return 0;
}


static PyObject* pModule = NULL;
static PyObject* pName = NULL;
static PyObject* pDict = NULL;
static int py_initialized = 0;

//初始化python
int py_init(void)
{
    int ret = 0;
    if(py_initialized == 0)
    {
        Py_Initialize();
        if (!Py_IsInitialized())
        {
            printf("python初始化失败！");
            ret = -1;
            return ret;
        }
        else{printf("python初始化成功！\n");}
        
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./')");
        //pName = PyString_FromString("buffer_show6");
        //if (pName == NULL)
        //{
        //    printf("PyString_FromString() error!");
        //    return  -1;
        //}
        //pModule = PyImport_Import(pName);
        pModule = PyImport_ImportModule("buffer_show6");
        
        if ( !pModule ) {  
            printf("can't find py_tobeCalled.py");  
            getchar(); 
            ret = -1; 
            return ret;  
        }else{
            printf("pModule成功！\n");
        }
        
        pDict = PyModule_GetDict(pModule);
        if ( !pDict ) {  
            PyRun_SimpleString("print('no pDict')");
            return -1;  
        }else{
            printf("pDict成功!\n");
        }
        
        if(pName) Py_INCREF(pName);
        if(pModule) Py_INCREF(pModule);
        if(pDict) Py_INCREF(pDict);
        py_initialized = 1;
    }
    
    
    printf("py init done\n");
    return ret;
    

}

void py_deinit(void)
{

    if(pName) Py_XDECREF(pDict);
    if(pModule) Py_XDECREF(pModule);
    if(pDict) Py_XDECREF(pName);
    
    Py_Finalize();
    py_initialized = 0;

}

int buffshow(char* ret1)
{
    int ret;
    char *pfunc_name = "array2dispaly";
    PyObject* pFuncDisp = PyDict_GetItemString(pDict, pfunc_name);
    if ( !pFuncDisp || !PyCallable_Check(pFuncDisp) ) {  
        printf("can't find function:%s\n",pfunc_name);  
        getchar(); 
        ret = -1; 
        return ret;  
    }else{
        printf("find function %s成功!\n",pfunc_name);
    }
    
    //将序列图像buffer传入python函数
    // 参数进栈  
    //创建元组
    PyObject *args = PyTuple_New(1);
    //传入具体图像数据
    PyObject *pArgDisp = Py_BuildValue("(s)", (const char*)ret1);
    PyTuple_SetItem(args, 0, pArgDisp);
    PyObject *resultDisp = PyEval_CallObject(pFuncDisp, args);
    
    Py_XDECREF(resultDisp);
    Py_XDECREF(pArgDisp);
    Py_XDECREF(args);
    
    return 0;

}


int main(int argc,char **argv)
{

    py_init();
    
    
    //读取目录下的文件序列
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
        char* ret1 = img2base64((char*)FileNames[i],i);
        
        buffshow(ret1);
        
        
    } 
    
    py_deinit();
 
    return 0;
}