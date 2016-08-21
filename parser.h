#include <malloc.h>
#include <sys/wait.h>

#include "history.h"
#include "builtins.h"

int pipefd[1000]={0};
int current_command_number=1,echo_set=0,in_exists=0,out_exists=0,in_modified=0,out_modified=0;
char *commands_to_implement[3] = {"history","!!","echo"};
char echo_string_entered[1000];
char echo_token[1000];

struct commands
{
	char* command;
};

typedef struct commands Commands;
void executeProcess(int in,int out,char *cmd[]);

Commands* all_commands[1000];							//array of structure command to store all command separated by pipe'|'

char* addSpecialCharacterToToken(char* token_passed)
{
	int len_token=strlen(token_passed);
	int i=0;

	while(i<len_token && token_passed[i] != '"')
		i++;

	if(i<len_token && token_passed[i] == '"')
	{
		token_passed[i]=' ';
		i++;
		while(token_passed[i] != '"')
		{
			if(token_passed[i] == ' ')
				token_passed[i]=5;
			i++;
		}
		if(token_passed[i] == '"')
			token_passed[i]=' ';
	}

	i=0;
	while(i<len_token && token_passed[i] != '\'')
		i++;

	if(i<len_token && token_passed[i] == '\'')
	{
		token_passed[i]=' ';
		i++;
		while(token_passed[i] != '\'')
		{
			if(token_passed[i] == ' ')
				token_passed[i]=5;
			i++;
		}
		if(token_passed[i] == '\'')
			token_passed[i]=' ';
	}

	return token_passed;
}


char* checkForRedirections(int type_of_fd,char* this_token)						//type_of_fd=0 means 'in' required else 'out'
{
	int length_of_this_token=strlen(this_token),i=0;
	char *input_file=(char*)malloc(sizeof(char)*length_of_this_token), *output_file=(char*)malloc(sizeof(char)*length_of_this_token);

	if(type_of_fd == 0)
	{
		int in_index=0;
		for(i=0;i<length_of_this_token;i++)
		{
			if(this_token[i] == '<')
			{
				in_exists=1;
				this_token[i]=' ';
				i++;
				break;
			}
		}

		while(i<length_of_this_token && this_token[i] == ' ')
			i++;
		while(i<length_of_this_token && this_token[i] != ' ')
		{
			input_file[in_index] = this_token[i];
			in_index++;
			this_token[i]=' ';
			i++;
		}

		if(in_exists == 1)
			in_modified = open(input_file, O_RDONLY);
	}

	else if(type_of_fd == 1)
	{
		int out_index=0;
		for(i=0;i<length_of_this_token;i++)
		{
			if(this_token[i] == '>')
			{
				this_token[i]=' ';
				i++;
				out_exists=1;
				break;
			}
		}

		while(i<length_of_this_token && this_token[i] == ' ')
			i++;
		while(i<length_of_this_token && this_token[i] != ' ')
		{
			output_file[out_index] = this_token[i];
			this_token[i]=' ';
			out_index++;
			i++;
		}

		if(out_exists == 1)
			out_modified = open(output_file, O_WRONLY | O_TRUNC | O_CREAT,0644);
	}

return this_token;
}

char* removeSpecialCharacters(char* token_with_sp_char)
{
	int len=strlen(token_with_sp_char),i=0;

	for(i=0;i<len;i++)
	{
		if(token_with_sp_char[i] == 5)
			token_with_sp_char[i]=' ';
		i++;
	}
	return token_with_sp_char;
}

