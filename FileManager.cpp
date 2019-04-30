#include "stdafx.h"

#include "FileCommon.h"
#include "FileManager.h"
#include "FileCommon.h"


#include "MappedFile.h"
#include "FileRefrence.h"

#include "FileTimer.h"
#include <boost/filesystem.hpp>
#include <boost/thread/shared_mutex.hpp>

using namespace boost;

// 时间轮盘 滴答 1/100 秒  一次  10ms 一次

// 物理文件 定时器超时时间 一天
#define CONST_FILE_DELAY_MS 200// 100 * 60 * 60 *24
// 映射文件 定时器超时时间 一小时
#define CONST_MAPPED_DELAY_MS  10000//100 * 60 * 60
// 解除引用之后 释放内存时间 
#define CONST_FREE_DELAY_MS 2000

/*
	文件读写业务上是分离的，不会存在一个文件即在读也在写
*/

// 正在读的文件列表
static void * writeFileList;
// 正在写的文件列表
static void * readFileList;

// 文件更新定时器
static void updateFileTimer(void * file, uint32_t delay, timer_execute_func cb)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)file;
	if (fileRef->timer == nullptr)
	{
		fileRef->timer = file_timeout(fileRef, delay, cb);
	}
	// file_updatetime(fileRef->timer);
}

static void onTimeCallback(void *file);
static void onTimeNocompleteCallback(void *file);
static void onTimeFreeCallback(void *file);

// 释放内存超时
static void onTimeFreeCallback(void *file)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)file;
	fileRef->timer = nullptr;

	// 释放内存	
	closeFileRefrence(file);
	
	// 删除文件
	if (!fileRef->complete)
		boost::filesystem::remove(getFilePath(fileRef));
	// 释放内存
	freeFileRefrence(file);
}

// 未上传完成文件超时
static void onTimeNocompleteCallback(void *file)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)file;
	fileRef->timer = nullptr;
	
	if (getFileRefCount(file) == 0)
	{
		deleteFileRefrence(fileRef);
		updateFileTimer(file, CONST_FREE_DELAY_MS, onTimeFreeCallback);
	}
	else {
		updateFileTimer(file, CONST_MAPPED_DELAY_MS, onTimeCallback);
	}
	
}

// 映射超时
static void onTimeCallback(void *file)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)file;
	fileRef->timer = nullptr;
	if (getFileRefCount(file) == 0){
		if (!fileRef->complete)
		{
			updateFileTimer(file, CONST_FILE_DELAY_MS, onTimeNocompleteCallback);
		}
		else
		{
			deleteFileRefrence(fileRef);
			updateFileTimer(file, CONST_FREE_DELAY_MS, onTimeFreeCallback);
		}
		
	}
	else {
		updateFileTimer(file, CONST_MAPPED_DELAY_MS, onTimeCallback);
	}
}

int getVersion()
{
	return 1;
}

void initFileManager()
{
	file_timer_init();
	file_timer_run();
	writeFileList = createFileRefrenceList();
	readFileList = createFileRefrenceList();
}

void * getWriteFile(char * filePath, size_t size)
{
	if (isFileExistent(filePath))
	{
		/*
			如果文件已经存在 大小不一样重新设置文件大小
		*/
		if ((boost::filesystem::file_size(filePath) != size) && (size!=0))
			boost::filesystem::resize_file(filePath, size);
	}
	else
	{
		/*
			如果文件不存在 不存在新建
		*/
		std::filebuf fbuf;
		fbuf.open(filePath, std::ios_base::in | std::ios_base::out
			| std::ios_base::trunc | std::ios_base::binary);
		fbuf.pubseekoff(size - 1, std::ios_base::beg);
		fbuf.sputc(0);
		fbuf.close();
	}

	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)addFileRefrence(filePath, writeFileList);
	if (fileRef != nullptr)
		fileRef->complete = false;
	return fileRef;

}


void * getReadFile(char * filePath)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)addFileRefrence(filePath, readFileList);

	if (fileRef != nullptr)
	{
		updateFileTimer(fileRef, CONST_MAPPED_DELAY_MS, onTimeCallback);
		fileRef->complete = true;
	}
	return fileRef;
}


void returnFile(void * file)
{
	updateFileTimer(file, CONST_MAPPED_DELAY_MS, onTimeCallback);
	removeFileRefrence(file);
}

void completeFile(void * file)
{
	fileRefrenceStruct * fileRef = (fileRefrenceStruct *)file;
	fileRef->complete = true;
	returnFile(file);
}


size_t fileReadRang(void * file, size_t pos, size_t count, void ** out)
{
	updateFileTimer(file, CONST_MAPPED_DELAY_MS, onTimeCallback);
	return fileRefReadRang(file, pos, count, out);
	
}

size_t fileWriteRang(void * file, size_t pos, size_t size, void * in)
{
	updateFileTimer(file, CONST_MAPPED_DELAY_MS, onTimeCallback);
	return fileRefWriteRang(file, pos, size, in);
}


size_t fileSize(void * file)
{
	return getFileSize(file);
}




