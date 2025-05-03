#include "DMusic.h"

DMusic::DMusic(const fs::path& filename)
{
	this->_filepath = filename;
}

void DMusic::checkHeader()
{
	char header[16];
	auto file = std::ifstream(this->_filepath, std::ios::binary);
	file.read(header, 16);
	if (strncmp(header, this->_ncm_header, 9) == 0)
	{
		this->_type = DMusicType::NCM;
	}
}
