#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <cstdio>
#include <cstring>
#include <regex>
#include <string>
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
	protected: //to be changed
		char* name;
		FileDir *parent, *next;

	    FileDir();
	    virtual FileDir* find(regex& exp);
	    void setName(char *name_) {name=name_;}
    public:
        virtual FileDir* find(const char *exp) = 0;
	    virtual ~FileDir();
	    virtual void getStructure(string &strct) = 0;

	    static FileDir* createTree(char *strct);
	    virtual void create(string path);
	    virtual string getPath();
};

class File: public FileDir{
    friend class Dir;
    friend class FileDir;
	protected:
	    File(): FileDir() {}
	    File(const char *name_);
	    virtual ~File() {}
    public:
        virtual FileDir* find(const char *exp);
	    virtual void getStructure(string &strct);
};

class Dir: public FileDir{
    friend class FileDir;
    protected:
		FileDir *first;

        Dir(): FileDir(), first(0) {}
		Dir(const char *dirName);
		virtual ~Dir();
        virtual FileDir* find(regex& exp);
    public:
        virtual FileDir* find(const char *exp);
	    virtual void getStructure(string &strct);
	    virtual void create(string path);
	    virtual string getPath();
};

class Root: public Dir{
    private:
        char *path;
    protected:
        Root(): Dir() {}
    public:
        Root(const char* dirName);
        ~Root();

        FileDir* find(const char *exp);
	    virtual string getPath();
};

#endif
