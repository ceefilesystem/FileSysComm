#ifndef _MAPPED_FILE_H_
#define _MAPPED_FILE_H_
/*
	ӳ���ļ���  ӳ���ļ����ڴ� �ṩ��д����
*/
 
void* createMappedFile(char * filePath);
size_t mappedFileReadRang(void * mappedFile, size_t pos, size_t count, void ** out);
size_t mappedFileWriteRang(void * mappedFile, size_t pos, size_t size, void * in);
void closeMappedFile(void * mappedFile);
void freeMappedFile(void * mappedFile);

size_t getMappedFileSize(void * mappedFile);
char* getMappedFilePath(void * mappedFile);

#endif