#ifndef FILE_TIMER_H
#define FILE_TIMER_H

#include <stdint.h>

// 回调 返回 0 释放 返回 1不释放
typedef void(*timer_execute_func)(void *data);

/* 新建一个超时回调 */
void *file_timeout(void *data, int time, timer_execute_func cb);

/* 重新设置超时回调 */
void file_updatetime(void * time);

/* 更新时间轮盘时间 */
void file_updatetimer(void);

/* 获取时间轮盘开始时间 */
uint32_t file_starttime(void);

/* 获取时间 */
uint64_t file_thread_time(void);	//milli second

/* 初始化时间轮盘 */
void file_timer_init(void);

/* 开始运行时间轮盘 */
void file_timer_run(void);

#endif