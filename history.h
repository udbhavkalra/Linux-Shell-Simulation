#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>										//for write
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void executeBangBang(int, int);

char path_history_global[100]="";
int no_of_lines_hist_file=0,line_number_file=0;
char lastLine_history_file[1000];
char* temporary[0];

void initializeArrayofCharacters(char s[],int size)
{
	int i=0;
	for(;i<size;i++)
		s[i]='\0';
}

int getNumberofLinesInHistoryFile(int file_des)
{
	char buf[1];
	int count=0;
	while(read(file_des,buf,1)>0)
	{
		if(*buf == '\n')
			count++;
	}

	return count;
}


//create a history file if not exists for the current user and appends the command executed to that file
void createHistoryFileIfNotExists(char string[],int bang_is_there)				
{
	char* user;
	user="USER";
	char *username=getenv(user);
	int open_fd=0,read_fd=0;
	
// printf("%d\n",no_of_lines_hist_file);
	char path_for_history_file[100]="/home/";
	char line_number_with_command[1000]=" ";
	strcat(path_for_history_file,username);				//to get the user name and create a file at that location
	strcat(path_for_history_file,"/udbhav_shell_history");

	strcpy(path_history_global,path_for_history_file);				//to use the path of history file in other functions

	open_fd = open(path_for_history_file, O_WRONLY | O_CREAT | O_APPEND, 0644);		//to write file in append mode, if file doesn't exists, create one
	
	read_fd = open(path_for_history_file, O_RDONLY);							//for reading file
	no_of_lines_hist_file=getNumberofLinesInHistoryFile(read_fd)+1;

// printf("#lines: %d\n",no_of_lines_hist_file);
	if(open_fd && bang_is_there == 0)
	{
		sprintf(line_number_with_command,"%d",no_of_lines_hist_file);
		strcat(line_number_with_command,"  ");
		strcat(line_number_with_command,string);

		write(open_fd,line_number_with_command,strlen(line_number_with_command)); //string is the input command from the shell
	}

	else if (open_fd < 0)
		perror("Error in opening/creating the file");
}

//execute the history command or simply cat the file which was created by the createHistoryFileIfNotExists()
void executeHistoryCommand()
{	
	char* array_to_exec[3];
  	array_to_exec[0]="cat";
  	array_to_exec[1]="/home/udbhav/udbhav_shell_history";
  	array_to_exec[2]='\0';
  	if(!fork())
	  	execvp(array_to_exec[0],array_to_exec);
	exit(0);
}

//this will print the lines from history file from line number mentioned to the last
void printCommandsLineNumber(int number_of_commands_from_last,int read_fd,int type_of_function_called)
{
	char buf[1],character[1];
	int count=0;
	int number=no_of_lines_hist_file-number_of_commands_from_last;

	initializeArrayofCharacters(lastLine_history_file,1000);

	while(read(read_fd,buf,1)>0)
	{
		if(*buf == '\n')
			count++;
		if(count == number)
			break;
	}
	
	count=0;
	if(type_of_function_called == 0)							//if called for printing line number to last
	{	while(read(read_fd,buf,1)>0)
		{
			if(*buf == '\n')
				count++;

			if(count != number_of_commands_from_last)
				printf("%c",*buf);
		}
	}

	if(type_of_function_called==1)							//to access only the last line of the history file
	{
		while(read(read_fd,buf,1)>0 && *buf != ' ');				//first remove the number in last line
		while(read(read_fd,buf,1)>0 && *buf == ' ');				//then remove spaces
		
		//add first character read after the space to the "lastLine_history_file", else it will disappear after next read
		character[0]=*buf;
		strcat(lastLine_history_file,character);				

		while(read(read_fd,buf,1)>0 && *buf != '\n')
		{
			character[0] = *buf;
			strcat(lastLine_history_file,character);			//concat each character from buf to the last line
		}
	}
}

void executeHistoryNumber(int number1)
{
	int read_fd = open(path_history_global, O_RDONLY);
	printCommandsLineNumber(number1,read_fd,0);									//'0' for line number to end
	printf("\n");
}

void executeBangBang(int type_of_bang,int number_line)
{
	int read_fd = open(path_history_global, O_RDONLY),i=0;

	if(type_of_bang == 0)													//last command i.e. !!
		printCommandsLineNumber(2,read_fd,1);

	else if(type_of_bang == 1)												//!number
		printCommandsLineNumber(no_of_lines_hist_file-number_line+1,read_fd,1);
		// printCommandsLineNumber(no_of_lines_hist_file-number_line,read_fd,1);

	else if(type_of_bang == 2)												//for !-number
		printCommandsLineNumber(number_line+1,read_fd,1);

	int length_of_command=strlen(lastLine_history_file);

	//conversion from char[] to char* [] in order to pass it to parser
	temporary[0]=(char*) malloc(sizeof(char)*length_of_command);

	for(i=0;i<length_of_command;i++)
		temporary[0][i]=lastLine_history_file[i];
}

void bangName(int length_of_bang_number,char compare_this[])
{	
	int read_fd = open(path_history_global, O_RDONLY),count=1,i=0;
	char buf[1];

// for(i=0;i<length_of_bang_number;i++)
// 	printf("%c at %d\n",compare_this[i],i);

	if(read_fd < 0)
		perror(" Error while opening the file !\n");

	while(count<no_of_lines_hist_file)
	{	
		i=1;
		while(read(read_fd,buf,1)>0 && *buf != ' ');
		while(read(read_fd,buf,1)>0 && *buf == ' ');
		
		if(*buf != compare_this[0])
		{
			while(read(read_fd,buf,1)>0 && *buf != '\n');
			count++;
			continue;
		}

		while(i<length_of_bang_number-1)
		{	
			if(read(read_fd,buf,1)>=0 && *buf != compare_this[i])
				break;
			i++;
		}
		if(i==length_of_bang_number-1)
			line_number_file=count;

		while(read(read_fd,buf,1)>0 && *buf != '\n');
		count++;
	}

}