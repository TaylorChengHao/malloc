/*
 * @Author: your name
 * @Date: 2020-11-15 15:14:45
 * @LastEditTime: 2020-11-16 11:53:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git_proj\malloc\memlib.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "memlib.h"

#define MAX_HEAP (20*(1<<20))  // 20 MB

void *Malloc(size_t size);
void unix_error(char *msg);

// memlib.c提供一个内存系统模型，使我们能在不干涉系统malloc的情况下，运行分配器。
// 在mem_heap和mem_brk之间的字节表示已分配的虚拟内存
static char *mem_heap;                  //指向堆的第一个byte
static char *mem_brk;                   //指向堆的最后一个byte地址加一
static char *mem_max_addr;              //堆最大地址加1

// 可用的虚拟内存模型化为一个大的、双字对齐的字节数组
void mem_init(void)
{
    mem_heap = (char *)Malloc(MAX_HEAP);
    mem_brk = (char *)mem_heap;
    mem_max_addr = (char *)(mem_heap + MAX_HEAP);
}

// 当堆不够用时来扩展堆的大小
void *mem_sbrk(int incr)
{
    char *old_brk = mem_brk;

    if ((incr < 0) || (mem_brk + incr) > mem_max_addr)
    {
        errno = ENOMEM;
        fprintf(stderr, "ERROR:mem_sbrk failed.Ran out of memory\n");
        return (void *)(-1);
    }
    mem_brk += incr;
    return (void *)old_brk;
}

// 对malloc的包装
void *Malloc(size_t size) 
{
    void *p;

    if ((p  = malloc(size)) == NULL)
	unix_error("Malloc error");
    return p;
}

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}