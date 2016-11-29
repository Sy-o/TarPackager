#pragma once
#include <fstream>
#include <vector>
#include <string>
#include "FileController.h"

class TarPackager
{
public:
	TarPackager(){};
	TarPackager(const std::string& filePath);
	~TarPackager();

	void Open(std::string filePath);
	void Close();
	bool IsOpened() const;

	void AddFile(std::string filePath);
	void DelFile(std::string fileName);
    void AddFiles(std::vector<std::string> files);
    void DelFiles(std::vector<std::string> files);
	void GetFile(std::string fileName, std::string pathToSave = "");
	void List() const;

private:
	void LoadPackageInfo();
	void SavePackageInfo();
	void TruncatePackage(unsigned long long size);

	void AddFragment(std::fstream& source, FileFragment fragment);
	void SaveFragment(std::fstream& dest, FileFragment fragment);

private:
	std::string FileName;
	FileController fileController;
	std::fstream Tar;
};