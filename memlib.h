/*
 * @Author: your name
 * @Date: 2020-11-15 23:37:57
 * @LastEditTime: 2020-11-16 11:53:38
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git_proj\malloc\memlib.h
 */
#ifndef _MEMLIB
#define _MEMLIB
void mem_init(void);  
void *mem_sbrk(int incr);
#endif