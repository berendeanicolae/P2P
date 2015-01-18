#include "File.h"
#include <regex>
using namespace std;

char fullPath[MAX_PATH];
struct stat st;

void baseName(const char *path, char **dest){
    const char *s, *e;
    int len;

    if (!path)
        return;

    len = strlen(path);
    for (e=path+len-1; e>path && *e=='/'; --e);
    for (s=e; s>path && *s!='/'; --s);
    ++s;
    *dest = new char [e-s+2];
    strncpy(*dest, s, e-s+1);
    (*dest)[e-s+1] = 0;
}

FileDir::FileDir(): name(0), parent(0), next(0) {}
FileDir::~FileDir() {if (name) delete[] name;}
FileDir* FileDir::find(regex& exp){
    if (!name)
        return 0;

    if (regex_match(name, exp))
        return this;
    return 0;
}
FileDir* FileDir::createTree(char *strct){
    char *lname, *rname;

    if (!strct){
        return 0;
    }

    for (lname=strct; *lname && !isalpha(*lname) && *lname!='}' && *lname!=':'; ++lname);
    for (rname=lname; *rname && (isalnum(*rname) || *rname=='.' || *rname=='_'); ++rname);

    if (*rname==':'){
        Dir *file = new Dir();
        file->setName(0);
        if (lname!=rname){
            int size = rname-lname;
            char *name = new char[size+1];
            memcpy(name, lname, size);
            name[size] = 0;
            file->setName(name);
        }
        for (rname+=2; *rname!=']'; ++rname){
            FileDir *aux = createTree(rname);
            for (int x=0; x!=1 || *rname!='}'; ++rname){
                x -= *rname=='}';
                x += *rname=='{';
            }
            aux->next = file->first;
            aux->parent = file;
            file->first = aux;
        }
        return file;
    }
    else{
        File *file = new File();
        file->setName(0);
        if (lname!=rname){
            int size = rname-lname;
            char *name = new char[size+1];
            memcpy(name, lname, size);
            name[size] = 0;
            file->setName(name);
        }
        return file;
    }
}
void FileDir::create(string path){
    if (path.back()!='/')
        path += '/';
    path += name;
    FILE *tout = fopen(path.c_str(), "w");
    fclose(tout);
}
string FileDir::getPath(){
    if (!name)
        return "";
    if (parent){
        string parentPath = parent->getPath();
        if (parentPath.back()=='/') parentPath.pop_back();
        return parentPath+'/'+name;
    }
    return name;
}

File::File(const char *name_){
    int len;

    if (!name_)
        return;

    len = strlen(name_);
    if (len){
        name = new char[len+1];
        strcpy(name, name_);
        name[len] = 0;
    }
}
FileDir* File::find(const char *exp){
    if (!name){
        return 0;
    }
    if (regex_match(name, regex(exp))){
        return this;
    }
    return 0;
}
void File::getStructure(string &strct){
    strct += "{";
    if (name)
        strct += name;
    strct += "}";
}
void File::getFiles(vector<FileDir*> &files){
    files.push_back(this);
}

Dir::Dir(const char *dirName): first(0){
    ///verifica lungimea maxima a cailor
    DIR *dir;
    dirent *ent;
    int len;
    char path[MAX_PATH]={};

    if (!dirName){
        return;
    }
    len = strlen(dirName); ///verifica daca len>maxpath
    strncpy(path, dirName, MAX_PATH);
    if (path[len]!='/'){
        path[len++]='/';
    }
    if ( (dir=opendir(path)) ){
        baseName(dirName, &name);
        while ( (ent=readdir(dir)) ){
            //ignoram fisierele ascunse
            if (ent->d_name[0]!='.'){
                strncpy(fullPath, path, MAX_PATH);
                strcat(fullPath, ent->d_name);
                lstat(fullPath, &st);
                if (S_ISDIR(st.st_mode)){
                    FileDir *child = new Dir(fullPath);
                    child->next = first;
                    child->parent = this;
                    first = child;
                }
                else{
                    FileDir *child = new File(ent->d_name);
                    child->next = first;
                    child->parent = this;
                    first = child;
                }
            }
        }
        closedir(dir);
    }
    else{
        //dir nu exista
    }
}
Dir::~Dir(){
    for (FileDir *i=first; i;){
        FileDir *aux = i;
        i = i->next;
        delete aux;
    }
}
FileDir* Dir::find(const char *exp){
    if (!name)
        return 0;
    if (regex_match(name, regex(exp)))
        return this;
    for (FileDir *p=first; p; p=p->next){
        FileDir *retValue;
        if ( (retValue=p->find(exp)) )
            return retValue;
    }
    return 0;
}
FileDir* Dir::find(regex& exp){
    if (!name)
        return 0;
    if (regex_match(name, exp))
        return this;
    for (FileDir *p=first; p; p=p->next){
        FileDir *retValue;
        if ( (retValue=p->find(exp)) )
            return retValue;
    }
    return 0;
}
void Dir::getStructure(string &strct){
    strct += "{";
    if (name)
        strct += name;
    strct += ":[";
    for (FileDir *p=first; p; p=p->next){
        p->getStructure(strct);
    }
    strct += "]}";
}
void Dir::create(string path){
    if (path.back()!='/')
        path += '/';
    path += name;
    mkdir(path.c_str(), 0755);
    for (FileDir *p=first; p; p=p->next){
        p->create(path);
    }
}
string Dir::getPath(){
    if (!name)
        return "";
    if (parent){
        string parentPath = parent->getPath();
        if (parentPath.back()=='/') parentPath.pop_back();
        return parentPath+'/'+name;
    }
    return name;
}
void Dir::getFiles(vector<FileDir*> &files){
    for (FileDir *p=first; p; p=p->next){
        p->getFiles(files);
    }
}

Root::Root(const char* dirName): Dir(dirName), path(0){
    if (name){
        int len;

        delete[] name;
        name = 0;
        len = strlen(dirName);
        if (dirName[len-1]!='/'){
            path = new char[len+2];
            strcpy(path, dirName);
            path[len++] = '/';
            path[len] = 0;
        }
        else{
            path = new char[len+1];
            path[len] = 0;
            strcpy(path, dirName);
        }
    }
}
Root::~Root() {if (path) delete[] path;}
FileDir* Root::find(const char *exp){
    for (FileDir *p=first; p; p=p->next){
        FileDir *retValue;
        if ( (retValue=p->find(exp)) )
            return retValue;
    }
    return 0;
}
string Root::getPath(){
    if (path)
        return path;
    return "";
}
