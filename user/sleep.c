#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int  main(int argc,char* argv[]){
	if(argc<2){ 
		printf("you should enter a sleep time");
		exit(0);
	}
	else if(argc == 2){
		int time = atoi(argv[1]);
		if(time<0)
			printf("get %d ,please enter a postive number.",time);
		else{
			printf("sleep for %s seconds\n",argv[1]);
			sleep(time);
			printf("sleep end\n");
		}	
		exit(0);
	}else{
		printf("too many args\n");	
		exit(1);
	}
	exit(0);
}
