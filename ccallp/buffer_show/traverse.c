#include <stdio.h> 
#include <stdlib.h>    
#include <string.h>  
#include <dirent.h> 

#define MAX_LEN 65535

int main(void) 
{ 
    DIR *dir; 
    struct dirent *ptr; 
    char *flow[MAX_LEN];
    int num = 0, i = 0;
   
    if ((dir=opendir("./")) == NULL) 
    //if ((dir=opendir("./data")) == NULL) 
    { 
        perror("Open dir error..."); 
        exit(1);        
    } 
    // readdir() return next enter point of directory dir
    while ((ptr=readdir(dir)) != NULL) 
    { 
        flow[num] = (char*)malloc(sizeof(char));
        strcpy(flow[num], ptr->d_name);
        num++;
    } 

    for(i = 0; i < num; i++)
    {
        printf("%s\n", flow[i]);
    }
   
    closedir(dir); 
}