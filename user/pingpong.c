#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int  main(int argc,char* argv[]){
    int p[2];
    if(pipe(p)!=0){
        printf("pipe failed\n");
    }
    int pid = fork();
    if(pid<0){
        printf("fork failed\n");
    }else if(pid==0){
        //child
        write(p[0],"ping",4);
        close(p[0]);
        void *res;
        read(p[1],res,4);
        printf("child:received %s",*((char *)res));
    }else{
        //parent
        write(p[1],"pong",4);
        void *res;
        read(p[0],res,4);
        printf("parent:received %s",*((char *)res));
    }
    exit(0);
}