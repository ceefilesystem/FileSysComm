#pragma once
class FileBase
{
public:
	FileBase();
	virtual ~FileBase();

	virtual int readFile(const char* filename, int pos, int size, void* in) = 0;
	virtual int writeFile(const char* filename, int pos, int size, void* out) = 0;


	//void upLoadByFilename();
};

