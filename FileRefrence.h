#ifndef _FILE_FRFRENCE_H_
#define _FILE_FRFRENCE_H_

#include <stdint.h>

/*
	ͨ�� refCount ����ʶ�ж��ٶ��������ļ�
*/
typedef struct fileRefrenceStruct
{
	void * file;
	void * timer;
	void * list;
	bool complete;
} fileRefrenceStruct;

/*
	�ļ����á��ļ��б����
*/
// �����б�
void * createFileRefrenceList();
// �ͷ��б�
void freeFileRefrenceList(void * list);

// ����list��file������
void * addFileRefrence(char * filePath, void* list);
// �黹list��file������
void removeFileRefrence(void * file);
// ���list��file������
void deleteFileRefrence(void * file);
// �ر�file�ڴ�
void closeFileRefrence(void * file);
// �ͷ�file���ڴ�
void freeFileRefrence(void * file);
// ��ȡ�ļ�·��
const char * getFilePath(void* file);
// ��ȡ�ļ���С
size_t getFileSize(void* file);
// ��ȡ����
int getFileRefCount(void * file);

size_t fileRefReadRang(void * file, size_t pos, size_t count, void ** out);
size_t fileRefWriteRang(void * file, size_t pos, size_t size, void * in);

#endif
