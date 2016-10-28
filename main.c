#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[1;31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

typedef struct running running;
struct running{
	int current;
	int process[1000];
	char* processname[1000];
};


running bgstack;


int current_processid;
char current_processname[1000];
void systemcalls(int process_id,char* ,char** ,int);
void changedirectory(char*,char**);
void sendtobackground(int ,char** ,int );
void sigproc(int);
void quitprocess(int);
void proc_exit(int);
void foreg(int);
void sendtobg(int);
void subproc (int, int,int, char** ,int);
int redirection(int, int*,char*,char*,char**,int,int);
//Giving a maximum size
int buffer_space=1024;

int main()
{
	char home_directory[buffer_space],current_directory[buffer_space],command[buffer_space],commandfull[buffer_space],myname[100];
	getcwd(home_directory,sizeof(home_directory));
	int flag=1;
	int rr;
	signalhandler();
	for(rr=0;rr<1000;rr++)
	{
		bgstack.process[rr]=0;
	}
	bgstack.current=1;
	pid_t process_id;
	gethostname(myname,sizeof(myname));
	//Myname has the pc's name.
	do{
		//current directory
		getcwd(current_directory,sizeof(current_directory));


		//Checking if home is same as current.
		if(!strcmp(home_directory,current_directory))
		{

			printf(KGRN"%s ~ $ "RESET,myname);
		}
		else
		{
			//Printing ~ instead of the whole home path
			if(strncmp(current_directory,home_directory,strlen(home_directory))==0)
			{

				printf(KGRN"%s"RESET KBLU " ~ %s $ "RESET,myname,current_directory+strlen(home_directory));	

			}
			else{
				printf(KGRN"%s" RESET KBLU " %s $ "RESET,myname,current_directory);	
			}
		}

		//Reading the command from stdin
		fgets(command,sizeof(command),stdin);

		char* nextptr;

		//Separating the commands on basis of semi-colon
		char* maintoken=strtok_r(command,";",&nextptr);
		int i;
		while(maintoken!=NULL)
		{
			//Removing extra white-spaces
			char* token=strtok(maintoken," \n\t");
			if(token==NULL)
			{
				break;
			}
			//printf("\tToken is~%s\n",token);
			char* argument[buffer_space];
			for(i=0;i<buffer_space;i++)
			{
				argument[i]='\0';
			}
			int itr=0;

			//Commandfull variable contains only the command.
			strcpy(commandfull,token);
			while(token!=NULL)
			{
				argument[itr]=(char*)malloc(sizeof(char)*strlen(token));
				strcpy(argument[itr++],token);
				token=strtok(NULL," \n\t");
			}


			char argswith[buffer_space];
			int argsfull=0;
			int c;
			for(i=0;i<itr;i++)
			{
				//printf("Arguments%sEOF\n",argument[i]);
				for(c=0;argument[i][c]!='\0';c++)
				{
					argswith[argsfull++]=argument[i][c];
				}
				if((i+1!=itr&&argument[i+1][0]=='|'))
				{

				}
				else if(argument[i][0]=='|' )
				{

				}
				else
				{
					argswith[argsfull++]=' ';
				}
			}
			if(argsfull!=0)
				argswith[argsfull-1]='\0';
			int args[buffer_space];
			for(i=0;i<itr;i++)
			{
				args[i]=0;
			}
			int pipecount=0;
			//Default 0
			//Pipe denoted by 1
			//Redirection > denoted by 2
			//Redirection < denoted by 3
			//Appending >> denoted by 4
			for(i=0;i<itr;i++)
			{
				if(argument[i][0]=='|')
				{
					pipecount++;
					args[i]=1;
				}

				else if(argument[i][0]=='<')
				{
					args[i]=3;
				}
				if(argument[i][0]=='>')
				{
					args[i]=2;
				}
				if(argument[i][0]=='>'&&argument[i][1]=='>')
				{
					args[i]=4;
				}
			}
			//printf("Piped%sEOF\n",argswith);
			char* nxt;
			char* piped;
			piped=strtok_r(argswith,"|",&nxt);
			char* pipedargs[buffer_space];
			int start=0;
			int j;
			char *temporarycommand[buffer_space];
			j=0;
			while(temporarycommand[j]!=NULL)
			{
				temporarycommand[j]='\0';
				free(temporarycommand[j++]);
			}			
			pid_t pid;
			int in,red_1,red_2, fd [2];
			in = 0;
			int x=0;
			char inputfile[buffer_space];
			char outputfile[buffer_space];
			while(piped!=NULL)
			{
				//printf("Post piping%sEOF\n",piped);
				int end;
				end=itr;
				for(i=start;i<itr;i++)
				{
					if(args[i]==1)
					{
						end=i;
						//args[i]=0;
						break;
					}
				}
				x=0;
				for(j=start;j<end;j++)
				{
					temporarycommand[x]=(char*)malloc(sizeof(char)*strlen(argument[j]));
					strcpy(temporarycommand[x++],argument[j]);
				}
				temporarycommand[x]='\0';

				for(j=0;j<x;j++)
				{
					//printf("%s\n",temporarycommand[j]);
				}
				//printf("\n");
				red_1=0;
				red_2=0;
				int iterator=0;

				for (j = start; j < end; j++,iterator++)
				{
					if (args[j]==3)
					{
						red_1=1;
						strcpy(inputfile,argument[j+1]);
						x=iterator;
						temporarycommand[iterator]='\0';
					}
					else if (args[j]==2 || args[j]==4)
					{
						if(args[j]==4)
						{
							red_2=2;
						}
						else
						{
							red_2=1;
						}
						strcpy(outputfile,argument[j+1]);
						if(!red_1)
						{
							x=iterator;
							temporarycommand[iterator]='\0';
						}
					}
				}
				for(j=0;j<x;j++)
				{
					//printf("Command%sEOF\n",temporarycommand[j]);
				}
				//printf("%s %s\n",inputfile,outputfile);
				pid_t proc;

				if(pipecount>=1)
				{
					if(!strcmp(commandfull,"cd"))
					{
						changedirectory(home_directory,temporarycommand);
						//printf(KMAG"%s\n" RESET ,current_directory);
					}
					else if(!strcmp(commandfull,"exit")||!strcmp(commandfull,"quit"))
					{
						//Closing our shell
						exit(0);
					}

					else{
						pipe (fd);

						pid_t processes;
						processes = fork ();

						if(red_1==1||red_2)
						{
							redirection(processes,fd,inputfile,outputfile,temporarycommand,red_1,red_2);
						}
						subproc (processes,in, fd [1], temporarycommand,x);

						close (fd [1]);

						in = fd [0];

						pipecount--;
					}
				}

				start=i+1;
				piped=strtok_r(NULL,"|",&nxt);
			}
			if(pipecount==0)
			{

				if(!strcmp(commandfull,"cd"))
				{
					changedirectory(home_directory,temporarycommand);
					//printf(KMAG"%s\n" RESET ,current_directory);
				}
				else if(!strcmp(commandfull,"exit")||!strcmp(commandfull,"quit"))
				{
					//Closing our shell
					exit(0);
				}	
				else if(!strcmp(temporarycommand[0],"jobs"))
				{
					int cntr=1;
					for(i=1;i<bgstack.current;i++)
					{
						if(bgstack.process[i]!=0)
						{	

							printf("[%d]" KRED " %s" RESET " [%d]\n",i,bgstack.processname[i],bgstack.process[i]);
						}
					}
				}
				else if(!strcmp(temporarycommand[0],"kjob"))
				{
					int tempc=atoi(temporarycommand[1]);
					if(bgstack.process[tempc]!=0)
					{
						printf("Killing process %d\t%d\n",tempc,atoi(temporarycommand[2]));
						int ret=kill(bgstack.process[tempc],atoi(temporarycommand[2]));
						for(i=tempc;i<bgstack.current-1;i++)
						{
							bgstack.process[i]=bgstack.process[i+1];
							strcpy(bgstack.processname[i],bgstack.processname[i+1]);	
						}
						bgstack.process[i]=0;
						bgstack.processname[i]='\0';
						bgstack.current--;
					}
					else
					{
						printf("Process does not exist\n");
					}

				}
				else if(!strcmp(temporarycommand[0],"overkill"))
				{
					for(i=1;i<bgstack.current;i++)
					{
						int ret=kill(bgstack.process[i],9);
					}
					bgstack.current=1;
				}
				else if(!strcmp(temporarycommand[0],"fg"))
				{
					int ff;
					tcsetpgrp(ff,getpgid(bgstack.process[atoi(temporarycommand[1])]));
				}
				else if(!strcmp(temporarycommand[0],"echo"))
				{
					//printf("In echo\n");
					int state=0;
					//printf("Full%s\n",maintoken+strlen(commandfull)+1);
					char* temp=maintoken+strlen(temporarycommand[0])+1;
					char echostring[buffer_space];
					int i;
					int j=0;
					//printf("%s\n",temp);
					//Checking for double and single quotes and sorting out accordingly.
					for(i=0;temp[i]!='\0';i++)
					{
					if(state==0&&(temp[i]==' '||temp[i]=='\t')&&(i==0||temp[i-1]==' '||temp[i-1]=='\t'))
					{
					continue;
					}
					else if(state==0)
					{

					if(temp[i]=='\"'||temp[i]=='\'')
					{
					state=1;
					}
					else{
					echostring[j++]=temp[i];
					}
					}
					else
					{
					if(temp[i]!='\''&&temp[i]!='\"')
					{
					echostring[j++]=temp[i];	
					}
					else{
					state=0;
					}
					}
					}
					echostring[j]='\0';
					//printf("%s",);
					if(!state)
					printf("%s\n",echostring);
					else{
					printf("Enter a valid string with correct quotations\n");
					}

				}
				else{

					process_id=fork();

					if(process_id==0)
					{
						if(red_1||red_2)
						{
							/*						printf("%s\n",outputfile);
													printf("%d%d%d\n",red_1,red_2,x); */
							//printf("%d\n",in);
							redirection(process_id,fd,inputfile,outputfile,temporarycommand,red_1,red_2);
						}
						if (in != 0&&red_1==0)
						{
							dup2(in, 0);
						}
					}
					//Background process	
					if(temporarycommand[x-1]!=NULL&&temporarycommand[x-1][0]=='&') 
					{
						//printf("Sending to background\n");
						if(process_id!=0)
						{

							bgstack.process[bgstack.current]=process_id;
							bgstack.processname[bgstack.current]=(char*)malloc(sizeof(char)*strlen(temporarycommand[0]));
							strcpy(bgstack.processname[bgstack.current++],temporarycommand[0]);
						}
						sendtobackground(process_id,temporarycommand,x);
					}
					else{ //Foreground process
						//fgsignalhandler();
						systemcalls(process_id,temporarycommand[0],temporarycommand,x);
					}

				}
			}




			//Checking if the command is echo


			if(!strcmp(commandfull,"exit")||!strcmp(commandfull,"quit"))
			{
				//Closing our shell
				printf("Bye\n");
				exit(0);
			}

			maintoken=strtok_r(NULL,";",&nextptr);

		}
	}while(flag);
	return 0;
}




