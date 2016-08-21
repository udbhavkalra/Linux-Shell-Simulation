#include <signal.h>
#include <string.h>

#include "parser.h"

void handle_signal(int signo);
void showPath();

typedef void (*sighandler_t)(int);

int main()
{
	char input_command_string[10000];

	showPath();

	signal(SIGINT,SIG_IGN);
	signal(SIGINT,handle_signal);

	fgets(input_command_string,sizeof(input_command_string),stdin);

	while(strcmp(input_command_string,"exit\n") != 0)
	{
		int input_length=0;

		input_length=strlen(input_command_string);

		//give the command to history
		if(input_command_string[0] != '!' && input_command_string[1] != '!' && input_length>1)
			createHistoryFileIfNotExists(input_command_string,0);
		
		else
			createHistoryFileIfNotExists(input_command_string,1);

		if(input_length > 1)
		{
			char* temp[0];
			int i=0;
			temp[0]=(char*) malloc(sizeof(char)*1000);

			for(i=0;i<input_length-1;i++)
				temp[0][i]=input_command_string[i];

			parse(temp);

		}

		showPath();
		fgets(input_command_string,sizeof(input_command_string),stdin);
	
	}

	printf("Bye...\n");
	kill(0, SIGKILL);
	exit(0);
	return 0;
}

void handle_signal(int signo)
{
	char path[200];		
	getcwd(path,200);
	fflush(stdout);
	printf("\nudbhav_shell:%s$: ",path);
	fflush(stdout);
}

void showPath()
{
	char path[200];
	getcwd(path,200);
	fflush(stdout);
	printf("udbhav_shell:%s$: ",path);
	fflush(stdout);
}