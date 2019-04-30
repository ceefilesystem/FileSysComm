#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
#if _MSC_VER
#ifndef EXPORT_FILESYS
#define FILESYS_API __declspec(dllimport)
#pragma comment(lib, "FileSysComm.lib")
#else
#define FILESYS_API __declspec(dllexport)
#endif 
#else
#define FILESYS_API extern
#endif 

/*
	映射文件类  映射文件到内存 提供读取操作
*/

//extern "C" FILESYS_API void* createMappedFile(const char * filePath);
//extern "C" FILESYS_API int mappedFileReadRang(void * mappedFile, int pos, int count, void * out);
//extern "C" FILESYS_API int mappedFileWriteRang(void * mappedFile, int pos, int size, void * in);
//extern "C" FILESYS_API void closeMappedFile(void * mappedFile);
//
//extern "C" FILESYS_API size_t getFileSize(void * mappedFile);
//extern "C" FILESYS_API const char* getFilePath(void * mappedFile);

// 初始化 文件管理
extern "C" FILESYS_API void initFileManager();

// 获取写file
extern "C" FILESYS_API void * getWriteFile(char * filePath, size_t size);


// 获取读file
extern "C" FILESYS_API void * getReadFile(char * filePath);

// 归还file对象
extern "C" FILESYS_API void returnFile(void * file);
// 文件传输完毕
extern "C" FILESYS_API void completeFile(void * file);

// 读数据
extern "C" FILESYS_API size_t fileReadRang(void * file, size_t pos, size_t count, void ** out);
// 写数据
extern "C" FILESYS_API size_t fileWriteRang(void * file, size_t pos, size_t size, void * in);

// 写数据
extern "C" FILESYS_API size_t fileSize(void * file);

#endif

