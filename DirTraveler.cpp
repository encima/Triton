#include "DirTraveler.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>

using namespace std;

DirTraveler::DirTraveler()
{
        //ctor
}

DirTraveler::~DirTraveler()
{
        //dtor
}

vector<string> DirTraveler::travelDirectory(string directory)
{
        // travel thru a directory gathering all the file and directory naems
    vector<string> fileList;
    DIR *dir;
    struct dirent *ent;
        // open a directory
    if ((dir=opendir(directory.c_str())) != NULL)
    {
        while((ent=readdir(dir)) != NULL) // loop until the directory is traveled thru
        {
                // push directory or filename to the list
            fileList.push_back(ent->d_name);
        }
            // close up
        closedir(dir);
    }
        //return the filelust
    return fileList;
}

void DirTraveler::travelDirectoryRecursive(string directory, vector<string> *fullList)
{
        // get the "root" directory's directories
    vector<string> fileList = travelDirectory(directory);
        // loop thru the list
    for (vector<string>::iterator i = fileList.begin(); i!=fileList.end(); ++i)
    {
            // test for . and .. directories (this and back)
        if (strcmp((*i).c_str(), ".") &&
            strcmp((*i).c_str(), ".."))
        {
                // i use stringstream here, not string = foo; string.append(bar);
            stringstream fullname;
            fullname << directory << "/" << (*i);
            fullList->push_back(fullname.str());
            travelDirectoryRecursive(fullname.str(), fullList);
        }
    }
}