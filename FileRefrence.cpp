
#include "stdafx.h"

#include "FileRefrence.h"
#include "MappedFile.h"
#include "FileTimer.h"

#include <string>
#include<boost/unordered_map.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/atomic.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;


typedef struct fileRefrenceStructExtend
{
	void * file;
	void * timer;
	void * list;
	bool complete;
	boost::atomic_int refCount;
	fileRefrenceStructExtend * next;
} fileRefrenceStructExtend;

typedef struct fileRefrenceListStruct
{
	unordered_map<string, fileRefrenceStructExtend *> * maps;
	shared_mutex  * sharedMutex;
} fileRefrenceListStruct;

// 可以使用得 文件内存 申请后得复用

static fileRefrenceStructExtend * canUsedFileRefList = nullptr;
static shared_mutex  * canUsedMutex = new shared_mutex();


void * createFileRefrenceList()
{
	fileRefrenceListStruct * list;
	list = new fileRefrenceListStruct();
	list->maps = new unordered_map<string, fileRefrenceStructExtend *>();
	list->sharedMutex = new shared_mutex();
	return list;
}

void freeFileRefrenceList(void * list)
{
	fileRefrenceListStruct * fileList = (fileRefrenceListStruct *)list;
	fileRefrenceStructExtend *fileRef = nullptr;
	fileList->sharedMutex->lock();
	for (unordered_map<string, fileRefrenceStructExtend *>::iterator iter = fileList->maps->begin(); iter != fileList->maps->end(); iter++)
	{
		fileRef = (fileRefrenceStructExtend *)(iter->second);
		closeMappedFile(fileRef->file);
		if (!(fileRef->complete))
			filesystem::remove(getFilePath(fileRef));
		freeFileRefrence(fileRef);
	}
	fileList->sharedMutex->unlock();
	delete fileList->maps;
	delete fileList->sharedMutex;
	delete	fileList;
	list = nullptr;
}

void * findFileRefrence(const char * filePath, void* list)
{
	fileRefrenceListStruct * fileList = (fileRefrenceListStruct *)list;
	if (fileList->maps->find(filePath) != fileList->maps->end())
	{
		return fileList->maps->at(filePath);
	}
	return nullptr;
}

void * addFileRefrence(char * filePath, void* list)
{
	void * file = nullptr;
	fileRefrenceListStruct * fileList = (fileRefrenceListStruct *)list;
	fileList->sharedMutex->lock();
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)findFileRefrence(filePath, list);
	fileList->sharedMutex->unlock();
	if (fileRef!=nullptr)
	{
		fileRef->refCount++;
		
	}	
	else 
	{
		void * mapFile = createMappedFile(filePath);
		if (mapFile != nullptr)
		{
			fileRef = new fileRefrenceStructExtend();
			fileRef->file = mapFile;
			fileRef->list = list;
			fileRef->refCount = 1;
			fileList->sharedMutex->lock();
			fileList->maps->insert(make_pair(filePath, fileRef));
			fileList->sharedMutex->unlock();
		}
	}
	
	return	fileRef;
}

void removeFileRefrence(void * file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	fileRef->refCount--;
}

int getFileRefCount(void * file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	return	fileRef->refCount;
}

void deleteFileRefrence(void * file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	fileRefrenceListStruct * fileList = (fileRefrenceListStruct *)(fileRef->list);
	const char * filePath = getMappedFilePath(fileRef->file);
	fileList->sharedMutex->lock();
	fileList->maps->erase(filePath);
	fileList->sharedMutex->unlock();
}

void closeFileRefrence(void * file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	closeMappedFile(fileRef->file);
}

void freeFileRefrence(void * file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	freeMappedFile(fileRef->file);
	delete fileRef;
}

const char * getFilePath(void* file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	return getMappedFilePath(fileRef->file);
}

size_t getFileSize(void* file)
{
	fileRefrenceStructExtend * fileRef = (fileRefrenceStructExtend *)file;
	return getMappedFileSize(fileRef->file);
}

size_t fileRefReadRang(void * file, size_t pos, size_t count, void ** out)
{
	return mappedFileReadRang(((fileRefrenceStructExtend *)file)->file, pos, count, out);
}

size_t fileRefWriteRang(void * file, size_t pos, size_t size, void * in)
{
	return mappedFileWriteRang(((fileRefrenceStructExtend *)file)->file, pos, size, in);
}