void parse(char* inputString[])
{	
	int no_of_commands=0,index_of_command=0,i=0,in=0,out=0,executeEcho=0;
	//no_of_commands will give the number of commands written if multiple pipes are used
	char* token=NULL;
	token=strtok(inputString[0],"|");


	int len=strlen(token);
	
	for(i=0;i<len;i++)
		echo_token[i]=token[i];

	/* parsing the string and putting all the tokens into the commands structure */
	while(token != NULL)
	{
		token=addSpecialCharacterToToken(token);         //adding special characters as '"' and spaces can be there which execvp cannot handle

		Commands* temp_commands=(Commands*) malloc(sizeof(Commands));
		temp_commands->command=token;
		no_of_commands++;

		all_commands[index_of_command]=temp_commands;			//it will add value of this token to the array at next index
		index_of_command++;

		token=strtok(NULL,"|");
	}



	for(i=0;i<no_of_commands-1 && no_of_commands!=1 ;i++)
	{
		char *token_command1=strtok(all_commands[i]->command," ");				//break the first command on basis of spaces
		char *first_array[1000];
		int count=0,index1=1;
		
		int fd[2];
		pipe(fd);
		out_exists=0;
		in_exists=0;
		out=fd[1];

		token_command1=checkForRedirections(0,token_command1);
		token_command1=checkForRedirections(1,token_command1);
		//for '1' it will give 'out' after checking if redirection is there or not

		//splitting command1 on ' ' -space and putting all the tokens in the first_array
		while(token_command1 != NULL)
		{
			token_command1=removeSpecialCharacters(token_command1);
			if(count==0)
			{
				first_array[0]=token_command1;
				count++;
			}

			else
			{
				first_array[index1]=token_command1;
				index1++;
			}
			token_command1=strtok(NULL," ");
		}
		first_array[index1]='\0';

		if(out_exists == 0)
			out=fd[1];
		else if(out_exists == 1)
			out=out_modified;

		if(in_exists == 1)
			in=in_modified;

		executeProcess(in,out,first_array);
		close(fd[1]);

		in=fd[0];

	} //end of for loop

/* now after ending the for loop, execute the last command for which the output is STDOUT */
	
	pid_t pid=fork();

	if(pid == 0)
	{	
		out=1;
		if(in != 0)
	    	dup2(in, 0);
	    
		out_exists=0;
		in_exists=0;

		char *token_command_temp1=all_commands[no_of_commands-1]->command;

		token_command_temp1=checkForRedirections(0,token_command_temp1);		//for '0' it will give 'in'
		token_command_temp1=checkForRedirections(1,token_command_temp1);

		char *token_command1=strtok(token_command_temp1," \n");

		if(out_exists == 1)
			out=out_modified;
		if(in_exists == 1)
			in=in_modified;

		if(out!= 1)													//if it is not the last command,output to pipe()
		  {
		 	dup2(out,1);
		 	close(out);
		  }

		  if(in!=0)														//if it is not the first command,input from pipe()
		  {
		    dup2(in,0);
		    close(in);
		  }

		char *second_array[100];
		int index2=0,length_of_bang_number;

		//splitting command1 on ' ' -space and putting all the tokens in the first_array
		while(token_command1 != NULL)
		{
			token_command1=removeSpecialCharacters(token_command1);
			second_array[index2]=token_command1;
			index2++;
			token_command1=strtok(NULL," ");
		}
		second_array[index2]='\0';

		length_of_bang_number=strlen(second_array[0]);

		if(strcmp(second_array[0],commands_to_implement[0])==0)				//if the first or last command is history
		{
			if(second_array[1] == NULL)
				executeHistoryCommand();
			else
			{
				int history_number = atoi(second_array[1]);
				executeHistoryNumber(history_number);
				// printf("%d",history_number);
			}

		}

		else if(strcmp(second_array[0],commands_to_implement[1])==0)				// comparing with "!!"
		{
			executeBangBang(0,1);
			printf("%s\n",temporary[0]);
			parse(temporary);
		}
		
		//to handle !x, x is a number
		else if(length_of_bang_number>1 && second_array[0][0] == '!' && second_array[0][1]>='0' && second_array[0][1]<='9')
		{
			int i=0,bang_number=0;
			char number_bang[20];
			for(i=1;i<length_of_bang_number;i++)						//if !32 is given, then number should be 32
				number_bang[i-1]=second_array[0][i];
			bang_number = atoi(number_bang);
			executeBangBang(1,bang_number);								//1 for !number.
			printf("%s\n",temporary[0]);
			parse(temporary);
		}

		else if(length_of_bang_number>2 && second_array[0][0] == '!' && second_array[0][1] == '-')		//!-number							//to handle !x, x is a number
		{
			int i=0,bang_number=0;
			char number_bang[20];
			for(i=2;i<length_of_bang_number;i++)						//if !32 is given, then number should be 32
				number_bang[i-2]=second_array[0][i];
			bang_number = atoi(number_bang);

			if(bang_number == 0)
			{
				perror("bash: !-0: event not found");
			}
			else
			{	
				executeBangBang(2,bang_number);						
				printf("%s\n",temporary[0]);
				parse(temporary);
			} 
		}


		else if(length_of_bang_number>1 && second_array[0][0]=='!')		//!alphabet		
		{
			if(second_array[0][1]>='a' || second_array[0][1]<='z' || second_array[0][1]>='A' || second_array[0][1]<='Z')
			{
				int i=0;
				char compare_this[length_of_bang_number];
				for(i=1;i<length_of_bang_number;i++)
					compare_this[i-1]=second_array[0][i];
				bangName(length_of_bang_number,compare_this);
				executeBangBang(1,line_number_file);								//1 for !number.
				printf("%s\n",temporary[0]);
				parse(temporary);
			} 
		}

		else if(strcmp(second_array[0],"cd")==0)								//for cd
			changeDirectory(second_array);

		else if(strcmp(second_array[0],"echo") == 0 && executeEcho == 0)							//for echo
		{
			// implementEcho(second_array);
			implementEcho1(echo_token);
			executeEcho=1;
			parse(temporary1);
		}

		else
		{	
			if(executeEcho == 1)
      			executeEcho=0;
			execvp(second_array[0],second_array);											

		}

	}

	else if (pid > 0)
		wait(NULL);
	

}	//end of parse()


