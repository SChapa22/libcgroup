#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void permanent_sleep(){
	while (1){
		sleep(10000);
	}
}

int main(){
	char *delegated = "/testing.slice/testing-delegated.slice/testing-delegated.scope";
	int childPID = fork();
	if(childPID < 0){ //No child process created
		fprintf(stderr, "fork() failed: %d:%s\n", childPID, strerror(-childPID));
	}
	else if (childPID == 0){ //This is the child process, sleep forever
		permanent_sleep();
	}
	else{ //This is the parent process, move child process to the desired slice/scope
		char comm[1000];
		char childPIDstr[100];
		sprintf(childPIDstr, "%d", childPID);
		strcat(comm, "cgclassify -g cpu,memory,blkio:");
		strcat(comm, delegated);
		strcat(comm, " ");
		strcat(comm, childPIDstr);
		system(comm);
		
		fprintf(stdout, "%d, %s\n%s\n", childPID, delegated, comm);
	}
	fflush(stderr);
}
