/*  Nickolas Trevino
 *  COP4600
 *  Spring 2020
 *  Assignment 2 - mysh
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators
   
int main (int argc, char ** argv)
{
    char buf[MAX_BUFFER];                               // line buffer
    char * args[MAX_ARGS];                              // pointers to arg strings
    char ** arg;                                        // working pointer thru args
    char * prompt = "#" ;                               // shell prompt
    char cwd[1024+1];                                   // store current directory name
    DIR * dir;                                          // use to check if directory exists
    char recent_commands[MAX_ARGS][MAX_BUFFER];         // pointers to commands
    int number_commands = 0;                            // keep track of number of commands
    int i = 0;                                          // global counter variable
    __pid_t pid;                                        // global variable to store pids
    char *** arguments = malloc(3 * sizeof(char**));    // keep track of previous arguments for use with "lastcommands"
    int status;                                         // use to store status of program running in foreground
    int * pids = malloc(128 * sizeof(int));             // use for use in "exterminateall" command
    int num_processes = 0;                              // help keep track of the pids array


        while (!feof(stdin)) { 
            fputs (prompt, stdout);                // write prompt
            if (fgets (buf, MAX_BUFFER, stdin )) { // read a line
            
    /* store command in the commands array */
                
                strcpy(recent_commands[number_commands++], buf);

    /* tokenize the input into args array */

                arg = args;
                *arg++ = strtok(buf,SEPARATORS);   // tokenize input
                while ((*arg++ = strtok(NULL,SEPARATORS)));
                                                // last entry will be NULL 
    
                if (args[0]) {                     // if there's anything there

    /* check for internal/external command */
					// "clear" command
                    if (!strcmp(args[0],"clear")) {
                        system("clear");
                        continue;
                    }
		
					// changedir directory command
                    if(!strcmp(args[0], "changedir")) {
                        dir = opendir(args[1]); //check if the directory exists
                        if(dir) {
                            closedir(dir);
                            chdir(args[1]);
                        } else {
                            printf("Error, directory %s does not exist.\n", args[1]);
                        }
                        continue;
                    }

					// whereami command
                    if(!strcmp(args[0], "whereami")) {
                        getcwd(cwd, 1024);
                        fprintf(stdout, "%s\n", cwd);
                        continue;
                    }
					
					// lastcommands [-c] command
                    if(!strcmp(args[0], "lastcommands")) {
                        if(args[1] && !strcmp(args[1], "-c")) { // check for clear argument
                            number_commands = 0;
                        } else { 
                            for(i = 0; i < number_commands; i++) {
                                fprintf(stdout, "%s\n", recent_commands[i]);
                            }
                        }
                        continue;
                    }

					// run program [parameters] command
                    if(!strcmp(args[0], "run")) {
                        if(args[1]) {
                            arguments[0] = malloc(16 * sizeof(char*));
                            int argument_num = 2;
                            while(args[argument_num]) { // grab all the parameters
                                arguments[0][argument_num-2] = malloc(strlen(args[argument_num]) * sizeof(char));
                                strcpy(arguments[0][argument_num-2], args[argument_num]);
                                argument_num++;
                            }

                            arguments[0][argument_num] = NULL;

                            if(0 == (pid = fork())) {
                                if(-1 == execve(args[1], (char **)arguments[0], NULL)) {
                                    fprintf(stdout, "child process execve failed [%m]\n");
                                }
                            }
                            while(0 == waitpid(pid, &status, WNOHANG)) {
                            }

                            free(arguments[0]);
                        }

                        continue;
                    }

					// background program [parameters] command
                    if(!strcmp(args[0], "background")) {
                        if(args[1]) {
                            arguments[1] = malloc(16 * sizeof(char*));
                            int argument_num = 2;
                            while(args[argument_num]) { // grab all the parameters
                                arguments[1][argument_num-2] = malloc(strlen(args[argument_num]) * sizeof(char));
                                strcpy(arguments[1][argument_num-2], args[argument_num]);
                                argument_num++;
                            }

                            arguments[argument_num] = NULL;

                            if(0 == (pid = fork())) {
                                if(-1 == execve(args[1], (char **)arguments[1], NULL)) {
                                    fprintf(stdout, "child process execve failed [%m]");
                                }
                            }
                            fprintf(stdout, "PID %d\n", pid);
                            pids[num_processes++] = pid;

                            free(arguments[1]);
                            
                        }
                        continue;
                    }
					
					// exterminate PID command
                    if(!strcmp(args[0], "exterminate")) { 
                        if(args[1]) {
                            int pid = atoi(args[1]);
                            if(kill(pid, 0) == -1 && kill(pid, SIGKILL) == -1) {
                                fprintf(stdout, "Unable to stop proccess with PID %d.\n", pid);
                            } else {
                                fprintf(stdout, "Successfully killed process with PID %d.\n", pid);
                                for(i = 0; i < num_processes; i++) {
                                    if(pids[i] == pid && i != num_processes - 1) {
                                        pids[i] = num_processes;
                                    }
                                    num_processes--;
                                }
                            }
                        } else {
                            printf("Exterminate command requires a second parameter.\n");
                        }
                        continue;
                    }

					// quit command
                    if (!strcmp(args[0],"quit")){   
                        break;           
					}


                    arg = args;
                    while (*arg) fprintf(stdout,"%s ",*arg++);
                    fputs ("\n", stdout);
                }
            }
    }
    return 0; 
}