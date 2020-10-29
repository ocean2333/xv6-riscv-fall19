#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
void sourse();
void filter(int prime);
void sink();
void redirect(int k,int p[]);

int  main(int argc,char* argv[]){
    int p[2];
    if(pipe(p)!=0){
        printf("pipe failed\n");
        exit(0);
    }
    int pid = fork();
    if(pid<0){
        printf("fork failed\n");
        exit(0);
    }else if(pid==0){
        //child
        redirect(0,p);
        sink();
    }else{
        //parents
        redirect(1,p);
        sourse();
    }
    exit(0);
}

//input nums to pipe
void sourse(){
    for(int i=2;i<36;i++){
        write(1, &i, sizeof(i));
    }
}

//use a prime to check num
void filter(int prime){
    int n;
    while(read(0,&n,sizeof(n))){
        if(n%prime!=0){
            //give it to pipe
            write(0,&n,sizeof(n));
        }
    }
}

//
void sink(){
    int p[2];
    int prime;
    if(read(0,&prime,sizeof(prime))){
        printf("prime:%d",prime);
        pipe(p);
        if(fork()){
            redirect(0,p);
            sink();
        }else{
            redirect(1,p);
            filter(prime);
        }
    }
}

//let stdin turn to p[n]
void redirect(int n,int p[2]){
    close(n);
    dup(p[n]);
    close(p[0]);
    close(p[1]);
}
