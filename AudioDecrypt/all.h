#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include "NCMDecryptor.h"
#include "KGMADecoder.h"

void DecodeFactory(const filesystem::path& filename, filesystem::path outputpath = *new filesystem::path(), bool skip = false)
{
	{
		if (filename.extension().string() == ".ncm")
		{
			ncm::Decrypt(filename, outputpath, skip);
		}
		else if (filename.extension().string() == ".kgm" or filename.extension().string() == ".kgma")
		{
			kgma::Decrypt(filename, outputpath, skip);
		}
	}
}