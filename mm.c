/*
 * @Author: your name
 * @Date: 2020-11-14 22:36:21
 * @LastEditTime: 2020-11-16 12:32:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git_proj\malloc\mm.cpp
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memlib.h"
#include "mm.h"
// 基本的常数和宏，定义一小组宏来访问和遍历空闲链表很有帮助
#define WSIZE 4             //字的大小，也是堆块的头部和尾部的大小
#define DSIZE 8             //双字
#define CHUNKSIZE (1 << 12) //扩展堆时的默认大小

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// size表示堆块的大小，而alloc表示是否已经分配出去，因为堆是双字的
// 所以二进制的后三位必为0，可以利用这个空间放一个alloc标记，
// 表示这个堆块是否有被分配，PACK结构可以放在堆块的头部和尾部
#define PACK(size, alloc) ((size) | (alloc))

// 从地址p读写一个字的操作
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// 从地址p读取一定大小的已分配内存
#define GET_SIZE(p) (GET(p) & ~0x7)
// 从地址p读取是否分配了
#define GET_ALLOC(p) (GET(p) & 0x1)

// 给定一个块的指针bp（指向第一个有效载荷字节），计算头部(一个WSIZE大)和尾部（一个WSIZE大）的地址
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// 分别返回指向前面的块和后面的块的块指针，从这里可以看出来size的大小是包括了头尾的。
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

// 对一个堆块的指针
static char *heap_listp = 0; /* Pointer to first block */

// 分配这三个函数给应用程序
extern int mm_init(void);
extern void *mm_malloc(size_t size);
extern void mm_free(void *ptr);

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

int mm_init(void)
{
    // 从内存系统得到4个字，并初始化，创建一个空的空闲链表
    if ((heap_listp = (char *)mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    // heap_listp的4个WSIZE数据设置成了 0|0x1001（头部）|0x1001（尾部）||0x1（结尾块）
    PUT(heap_listp, 0);                            //写一个0
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); //放置好头部
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); //放置好尾部
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     //结尾块（大小为0，已使用）
    heap_listp += (2 * WSIZE);                     //

    // 将堆扩展CHUNKSIZE字节的空间
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

// extend_heap可以在堆初始化和堆空间不够的时候使用，向内核要更大的空间
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    // 始终分配偶数个WSIZE保证对齐
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = (char *)mem_sbrk(size)) == -1)
        return NULL;

    // 初始化空闲块的头尾
    PUT(HDRP(bp), PACK(size, 0));         //空闲块的头部
    PUT(FTRP(bp), PACK(size, 0));         //空闲块的尾部
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //新的结尾块（就是刚分配的数据的最后一个字）头部设为大小为0、已用

    // 如果前一个堆以一个空闲块结束的就合并
    return coalesce(bp);
}

void *mm_malloc(size_t size)
{
    size_t asize;      //调整后的块大小
    size_t extendsize; //如果堆不够需要扩展的大小
    char *bp;

    if (size == 0)
        return NULL;

    // 调整块大小
    if (size <= DSIZE)
        // 强制最小块大小是16字节，开销8字节，头尾部8字节
        asize = 2 * DSIZE;
    else
        // 加上开销字节，然后向上舍入到最接近的8的整数倍
        asize = DSIZE * (size + (DSIZE) + (DSIZE - 1) / DSIZE);

    // 寻找空闲的list来适配
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    // 找不到适配的。需要扩展堆
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    // 头尾部的标志位设0
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    // 检测合并
    coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { // 前后都被分配了
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    {                                          // 后面空闲
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); // 计算后面空闲块的大小
        PUT(HDRP(bp), PACK(size, 0));          // 标记合并后面的块
        // PUT(FTRP(bp), PACK(size, 0));            // 感觉这个地方不对
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0)); // 感觉这样才对
    }
    else if (!prev_alloc && next_alloc)
    { // 前面空闲
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    { // 前后都空闲
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

static void *find_fit(size_t asize)
{
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE))
    {
        // 设置成已使用
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        // 多余空间切成一个新的块
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        // 设置成已使用
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}