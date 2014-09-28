#include "stdafx.h"
#include <Windows.h>
#include <string>
#include "FileIO.h"
#include <fstream>


void FileIO::write(std::string file, char* data, int len, bool overwrite){
	using namespace std;
	ofstream myFile;
	if(overwrite){
			myFile.open(file, ofstream::out | ofstream::binary);
	}
	else{
	myFile.open(file, ofstream::out | ofstream::binary | ofstream::app);
	}
	myFile.write(data,len);
	myFile.flush();
	myFile.close();
}
void FileIO::read(std::string file, char* data, int len, int pt){
	using namespace std;
	ifstream myFile (file, ifstream::in | ifstream::binary);
	myFile.seekg(pt*len);
	myFile.read(data, len);
	myFile.close();
}