void executeProcess(int in,int out,char *thisCommand[])
{
  int length_of_bang_number=strlen(thisCommand[0]),executeEcho=0;
  pid_t ppid = fork();
  if(ppid==0)														//child process
    {
      if(out!= 1)													//if it is not the last command,output to pipe()
      {
     	dup2(out,1);
     	close(out);
      }

      if(in!=0)														//if it is not the first command,input from pipe()
	  {
	    dup2(in,0);
	    close(in);
	  }

	if(strcmp(thisCommand[0],commands_to_implement[0])==0)				//if the first or last command is history
	{
		if(thisCommand[1] == NULL)
			executeHistoryCommand();
		else
		{
			int history_number = atoi(thisCommand[1]);
			executeHistoryNumber(history_number);
		}

	}

	 else if(strcmp(thisCommand[0],commands_to_implement[1])==0)				// comparing with "!!"
	  {
		executeBangBang(0,1);										//0 for type of bang, 1 is just like that
		// printf("inside function !! else if");
		printf("%s\n",temporary[0]);
		parse(temporary);
	  }

	else if(length_of_bang_number>1 && thisCommand[0][0] == '!' && thisCommand[0][1]>= '0' && thisCommand[0][1] <= '9')		//!number								//to handle !x, x is a number
	{
		int i=0,bang_number=0;
		char number_bang[20];
		for(i=1;i<length_of_bang_number;i++)						//if !32 is given, then number should be 32
			number_bang[i-1]=thisCommand[0][i];
		bang_number = atoi(number_bang);

		if(bang_number == 0)
			perror("bash: !0: event not found");

		executeBangBang(1,bang_number);								//1 for !number.
		printf("%s\n",temporary[0]);
		parse(temporary); 
	}

	else if(length_of_bang_number>2 && thisCommand[0][0] == '!' && thisCommand[0][1] == '-')		//!-number							//to handle !x, x is a number
	{
		int i=0,bang_number=0;
		char number_bang[20];
		for(i=2;i<length_of_bang_number;i++)						//if !32 is given, then number should be 32
			number_bang[i-2]=thisCommand[0][i];
		bang_number = atoi(number_bang);
		executeBangBang(2,bang_number);								//1 for !number.
		printf("%s\n",temporary[0]);
		parse(temporary); 
	}

	else if(length_of_bang_number>1 && thisCommand[0][0]=='!')		//!alphabet		
	{
		if(thisCommand[0][1]>='a' || thisCommand[0][1]<='z' || thisCommand[0][1]>='A' || thisCommand[0][1]<='Z')
		{
			int i=0;
			char compare_this[length_of_bang_number];
			for(i=1;i<length_of_bang_number;i++)
				compare_this[i-1]=thisCommand[0][i];
			bangName(length_of_bang_number,compare_this);
		} 
	}

	else if(strcmp(thisCommand[0],"echo") == 0 && executeEcho==0)							//for echo
	{
		implementEcho1(echo_token);
		executeEcho=1;
		parse(temporary1);
	}

	else
    {
      	if(executeEcho == 1)
      		executeEcho=0;
      	execvp(thisCommand[0],thisCommand);

    }

    }

    else if (ppid > 0)
    	wait(NULL);
}