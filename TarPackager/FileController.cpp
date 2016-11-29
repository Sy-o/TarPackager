#include "stdafx.h"
#include "FileController.h"
#include <algorithm>
using namespace std;

const unsigned __int64 START_DATA_POS = sizeof(unsigned __int64);

FileController::FileController(const FileController& a)
{
	Files = a.Files;
	Spaces = a.Spaces;
}

FileController& FileController::operator = (const FileController& a)
{
	Files = a.Files;
	Spaces = a.Spaces;
	return *this;
}

std::ostream& operator<<(std::ostream& os, const FileController& m)
{
    size_t len = m.Files.size();
    os.write((char*)&len, sizeof(len));
    for (auto& f : m.Files)
        os << f;
    len = m.Spaces.size();
    os.write((char*)&len, sizeof(len));
    for (auto& s : m.Spaces)
        os << s;
    return os;
}

std::istream& operator>>(std::istream& is, FileController& m)
{
    m.Files.clear();
    m.Spaces.clear();
    size_t len = 0;
    is.read((char*)&len, sizeof(len));
    for (size_t i = 0; i < len; i++)
    {
        FileInfo f;
        is >> f;
        m.Files.push_back(f);
    }

    len = 0;
    is.read((char*)&len, sizeof(len));
    for (size_t i = 0; i < len; i++)
    {
        FileFragment s;
        is >> s;
        m.Spaces.push_back(s);
    }    
    return is;
}

void FileController::Clear()
{
	Files.clear();
	Spaces.clear();
}

bool FileController::Empty() const
{
	return Files.empty();
}

void FileController::AddFile(std::string name, unsigned __int64 size, FileInfo& newFile)
{
	if (FindFile(name)) 
		return;

	newFile.Fragments.clear();
	newFile.Name = name;
	newFile.TotalSize = 0;
	for (auto& s : Spaces)
	{
		if (newFile.TotalSize + s.Size <= size)
		{
			newFile.Fragments.push_back(s);
			newFile.TotalSize += s.Size;
			s.Size = 0;
		}
		else
		{
			unsigned __int64 partSize = size - newFile.TotalSize;
			newFile.Fragments.push_back(FileFragment(s.Pos, partSize));
			newFile.TotalSize += partSize;
			s.Size -= partSize;
			s.Pos += partSize;
		}
		if (newFile.TotalSize == size) break;
	}
	if (newFile.TotalSize < size)
	{
		newFile.Fragments.push_back(FileFragment(GetLastDataPos(), size - newFile.TotalSize));
		newFile.TotalSize += size - newFile.TotalSize;
	}

	sort(newFile.Fragments.begin(), newFile.Fragments.end(), FragmentComp());
	Files.push_back(newFile);

	vector<FileFragment>::iterator it;
	while ((it = find_if(Spaces.begin(), Spaces.end(), [](const FileFragment& f){ return f.Size == 0; })) != Spaces.end())
	{
		Spaces.erase(it);
	}
}

void FileController::DelFile(std::string name)
{
    auto it = find_if(Files.begin(), Files.end(), [&name](FileInfo& f) {return f.Name == name; });
    if (it != Files.end())
    {
		auto fragments = it->Fragments;
		Files.erase(it);
		AddFreeFragments(fragments);
    }
}

bool FileController::FindFile(std::string name) const
{
    return find_if(Files.begin(), Files.end(), [&name](const FileInfo& f) {return f.Name == name; }) != Files.end();
}

bool FileController::GetFile(std::string name, FileInfo& file) const
{
    auto it = find_if(Files.begin(), Files.end(), [&name](const FileInfo& f) {return f.Name == name; });
    if (it != Files.end())
    {
        file = *it;
        return true;
    }
    return false;
}

unsigned __int64 FileController::GetLastDataPos() const
{
    unsigned __int64 lastPos = START_DATA_POS;
    for (auto& f : Files)
    {
        for (auto& fr : f.Fragments)
        {
            if (fr.Pos + fr.Size > lastPos)
                lastPos = fr.Pos + fr.Size;
        }
    }
    return lastPos;
}

void FileController::AddFreeFragments(std::vector<FileFragment>& fragments)
{
	Spaces.insert(Spaces.end(), fragments.begin(), fragments.end());
	if (Spaces.empty()) return;

	sort(Spaces.begin(), Spaces.end(), FragmentComp());
	//Join spaces
	vector<FileFragment> newSpaces;
	FileFragment current = Spaces[0];
	for (int i = 1; (size_t)i < Spaces.size(); i++)
	{
		if (current.Pos + current.Size == Spaces[i].Pos) //neighbors
		{
			current.Size += Spaces[i].Size;
		}
		else
		{
			newSpaces.push_back(current);
			current = Spaces[i];
		}
	}
	if (current.Pos < GetLastDataPos())
		newSpaces.push_back(current);

	Spaces = newSpaces;
}

const std::vector<FileInfo>& FileController::GetFiles() const
{
	return Files;
}