#include "stdafx.h"
#include "FileFragment.h"

std::ostream& operator<<(std::ostream& os, const FileFragment& m)
{
    os.write((char*)&m.Size, sizeof(m.Size));
    os.write((char*)&m.Pos, sizeof(m.Pos));
    return os;
}

std::istream& operator>>(std::istream& is, FileFragment& m)
{
    is.read((char*)&m.Size, sizeof(m.Size));
    is.read((char*)&m.Pos, sizeof(m.Pos));
    return is;
}