#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *re) 
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
      fprintf(2, "ls: cannot open %s\n", path);
      return;
    }

    if(fstat(fd, &st) < 0){
      fprintf(2, "ls: cannot stat %s\n", path);
      close(fd);
      return;
    }

    //get all file in path
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      //refresh path
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';

      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      //judge it is dir or not
      switch (st.type){
        case T_DIR:
          if(strcmp(".",p)!=0 && strcmp("..",p)!=0)
            find(buf,re);
          break;
        case T_FILE:
          if(strcmp(de.name,re)!=0)
            printf("%s\n",p);
          break;
      }
    }

}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find <path> <expression>\n");
    exit(0);
  }
  find(argv[1], argv[2]);
  exit(0);
}