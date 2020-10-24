#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int  main(int argc,char* argv[]){
	if(argc<2){ 
		printf("you should enter a sleep time");
		exit(0);
	}
	else if(argc == 2){
		printf("sleep for %s seconds\n",argv[1]);
		sleep(atoi(argv[1]));
		printf("sleep end\n");
		exit(0);
	}else{
		printf("too many args\n");	
		exit(1);
	}
	exit(0);
}
