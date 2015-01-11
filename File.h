#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <cstdio>
#include <cstring>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_PATH 255
using namespace std;

class Dir;
class Root;

class FileDir{
    friend class Dir;
    friend class Root;
	protected:
		char* name;
		FileDir *next;

	    FileDir();
	    virtual ~FileDir();
	    virtual FileDir* find(regex& exp);
    public:
        virtual FileDir* find(const char *exp) = 0;
};

class File: public FileDir{
    friend class Dir;
	protected:
	    File(const char *name_);
	    virtual ~File() {}
    public:
        virtual FileDir* find(const char *exp);
};

class Dir: public FileDir{
    protected:
		FileDir *first;

		Dir(const char *dirName);
		virtual ~Dir();
        virtual FileDir* find(regex& exp);
    public:
        virtual FileDir* find(const char *exp);
};

class Root: public Dir{
    private:
        char *path;
    public:
        Root(const char* dirName);
        ~Root();

        FileDir* find(const char *exp);
};

#endif
