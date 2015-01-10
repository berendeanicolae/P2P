#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_PATH 255

char fullPath[MAX_PATH];
struct stat st;

class Dir;

class FileDir{
    friend class Dir;
	protected:
		char* name;
		FileDir *next;

	    FileDir();
	    virtual ~FileDir();
};

class File: public FileDir{
    friend class Dir;
	protected:
	    File(const char *name_);
	    virtual ~File() {}
};

class Dir: public FileDir{
    protected:
		FileDir *first;

		Dir(const char *dirName);
		virtual ~Dir();
};

class Root: public Dir{
    private:
        char *path;
    public:
        Root(const char* dirName);
        ~Root();
};

#endif
