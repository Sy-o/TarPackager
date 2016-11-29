// TarPackager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "options.h"
#include "TarPackager.h"

using namespace std;

void usage()
{
    cout << "Usage: tar [options] packageName [path path ...]" << endl
        << "options:" << endl;
    options::print_usage(cerr);
}

bool isFileExists(const string& name)
{
    wstring ws(name.begin(), name.end());
    return GetFileAttributes(ws.c_str()) != INVALID_FILE_ATTRIBUTES;
}

int main(int argc, char* argv[])
{
    try
    {
        int end; // End of options.
        options o(argc, argv, end);
        if (o.help() || argc == 1)
        {
            usage();
            return 0;
        }
        if (end == argc)
        {
            cerr << "no package file name!" << endl;
            usage();
            return 1;
        }
        if ((o.list() || o.get() || o.del()) && !isFileExists(argv[end]))
        {
            cerr << "no package file found!" << endl;
            return 1;
        }

        if (!o.list() && end == argc - 1) //if specified only package name
        {
            cerr << "no file(s) specified, nothing to do." << endl;
            usage();
            return 1;
        }

        TarPackager packager(argv[end]);

        if (o.list())
        {
            packager.List();
            return 0;
        }        
    
        vector<string> files;
        for (int i = end+1; i < argc; i++)
        {
            files.push_back(argv[i]);
        }

        if (o.add())
        {
            packager.AddFiles(files);
        }
        else if (o.del())
        {
            packager.DelFiles(files);
        }
        else if (o.get())
        {
            packager.GetFile(files[0], files.size() > 1 ? files[1] : "");
        }               
    }
    catch (const cli::exception& e)
    {
        cerr << e << endl;
        usage();
        return 1;
    }
	return 0;
}



