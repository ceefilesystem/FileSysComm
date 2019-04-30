#include "stdafx.h"
#include "MappedFile.h"
#include "FileCommon.h"

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;



typedef struct mappedFileStruct
{
	char* filePath;
	void * fileAddress;
	size_t fileSize;
	file_mapping *fileMapped;
	mapped_region *mappedRegion;
}mappedFileStruct;



void* createMappedFile(char * filePath)
{
	if (isFileExistent(filePath))
	{
		mappedFileStruct * file = new mappedFileStruct();
		file->fileMapped = new file_mapping(filePath, read_write);
		file->mappedRegion = new mapped_region(*file->fileMapped, read_write);
		file->filePath = (char *)malloc(strlen(filePath) + 1);
		memcpy(file->filePath, filePath, strlen(filePath));
		file->filePath[strlen(filePath)] = 0;
		file->fileAddress = file->mappedRegion->get_address();
		file->fileSize = file->mappedRegion->get_size();
		return file;
	}
	return nullptr;
}


size_t mappedFileReadRang(void * mappedFile, size_t pos, size_t count, void ** out)
{

	mappedFileStruct * file = (mappedFileStruct *)mappedFile;
	if (pos < 0)
		return -1;
	if (pos > file->fileSize)
		return -1;
	*out = (void **)((size_t)file->fileAddress + pos);
	if (count == 0)
		return file->fileSize - pos;
	else if (pos + count < file->fileSize)
		return count;
	else
		return file->fileSize - pos;
}

size_t mappedFileWriteRang(void * mappedFile, size_t pos, size_t size, void * in)
{
	mappedFileStruct* file = (mappedFileStruct*)mappedFile;
	if (file->fileAddress == nullptr || in == nullptr)
		return -1;
	if (pos < 0)
		return -1;
	if ((pos > file->fileSize) || (pos + size > file->fileSize))
		return -1;
	memcpy((void *)((size_t)file->fileAddress + pos), in, size);
	return size;
}

void closeMappedFile(void * mappedFile)
{
	mappedFileStruct* file = (mappedFileStruct*)mappedFile;
	delete file->mappedRegion;
	delete file->fileMapped;
}

void freeMappedFile(void * mappedFile)
{
	mappedFileStruct* file = (mappedFileStruct*)mappedFile;
	delete file->filePath;

	delete file;
	mappedFile = nullptr;
}

size_t getMappedFileSize(void * mappedFile)
{
	return ((mappedFileStruct*)mappedFile)->fileSize;
}

char* getMappedFilePath(void * mappedFile)
{
	return ((mappedFileStruct*)mappedFile)->filePath;
}