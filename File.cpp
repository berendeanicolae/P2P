#include "File.h"

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

FileDir::FileDir(): name(0), next(0) {}
FileDir::~FileDir() {if (name) delete[] name;}

File::File(const char *name_){
    int len;

    if (!name_)
        return;

    len = strlen(name_);
    if (len){
        name = new char[len+1]; //len+null
        strcpy(name, name_);
        name[len] = 0;
    }
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
            if (ent->d_name[0]!='.'){ //ignoram fisierele ascunse, directorul curent si parintele
                strncpy(fullPath, path, MAX_PATH);
                strcat(fullPath, ent->d_name);
                lstat(fullPath, &st);
                if (S_ISDIR(st.st_mode)){
                    FileDir *child = new Dir(fullPath);
                    //child->next = first;
                    first = child;
                }
                else{
                    FileDir *child = new File(ent->d_name);
                    //child->next = first;
                    first = child;
                }
            }
        }
        closedir(dir);
    }
    else{
        printf("dir nu exista");
    }
}
Dir::~Dir(){
    for (FileDir *i=first; i;){
        FileDir *aux = i;
        i = i->next;
        delete aux;
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
        }
    }
}
Root::~Root() {if (path) delete[] path;}
