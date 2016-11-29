#include "stdafx.h"
#include "FileInfo.h"

FileInfo::FileInfo(const FileInfo& a)
{
    Name = a.Name;
    TotalSize = a.TotalSize;
    Fragments = a.Fragments;
}

FileInfo& FileInfo::operator = (const FileInfo& a)
{
    Name = a.Name;
    TotalSize = a.TotalSize;
    Fragments = a.Fragments;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const FileInfo& m)
{
    os.write((char*)&m.TotalSize, sizeof(m.TotalSize));
    size_t len = m.Name.size();
    os.write((char*)&len, sizeof(len));
    os.write(m.Name.c_str(), len);
    len = m.Fragments.size();
    os.write((char*)&len, sizeof(len));
    for (auto& f : m.Fragments)
        os << f;
    return os;
}

std::istream& operator>>(std::istream& is, FileInfo& m)
{
    size_t len = 0;
    is.read((char*)&m.TotalSize, sizeof(m.TotalSize));
    is.read((char*)&len, sizeof(len));

    char * buff = new char[len + 1];
    buff[len] = 0;
    is.read(buff, len);
    m.Name = buff;
    delete[] buff;

    len = 0;
    is.read((char*)&len, sizeof(len));
    m.Fragments.clear();
    for (size_t i = 0; i < len; i++)
    {
        FileFragment f;
        is >> f;
        m.Fragments.push_back(f);
    }        
    return is;
}