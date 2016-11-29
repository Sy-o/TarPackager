#pragma once
#include <vector>
#include "FileInfo.h"

class FileController
{
public:
	FileController(){};
	FileController(const FileController& a);
	FileController& operator=(const FileController& a);

	friend std::ostream& operator<<(std::ostream& os, const FileController& m);
	friend std::istream& operator>>(std::istream& is, FileController& m);

	void Clear();
	bool Empty() const;
	void AddFile(std::string name, unsigned __int64 size, FileInfo& file);
    void DelFile(std::string name);
    bool FindFile(std::string name) const;
    bool GetFile(std::string name, FileInfo& file) const;
    unsigned __int64 GetLastDataPos() const;

	const std::vector<FileInfo>& GetFiles() const;

private:
	void AddFreeFragments(std::vector<FileFragment>& fragments);

private:
    std::vector<FileInfo> Files;
    std::vector<FileFragment> Spaces;
};