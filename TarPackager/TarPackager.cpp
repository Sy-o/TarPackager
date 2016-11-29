#include "stdafx.h"
#include <windows.h>
#include "TarPackager.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <stdio.h>
using namespace std;

#define uint64 unsigned __int64
const uint64 DATA_START_POS = sizeof(uint64);

TarPackager::TarPackager(const std::string& filePath)
{
	Open(filePath);
}

TarPackager::~TarPackager()
{
	try
	{
		if (!Tar.is_open()) return;
		Close();
	}
	catch (...)
	{
		cerr << "Error was occurred while saving the package!" << endl;
	}
}

void TarPackager::Open(std::string filePath)
{
	if (Tar.is_open()) Close();
	
	Tar.open(filePath, ios::binary | ios::out | ios::app);
	if (Tar.fail())
	{
		cerr << "Error: " << strerror(errno) << endl;
		return;
	}
	Tar.close();
	Tar.open(filePath, ios::binary | ios::in | ios::out);

	FileName = filePath;
	LoadPackageInfo();
}

void TarPackager::Close()
{
	if (!Tar.is_open()) return;
	SavePackageInfo();
	Tar.close();
	fileController.Clear();
	FileName.clear();
}

bool TarPackager::IsOpened() const
{
	return Tar.is_open();
}

void TarPackager::AddFile(std::string filePath)
{
	if (!Tar.is_open())
	{
		cerr << "Error: No package file opened!" << endl;
		return;
	}
	
	string name(filePath);
	size_t pos = name.find_last_of('\\');
	if (pos != string::npos) name.erase(0, pos + 1);	

	if (fileController.FindFile(name))
	{
		cout << "File '" << name << "' has already packed!" << endl;
		return;
	}

	fstream f(filePath, ios::binary | ios::in | ios::out);
	if (f.fail())
	{
		cerr << "Error: " << strerror(errno) << endl;
		return;
	}

	uint64 newFileSize = 0;
	f.seekp(0, ios::end);
	newFileSize = f.tellp();
	f.seekp(0, ios::beg);

	FileInfo info;
	fileController.AddFile(name, newFileSize, info);
	
	for (auto& fragment : info.Fragments)
	{
		AddFragment(f, fragment);
	}
	f.close();
	cout << "File '" << name << "' is added" << endl;
}

void TarPackager::DelFile(std::string fileName)
{
	if (!Tar.is_open())
	{
		cerr << "Error: No package file opened!" << endl;
		return;
	}
	if (!fileController.FindFile(fileName))
	{
		cerr << "Error: No file '" << fileName << "' was found in the package!" << endl;
		return;
	}

	uint64 lastPosPrev = fileController.GetLastDataPos();
	fileController.DelFile(fileName);
	uint64 lastPosCur = fileController.GetLastDataPos();

	if (lastPosPrev != lastPosCur)
	{
		TruncatePackage(lastPosCur);
	}
	cout << "File '" << fileName << "' is deleted" << endl;
}

void TarPackager::AddFiles(std::vector<std::string> files)
{
    for (auto& f : files)
    {
        AddFile(f);
    }
}

void TarPackager::DelFiles(std::vector<std::string> files)
{
    for (auto& f : files)
    {
        DelFile(f);
    }
}

void TarPackager::GetFile(std::string fileName, std::string pathToSave)
{
	if (!Tar.is_open())
	{
		cerr << "Error: No package file opened!" << endl;
		return;
	}
	if (!fileController.FindFile(fileName))
	{
        cerr << "Error: No file '" << fileName << "' was found in the package!" << endl;
		return;
	}
	if (pathToSave.empty()) pathToSave = fileName;

	fstream f(pathToSave, ios::binary | ios::out | ios::in | ios::trunc);
	if (f.fail())
	{
		cerr << "Can't save file " << pathToSave << ". Error: " << strerror(errno) << endl;
		return;
	}

	FileInfo info;
	fileController.GetFile(fileName, info);
	for (auto& fragment : info.Fragments)
	{
		SaveFragment(f, fragment);
	}

	cout << "File '" << fileName << "' saved to '" << pathToSave << "'" << endl;
	f.close();
}

void TarPackager::List() const
{
	cout << setw(40) << left << "File Name" << " File Size" << endl;
	cout << string(60, '=') << endl;
	for (auto& i : fileController.GetFiles())
	{
		if (i.Name.empty()) continue;
		cout << setw(40) << left << i.Name << " " << i.TotalSize << " bytes" << endl;
	}
}

void TarPackager::LoadPackageInfo()
{
	fileController.Clear();
	Tar.seekp(0, ios::end);
	if (Tar.tellp())
	{
		Tar.seekp(0, ios::beg);
		uint64 pos = 0;
		Tar.read((char*)&pos, sizeof(pos));
		Tar.seekp(pos, ios::beg);
		Tar >> fileController;
	}
}

void TarPackager::SavePackageInfo()
{
	if (!fileController.Empty())
	{
		uint64 pos = fileController.GetLastDataPos();
		Tar.seekp(0, ios::beg);
		Tar.write((char*)&pos, sizeof(pos));
		Tar.seekp(pos, ios::beg);
		Tar << fileController;
	}
}

void TarPackager::TruncatePackage(uint64 size)
{
	Tar.close();
    if (size <= DATA_START_POS) size = 0;
	
    HANDLE hFile = CreateFileW(wstring(FileName.begin(), FileName.end()).c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    _LARGE_INTEGER newSize;
    newSize.QuadPart = size;
    if (!SetFilePointerEx(hFile, newSize, NULL, FILE_BEGIN) || !SetEndOfFile(hFile))
    {
        cerr << "Error: can't truncate file size!" << endl;
    }
    CloseHandle(hFile);
	
	Tar.open(FileName, ios::binary | ios::in | ios::out);
}

void TarPackager::AddFragment(fstream& source, FileFragment fragment)
{
	Tar.seekp(fragment.Pos, ios::beg);
	
	const int bufferSize = 2048;
	char buff[bufferSize];

	uint64 copied = 0;
	for (copied; (copied + bufferSize) < fragment.Size; copied += bufferSize)
	{
		source.read(buff, bufferSize);
		Tar.write(buff, bufferSize);
	}
	if (copied < fragment.Size)
	{
		source.read(buff, fragment.Size - copied);
		Tar.write(buff, fragment.Size - copied);
	}
}

void TarPackager::SaveFragment(fstream& dest, FileFragment fragment)
{
	Tar.seekp(fragment.Pos, ios::beg);

	const int bufferSize = 2048;
	char buff[bufferSize];

	uint64 copied = 0;
	for (copied; (copied + bufferSize) < fragment.Size; copied += bufferSize)
	{
		Tar.read(buff, bufferSize);
		dest.write(buff, bufferSize);
	}
	if (copied < fragment.Size)
	{
		Tar.read(buff, fragment.Size - copied);
		dest.write(buff, fragment.Size - copied);
	}
}