#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "string.h"
#include "dirent.h"
 
  //#########################################################//
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
            continue;//����.��..Ŀ¼
        else if(ptr->d_type == 8)    ///d_type=8��Ӧfile
        {  
            CurFileExtName = rindex(ptr->d_name, '.');//char *rindex(const char *s, int c);rindex()�����ҳ�����s �ַ��������һ�����ֵĲ���c ��ַ��Ȼ�󽫸��ַ����ֵĵ�ַ���ء��ַ��������ַ�(NULL)Ҳ��Ϊ�ַ���һ���֡�
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
//#########################################################//
 
 
int main(void)
{
   
   //char *DirPath="./";
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
   } 
 
}