#ifndef FILE_TIMER_H
#define FILE_TIMER_H

#include <stdint.h>

// �ص� ���� 0 �ͷ� ���� 1���ͷ�
typedef void(*timer_execute_func)(void *data);

/* �½�һ����ʱ�ص� */
void *file_timeout(void *data, int time, timer_execute_func cb);

/* �������ó�ʱ�ص� */
void file_updatetime(void * time);

/* ����ʱ������ʱ�� */
void file_updatetimer(void);

/* ��ȡʱ�����̿�ʼʱ�� */
uint32_t file_starttime(void);

/* ��ȡʱ�� */
uint64_t file_thread_time(void);	//milli second

/* ��ʼ��ʱ������ */
void file_timer_init(void);

/* ��ʼ����ʱ������ */
void file_timer_run(void);

#endif