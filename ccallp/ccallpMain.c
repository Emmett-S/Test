#include "ccallp.h"
#include <stdio.h>
#include <stdlib.h>
 
// 执行命令：gcc ccallp.c ccallpMain.c -o ccallpMain -I/usr/include/python2.7/ -lpython2.7
//  ./ccallpMain
 
int main(int argc, char *argv[])
{
	printf("%d\n",great_function_from_python(3));
 
} 