void subproc (int pid, int in, int out, char** argument,int x)
{

	if (pid== 0)
	{
		//printf("In spawn\n");
		if (in != 0)
		{
			dup2 (in, 0);
			close (in);
		}

		if (out != 1)
		{
			dup2 (out, 1);
			close (out);
		}

		systemcalls(0,argument[0],argument,x);

		return;
	}
	//wait();
	return;
}

int redirection(int proc,int fd[2],char* input,char* output,char** argv,int inp,int out)
{

	if (proc == 0) {
		int fd1,fd0;
		if (inp) 
		{ //if '<' present
			fd0 = open(input, O_RDONLY, 0);
			if(fd0==-1)
			{
				if(errno==2)
					printf("The file " KRED "%s" RESET " does not exist." "\n",input);
				if(errno==13)
					printf("The file " KRED "%s" RESET " does not have read permissions." "\n",input);
				//printf("%d\n",errno);
				_exit(1);
			}
			else{
				dup2(fd0, STDIN_FILENO);
				close(fd0);
			}
		}

		if (out==1) 
		{ //if '>' present
			fd1 = creat(output, 0644);
			if(fd1==-1)
			{
				if(errno==2)
					printf("The file " KRED "%s" RESET " does not exist." "\n",output);
				if(errno==13)
					printf("The file " KRED "%s" RESET " does not have write permissions." "\n",output);
				//printf("%d\n",errno);
				_exit(1);
			}
			else{
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}
		}
		else if (out==2) 
		{ //if '>' present
			fd1 = open(output,O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
			if(fd1==-1)
			{
				if(errno==2)
					printf("The file " KRED "%s" RESET " does not exist." "\n",output);
				if(errno==13)
					printf("The file " KRED "%s" RESET " does not have write permissions." "\n",output);
				_exit(1);
			}
			else{
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}
		}
	}
}


void changedirectory(char* home_directory,char* argument[])
{
	if(argument[1]==NULL||strcmp(argument[1],"~")==0)
	{
			chdir(home_directory);
	}
	else
	{
		int return_val=chdir(argument[1]);
		if(return_val==-1)
			printf("Invalid directory.Check the directory.\n");
	}
}




void sendtobackground(int process_id,char** command,int itr)
{
	int i;
	if(process_id<0)
	{
		perror("Process was not created\n");
		_exit(-1);
	}
	else if(process_id>0)
	{
		printf("[%d]" KRED " %s" RESET " [%d]\n",bgstack.current-1,command[0],bgstack.process[bgstack.current-1]);
	}
	else
	{
		setpgid(0, 0);
		int ret;
		//printf("Process was created\n");

		//commandfull is the command and argument is the command including all arguments"
		if(!strcmp(command[0],"ls") || !strcmp(command[0],"grep"))
		{
			command[itr-1]=(char*)malloc(sizeof(char)*7);
			strcpy(command[itr-1],"--color");
		}
		command[itr]='\0';
		ret=execvp(command[0],command);
		
		if(ret<0)
		{
			perror("Incorrent command.Syntax error.Try again\n");
			_exit(-1);
		}


	}

}


int signalhandler()
{
	signal(SIGINT, sigproc);
	signal(SIGQUIT, quitprocess);
	signal (SIGCHLD, proc_exit);
	signal(SIGTSTP,sendtobg);
	signal(SIGCONT,foreg);

	return 0;
}

int fgsignalhandler()
{
	signal(SIGTSTP,sendtobg);

	return 0;
}

void sigproc(int a)
{
	signal(SIGINT, sigproc);
	/*printf("Cant Quit\n");*/
}
void quitprocess(int a)
{

}

void proc_exit(int a)
{
	int wstat,i;

	pid_t	pid;

	while (1) {
		pid = waitpid(-1,&wstat, WNOHANG);
		if (pid == 0)
			return;
		else if (pid == -1)
			return;
		else
			if(wstat==0)
			{
				for(i=1;i<bgstack.current;i++)
				{
					if(bgstack.process[i]==pid)
					{
						printf (KRED"%s" RESET " with pid " KBLU "%d" RESET " exited normally\n",bgstack.processname[i],pid);
						for(;i<bgstack.current-1;i++)
						{
							bgstack.process[i]=bgstack.process[i+1];
							strcpy(bgstack.processname[i],bgstack.processname[i+1]);	
						}
						bgstack.process[i]=0;
						bgstack.processname[i]='\0';
						bgstack.current--;
						break;
					}
				}
			}
			else{
				printf("Process exitted abnormally\n");
			}
	}
}

void foreg(int a)
{

}

void sendtobg(int a)
{
	printf("%d\n",getpid());
	a=1;
	bgstack.process[bgstack.current]=getpid();
	bgstack.processname[bgstack.current]=(char*)malloc(sizeof(char)*10);
	//for(a=1;a<=bgstack.current;a++)
	//{
	//	if(bgstack.process[a]==getpid())
		strcpy(bgstack.processname[bgstack.current++],"gedit");	
	//}
	setpgid(getpid(), 0);
	
}



void systemcalls(int process_id,char* commandfull,char* argument[],int itr)
{
	int status;
	if(process_id<0)
	{
		perror("Process was not created\n");
		_exit(-1);
	}
	else if(process_id==0)
	{
		current_processid=getpid();
		strcpy(current_processname,commandfull);
		int ret;
		//printf("Process was created\n");

		//commandfull is the command and argument is the command including all arguments"
		if(!strcmp(commandfull,"ls") || !strcmp(commandfull,"grep"))
		{
			argument[itr]=(char*)malloc(sizeof(char)*7);
			strcpy(argument[itr++],"--color");
		}
		argument[itr]='\0';
		for(ret=0;ret<itr;ret++)
		{

			//printf("%s\n",argument[ret]);
		}
		ret=execvp(commandfull,argument);
		
		if(ret<0)
		{
			perror("Incorrent command.Syntax error.Try again\n");
			_exit(-1);
		}

		_exit(0);

	}
	waitpid(process_id,&status,WUNTRACED);
	
}
