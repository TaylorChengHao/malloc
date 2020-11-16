/*
 * @Author: your name
 * @Date: 2020-11-15 23:34:47
 * @LastEditTime: 2020-11-16 11:54:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git_proj\malloc\mm.h
 */
#ifndef _MM
#define _MM
int mm_init(void); 
void *mm_malloc(size_t size); 
void mm_free(void *bp);
#endif