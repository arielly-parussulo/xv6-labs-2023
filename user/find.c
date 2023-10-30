#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char* fmtname(char *path) {
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

    return p;
}

void find(char *path, char* filename) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long: %s\n", path);
      close(fd);
      return;
    }


    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      switch(st.type){
        case T_FILE:
            if (strcmp(fmtname(buf), filename) == 0) {
                printf("%s/%s\n", path, fmtname(buf));
            }
            break;
        case T_DIR:
            if(strcmp(fmtname(buf), ".") == 0 || strcmp(fmtname(buf), "..")  == 0) {
                continue;
            }

            // generate full path
            char fullpath[512];
            memcpy(fullpath, path, strlen(path));
            memcpy(&fullpath[strlen(fullpath)], "/", 1);
            memcpy(&fullpath[strlen(fullpath)], fmtname(buf), strlen(fmtname(buf)));

            find(fullpath, filename);
            break;
      }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: find <dir> <filename>. E.g: find . file.txt\n");
        exit(1);
    } 

    find(argv[1], argv[2]);
    exit(0);
}