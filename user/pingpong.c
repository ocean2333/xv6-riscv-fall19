#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int  main(int argc,char* argv[]){
    int p[2];
    int p2[2];
    char res[10];
    if(pipe(p)!=0){
        printf("pipe failed\n");
    }
    if(pipe(p2)!=0){
    	printf("pipe failed\n");
    }

    int pid = fork();
    if(pid<0){
        printf("fork failed\n");
    }else if(pid==0){
        //child
        read(p2[0],res,4);
        printf("%d: received %s\n",getpid(),res);
        write(p[1],"pong",4);   
    }else{
        //parent
        write(p2[1],"ping",4);
        read(p[0],res,4);
        printf("%d: received %s\n",getpid(),res);
    }
    exit(0);
}
