#ifndef _FILE_FRFRENCE_H_
#define _FILE_FRFRENCE_H_

#include <stdint.h>

/*
	通过 refCount 来标识有多少对象引用文件
*/
typedef struct fileRefrenceStruct
{
	void * file;
	void * timer;
	void * list;
	bool complete;
} fileRefrenceStruct;

/*
	文件引用、文件列表操作
*/
// 创建列表
void * createFileRefrenceList();
// 释放列表
void freeFileRefrenceList(void * list);

// 申请list中file的引用
void * addFileRefrence(char * filePath, void* list);
// 归还list中file的引用
void removeFileRefrence(void * file);
// 解除list中file的引用
void deleteFileRefrence(void * file);
// 关闭file内存
void closeFileRefrence(void * file);
// 释放file的内存
void freeFileRefrence(void * file);
// 获取文件路径
const char * getFilePath(void* file);
// 获取文件大小
size_t getFileSize(void* file);
// 获取引用
int getFileRefCount(void * file);

size_t fileRefReadRang(void * file, size_t pos, size_t count, void ** out);
size_t fileRefWriteRang(void * file, size_t pos, size_t size, void * in);

#endif
