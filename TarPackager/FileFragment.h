#pragma once
#include <fstream>

struct FileFragment
{
    FileFragment() : Pos(0), Size(0) {};
    FileFragment(unsigned __int64 pos, unsigned __int64 size): Pos(pos), Size(size) {};

    friend std::ostream& operator<<(std::ostream& os, const FileFragment& m);
    friend std::istream& operator>>(std::istream& is, FileFragment& m);

    unsigned __int64 Pos;
    unsigned __int64 Size;
};

class FragmentComp
{
public:
	bool operator()(const FileFragment& a, const FileFragment& b)
	{
		return a.Pos < b.Pos;
	}
};