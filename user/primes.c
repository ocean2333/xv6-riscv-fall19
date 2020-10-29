#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int
main(int argc, char *argv[])
{
    if(argc  != 1){
        fprintf(2, "Usage: primes\n");
        exit(1);
    }
    int pipe1[2];
    pipe(pipe1);
    int pid;
    pid = fork();
    if(0==pid){
        close(pipe1[1]);
        int p;
        int r;
        while((r=read(pipe1[0],(void*)&p,sizeof(p)))!=0){
            int pipe2[2];
            pipe(pipe2);
            printf("primes:%d\n",p);
            int pid2 =fork();
            if(-1==pid2){
                printf("too big!!!\n");
                exit(1);
            }
            else if(0==pid2){
                close(pipe1[0]);
                close(pipe2[1]);
                pipe1[0]=pipe2[0];
                sleep(1);
            }else{
                close(pipe2[0]);
                int ne;
                while((r=read(pipe1[0],(void*)&ne,sizeof(ne)))!=0){
                    if(ne%p!=0){
                        write(pipe2[1],(void*)&ne,sizeof(ne));
                    }
                }
                close(pipe1[0]);
                close(pipe2[1]);

                exit(0);
            }
        }
        exit(0);
    }else{
        close(pipe1[0]);
        for(int i=2;i<32;i++){
            write(pipe1[1],(void*)&i,sizeof(i));
        }
        close(pipe1[1]);
        wait(0);
	sleep(5);
        exit(0);
    }
}
