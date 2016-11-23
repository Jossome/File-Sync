#include<iostream>
#include<stdio.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/dirent.h>
#include<sys/dir.h>
#include<unistd.h>
#include<string>
#include<map>

using namespace std;

string root, A, B;
string rootA, rootB;

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
            stat((pathA + "/" + filename).c_str(), &buf);
            files.insert(make_pair(filename, buf.st_mtime));
        }
    }
    
    dir = opendir(pathB.c_str());
    while ((dirptr = readdir(dir)) != NULL){
        string filename;
		filename.assign(dirptr->d_name);
        if (filename == "." || filename == "..") continue;
        if (dirptr->d_type == 4){
            if (dirs.find(filename) == dirs.end()){
                system(("cp -p -r " + pathB + "/" + filename + " " + pathB + "/" + filename + "_bk").c_str());
                system(("rm -r -i " + pathB + "/" + filename).c_str());
                cout << ("Folder deleted: " + pathB + "/" + filename) << endl;
            }
            else dirs.find(filename)->second = 0;
        }
        else{
            if (files.find(filename) == files.end()){
                system(("cp -p " + pathB + "/" + filename + " " + pathB + "/" + filename + "_bk").c_str());
                system(("rm -r -i " + pathB + "/" + filename).c_str());
                cout << ("File deleted: " + pathB + "/" + filename) << endl;
                cout << ("File backup: " + pathB + "/" + filename + "_bk") << endl;
            }
            else{
                struct stat buf;
                stat((pathB + "/" + filename).c_str(), &buf);
                if (files.find(filename)->second != buf.st_mtime){
                    system(("cp -p " + pathB + "/" + filename + " " + pathB + "/" + filename + "_bk").c_str());
                    system(("cp -p " + pathA + "/" + filename + " " + pathB + "/" + filename).c_str());
                    cout << ("File synchronized: " + pathB + "/" + filename) << endl;
                    cout << ("File backup: " + pathB + "/" + filename + "_bk") << endl;
                }
                files.find(filename)->second = 0;
            }
        }
    }
    for (it = dirs.begin(); it != dirs.end(); it++){
        if (it->second){
            system(("mkdir "+ pathB + "/" + it->first).c_str());
            cout << ("Folder created: " + pathB + "/" + it->first);
        }
        synchronize(curA + "/" + it->first, curB + "/" + it->first);
    }
    for (it = files.begin(); it != files.end(); it++){
        if (it->second){
            system(("cp -p " + pathA + "/" + it->first + " " + pathB + "/" + it->first).c_str());
            cout << ("File created: " + pathB + "/" + it->first) << endl;
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
	A.assign(a); B.assign(b);
	rootA = root + "/" + A;
	rootB = root + "/" + B;
    freopen((root+"/log").c_str(), "w", stdout);
    synchronize("", "");
    fclose(stdout);
    cout << "Synchronization finished." << endl;
    return 0;
}

