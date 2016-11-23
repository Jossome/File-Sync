#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <windows.h>
#include <sys/types.h>
# define __dirfd(dp)    ((dp)->dd_fd)

typedef struct _dirdesc{
    int     dd_fd;
    long    dd_loc;
    long    dd_size;
    char    *dd_buf;
    int     dd_len;
    long    dd_seek;
} DIR;

DIR *opendir (const char *);
struct dirent *readdir (DIR *);
void rewinddir (DIR *);
int closedir (DIR *);

struct dirent{
     long d_ino;
     off_t d_off;
     unsigned short d_reclen;
     unsigned char d_type;
     char d_name[1];
};

using namespace std;

string root, A, B;
string rootA, rootB;
FILE tempA, tempB;

static HANDLE hFind;

DIR *opendir(const char *name){
    DIR *dir;
    WIN32_FIND_DATA FindData;
    char namebuf[512];
    sprintf(namebuf, "%s\\*.*",name);
    hFind = FindFirstFile(namebuf, &FindData );
    if(hFind == INVALID_HANDLE_VALUE){
        printf("FindFirstFile failed (%d)\n", GetLastError());
        return 0;
    }

    dir = (DIR *)malloc(sizeof(DIR));
    if(!dir){
        printf("DIR memory allocate fail\n");
        return 0;
    }

    memset(dir, 0, sizeof(DIR));
    dir->dd_fd = 0;
    return dir;
}

struct dirent *readdir(DIR *d){
    int i;
    static struct dirent dirent;
    BOOL bf;
    WIN32_FIND_DATA FileData;
    if(!d){
        return 0;
    }

    bf = FindNextFile(hFind,&FileData);
    if(!bf){
        return 0;
    }

    for(i = 0; i < 256; i++){
        dirent.d_name[i] = FileData.cFileName[i];
        if(FileData.cFileName[i] == '\0') break;
    }
    dirent.d_reclen = i;
    dirent.d_reclen = FileData.nFileSizeLow;

    if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
        dirent.d_type = 4;
    }
    else{
        dirent.d_type = 8;
    }
    return (&dirent);
}

int closedir(DIR *d){
    if(!d) return -1;
    hFind=0;
    free(d);
    return 0;
}

int synchronize(string curA, string curB){
    string pathA = rootA + curA;
    string pathB = rootB + curB;
    struct dirent *dirptr;
    map<string, long> dirs, files;
    map<string, long>::iterator it;

    DIR *dir = opendir(pathA.c_str());
    while ((dirptr = readdir(dir)) != NULL){
        string filename;
        filename.assign(dirptr->d_name);
        if (filename == "." || filename == "..") continue;
        if (dirptr->d_type == 4){
            dirs.insert(make_pair(filename, 1));
        }
        else{
            struct stat buf;
            stat((pathA+"\\"+filename).c_str(), &buf);
            files.insert(make_pair(filename, buf.st_mtime));
        }
    }

    dir = opendir(pathB.c_str());
    while ((dirptr = readdir(dir)) !=NULL){
        string filename;
        filename.assign(dirptr->d_name);
        if (filename == "." || filename == "..") continue;
        if (dirptr->d_type == 4){
            if (dirs.find(filename) == dirs.end()){
                system(("rd /s /q " + pathB + "\\" + filename).c_str());
                cout << ("Directory deleted: " + pathB + "\\" + filename) << endl;
            }
            else dirs.find(filename)->second = 0;
        }
        else{
            if (files.find(filename) == files.end()){
                system(("del /q " + pathB + "\\" + filename).c_str());
                cout << ("File deleted: " + pathB + "\\" + filename) << endl;
            }
            else{
                struct stat buf;
                stat((pathB+"/"+filename).c_str(), &buf);
                if (files.find(filename)->second != buf.st_mtime){
                    system(("copy " + pathB + "\\" +filename + " " + pathB + "\\" + filename + "_bk").c_str());
                    system(("copy " + pathA + "\\" +filename + " " + pathB + "\\" + filename).c_str());
                    cout << ("File synchronized: " + pathB + "\\" + filename) << endl;
                    cout << ("File backup: " + pathB + "\\" + filename + "_bk") << endl;
                }
                files.find(filename)->second = 0;
            }
        }
    }
    for (it = dirs.begin(); it != dirs.end(); it++){
        if (it->second){
            system(("mkdir "+ pathB + "\\" + it->first).c_str());
            cout << ("Directory created: " + pathB + "\\" + it->first);
        }
        synchronize(curA + "\\" + it->first, curB + "\\" + it->first);
    }
    for (it = files.begin(); it != files.end(); it++){
        if (it->second){
            system(("copy " + pathA + "\\" + it->first + " " + pathB + "\\" + it->first).c_str());
            cout << ("File created: " + pathB + "\\" + it->first) << endl;
        }
    }
    return 0;
}

int main(int argc, const char * argv[]){
    cout << "This program can sychronize two folders under the same path." << endl;
	cout << "Please enter the path of the root folder: ";
	char p[1000];
	gets(p);
	root.assign(p);
	cout << "Please enter the names of the two folders you want to operate on: " << endl;
	char a[1000], b[1000];
	gets(a); gets(b);
	A.assign(a);
	B.assign(b);
	rootA = root + "\\" + A;
	rootB = root + "\\" + B;
	freopen((root+"\\log.txt").c_str(), "a", stdout);
    synchronize("", "");
    fclose(stdout);
    cout << "Synchronization finished." << endl;
    return 0;
}

