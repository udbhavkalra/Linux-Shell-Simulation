//for cd, pwd, export
#include <unistd.h>

char last_cd[400],last_cd_1[400];
int active_last_cd=0;
char* temporary1[2];

void changeDirectory(char* argument_of_cd[])							//called on 'cd'
{
	int status_chdir=0;
	if(argument_of_cd[1] == NULL)
	{
		if(active_last_cd==0)
			getcwd(last_cd,400);
		else
			getcwd(last_cd_1,400);
		if(getenv("HOME") != NULL)										//it will return NULL if there is no match
			status_chdir= chdir(getenv("HOME"));
		if(status_chdir < 0)
			perror("Unable to change Directory!");
	}

	else if(strcmp(argument_of_cd[1],"~-") == 0)
	{
		if(active_last_cd == 0)
		{
			getcwd(last_cd_1,400);
			active_last_cd=1;
			status_chdir=chdir(last_cd);
		}

		else if(active_last_cd == 1)
		{
			getcwd(last_cd,400);
			active_last_cd=0;
			status_chdir=chdir(last_cd_1);
		}
	}

	else
	{	
		if(active_last_cd==0)
			getcwd(last_cd,400);
		else
			getcwd(last_cd_1,400);
		status_chdir=chdir(argument_of_cd[1]);
	}
	return;
}

char* clearQuotes(char* this_echo_arg)
{
	int i=0,length=strlen(this_echo_arg);
	for(i=0;i<length && this_echo_arg[i] != '\0' ;i++)
	{
		if(this_echo_arg[i] == '\'' || this_echo_arg[i] == '"')
			this_echo_arg[i]=' ';
	}
	return this_echo_arg;
}

void implementEcho1(char *echo_token)
{
	int length=strlen(echo_token),i=0,index=0,ind_en=0,first=1;
	char string_to_print[10000],current_env_var[1000];

	for(i=0;i<length;i++)
	{
		if(echo_token[i] == '\'' || echo_token[i] == '"')
			echo_token[i]=' ';
	}

	length=strlen(echo_token);
	
	for(i=0;i<length;i++)
	{
		ind_en=0;
		if(first==1)
		{
			while(i<length && echo_token[i] != ' ')
				i++;
			while(i<length && (echo_token[i] == ' ' || echo_token[i]=='"' || echo_token[i]=='\''))
				i++;
			first=0;
		}

		while(i<length && echo_token[i] != '$')
		{
			string_to_print[index] = echo_token[i];
			index++;
			i++;
		}
		if(echo_token[i] == '$')
		{
			i++;
			while(i<length && echo_token[i] != ' ')
			{
				current_env_var[ind_en]=echo_token[i];
				ind_en++;
				i++;
			}
			if(getenv(current_env_var) != NULL)
			{
				// int k=0;
				// temp=getenv(current_env_var);
				// while(k<150 && temp[k] != '\0')
				// {
				// 	string_to_print[index]=temp[k];
				// 	k++;
				// }
				int len= strlen(getenv(current_env_var));
				strcat(string_to_print,getenv(current_env_var));
				index = index+ len;
			}
			temporary1[0]=(char*) malloc(sizeof(char)*index);
			temporary1[1]=(char*) malloc(sizeof(char)*index);

			string_to_print[index++]=' ';
			temporary1[0] = "echo";
			for(i=0;i<index;i++)
				temporary1[1][i]=string_to_print[i];

			
			memset(current_env_var,'\0',1000);
		}
	}
printf("%s\n",string_to_print);
memset(string_to_print,'\0',10000);
}


void implementExport(char* export_line[])
{
	if(export_line[2] != NULL)
		printf("bash: export:`%s` not a valid identifier\n",export_line[2]);
	
	else
	{
		char *temp=export_line[1],*varname,*value;
		varname = (char*)malloc(sizeof(char)*100);
		value= (char*)malloc(sizeof(char)*100);
		int length=strlen(temp),i=0,val_index=0,status=0;

		for(i=0;i<length;i++)
		{
			while(i<length && temp[i] != '=')
			{
				varname[i]=temp[i];
				i++;
			}
			i++;
			while(i<length)
			{
				value[val_index]=temp[i];
				val_index++;
				i++;
			}
		}

		status=setenv(varname," ",100);
		status=setenv(varname,value,100);

		if(status < 0)
			printf("Unable to set the variable:%s\n",varname);
	}
}