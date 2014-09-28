#pragma once

class FileIO{
public:

	//writes the buffer to an output file.
	void write(std::string file, char* buffer, int len, bool overwrite);
	//reads the output file and saves to buffer.
	void read(std::string file, char* buffer, int len, int pt);
	
};