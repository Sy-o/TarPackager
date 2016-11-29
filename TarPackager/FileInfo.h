#pragma once
#include <vector>
#include <string>
#include "FileFragment.h"

struct FileInfo
{
    FileInfo() :TotalSize(0) {};

    FileInfo(const FileInfo& a);
    FileInfo& operator=(const FileInfo& a);

    friend std::ostream& operator<<(std::ostream& os, const FileInfo& m);
    friend std::istream& operator>>(std::istream& is, FileInfo& m);

    std::vector<FileFragment> Fragments;
    std::string Name;
    unsigned __int64 TotalSize;
};