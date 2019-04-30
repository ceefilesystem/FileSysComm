#include "stdafx.h"

#include "FileCommon.h"

#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>

bool isFileExistent(const char * filePath) 
{
	boost::system::error_code error;
	auto file_status = boost::filesystem::status(filePath, error);
	if (error) {
		return false;
	}

	if (!boost::filesystem::exists(file_status)) {
		return false;
	}

	if (boost::filesystem::is_directory(file_status)) {
		return false;
	}

	return true;
}