/*
 * @Author: your name
 * @Date: 2020-11-16 11:19:26
 * @LastEditTime: 2020-11-16 12:15:52
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git_proj\malloc\test.c
 */

#include <stdio.h>
#include "mm.h"

int main()
{

    mem_init();                                     //初始化模型
	mm_init();                                      //初始化分配器
	int *i = mm_malloc(sizeof(int));
	*i = 10;
	char *c = mm_malloc(sizeof(char));
	*c = 'c';
	double *d = mm_malloc(sizeof(double));
	*d = 10.0;
	printf("i:%d\n",*i);
	printf("c:%d\n",*c);
	printf("d:%d\n",*d);
}