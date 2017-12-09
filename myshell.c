#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#define INPUTBUFF 1024
#define DF "asdfi3ygjp14y3j.txt"
#define DF2 "asdfi3ygjp14y3ja.txt"

char* children[4096];

void sigint_handler(int signum) {
	//kill child processes
	printf("\n   Killing child processes\n");
	char *itt;
	int x;
	itt = strtok(children, ":");
	while(itt != NULL) {
		printf("   Killing process %s\n", itt);
		int processid = strtol(itt, NULL, 10);
		kill(processid, SIGINT);
		waitpid(processid, x, WNOHANG);
		itt = strtok(NULL, ":");
	}
}

void exec_cmd(char ** args, char * iofile, int iotype, int pipetype, int pff, int ampersand) {

	pid_t pi;
	int *stat;

	if((pi = fork()) < 0) {
		printf("Fork Error\n");
	} else if (pi == 0) {
		//child
		//I/O redirection
		FILE * ioa;
		int iofd = -1;

		//redirect output
		if (iotype == 1 || iotype == 4) {
			ioa = fopen(iofile, "w");
			if (ioa == NULL) {
				printf("Err preparing output file\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(iofile, O_WRONLY | O_APPEND);
				if (dup2(iofd, 1) < 0) {
					printf("dup2 error\n");
					exit(1);
				} else {
					close(iofd);
				}
			}
		}

		//redirect input
		if (iotype == 2) {
			ioa = fopen(iofile, "r");
			if (ioa == NULL) {
				printf("Err reading from input file\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(iofile, O_RDONLY);
				if (dup2(iofd, 0) < 0) {
					printf("dup2 err\n");
				} else {
					close(iofd);
				}
			}
		}

		//redirect err
		if (iotype == 3 || iotype == 4) {
			ioa = fopen(iofile, "w");
			if (ioa == NULL) {
				printf("Err reading from input file\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(iofile, O_WRONLY | O_APPEND);
				if (dup2(iofd, 2) < 0) {
					printf("dup2 err\n");
				} else {
					close(iofd);
				}
			}
		}

		//pipe
		char * rf = DF;
		char * wf = DF2;
		if (pff) {
			rf = DF2;
			wf = DF;
		}
		if (pipetype == 1) {
			ioa = fopen(wf, "w");
			if (ioa == NULL) {
				printf("Piping error\n");
				return;
			} else {
				fclose(ioa);
				ioa = fopen(rf, "w");
				fclose(ioa);
				iofd = open(wf, O_WRONLY | O_APPEND);
				if (dup2(iofd, 1) < 0) {
					printf("rip1\n");
				} else {
					close (iofd);
				}
			}
		} else if (pipetype == 2) {
			ioa = fopen(rf, "r");
			if (ioa == NULL) {
				printf("Piping error\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(rf, O_RDONLY);
				if (dup2(iofd, 0) < 0) {
					printf("rip2\n");
				} else {
					close (iofd);
				}
			}
		} else if (pipetype == 3) {
			ioa = fopen(rf, "r");
			if (ioa == NULL) {
				printf("Piping error\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(rf, O_RDONLY);
				if (dup2(iofd, 0) < 0) {
					printf("rip3a\n");
				} else {
					close (iofd);
				}
			}
			ioa = fopen(wf, "w");
			if (ioa == NULL) {
				printf("Piping error\n");
				return;
			} else {
				fclose(ioa);
				iofd = open(wf, O_WRONLY | O_APPEND);
				if (dup2(iofd, 1) < 0) {
					printf("rip3b\n");
				} else {
					close (iofd);
				}
			}
		}

		//Execute program
		execvp(args[0], args);	//may need to add NULL at the end
		printf("Error executing cmd\n");
	} else {
		//parent
		char * procid[64];
		sprintf(procid, "%d", pi);
		strcat(children, procid);
		strcat(children, ":");

		if (ampersand) {
			waitpid(pi, stat, WNOHANG);
		} else {
			waitpid(pi, stat, 0);
		}

		char* loc = strstr(children, procid);
		if (loc != NULL) {
			strcpy(loc, loc + strlen(procid) + 1);
		}
	}
}


int get_io_file(char *pointer, char *ioname) {
	pointer = strtok(NULL, " ");
	strcpy(ioname, pointer);
	if (ioname == NULL) {
		printf("Error: no input file specified\n");
		return 0;
	} else {
		return 1;
	}
}

void cleanstuff() {
	int as1;
	int as2;
	int *res;
	as1 = fork();
	if (as1) {
		execlp("rm", "rm", DF, NULL);
	} else {
		as2 = fork();
		if (as2) {
			execlp("rm", "rm", DF2, NULL);
		}
		waitpid(as1, res, 0);
		waitpid(as2, res, 0);
	}
}

int main (int arg, int argc, char **argv) {

	int shelling = 1;
	char * poi;
	char holding[INPUTBUFF * 4];
	char input[INPUTBUFF];
	int c = 0;

	struct sigaction ctrlc;
	ctrlc.sa_sigaction = sigint_handler;

	sigaction(SIGINT, &ctrlc, NULL);

	while (shelling) {
		//get input from terminal or file
		input[0] = '\0';
		holding[0] = '\0';
		char nl = '\n';
		char *cp;

		if (isatty(fileno(stdin))) {
			printf("myshell> ");
			if (feof(stdin)) {
				if (c) {
					cleanstuff();
				}

				printf("\n   Exiting myshell\n");
				shelling = 0;
				input[0] = '\0';
				exit(0);
			} else {
				fgets(input, INPUTBUFF, stdin);
				input[strcspn(input, "\n")] = '\0';
			}
		} else {
			while (fgets(input, sizeof(input), stdin) != NULL) {
				strcat(holding, input);
			}
			input[0] = '\0';
			poi = strtok(holding, "\n");
				
			while (poi != NULL) {
				strcat(input, poi);
				strcat(input, " ; ");
				poi = strtok(NULL, "\n");
			}
			shelling = 0;
		}

		//parse and execute input
		char cheque = ';';
		char leftover[INPUTBUFF];
		int left = 0;

		int cmdready = 1;
		int cmdorarg = 0;
		int stopop = 0;
		//FILE * iofile;
		char ioname[128];
		ioname[0] = '\0';
		int iotyp = 0;		//0 no io op, 1 redirect out, 2 redirect in, 3 redirect err, 4 redirect out and err
		int * ifd; //pointer to the input half of pipe
		int * ofd; //pointer to the output half of pipe
		int nexthasin = 0;
		int currhasin = 0;
		int pipetyp = 0;	//0 no piping, 1 output->pipe, 2 pipe->input, 3 both
		int pone[2], ptwo[2];
		int flip = 0;
		int amp = 0;	//presence of & symbol

		char *exeargs[32];

		int asdf;
		for (asdf = 0; asdf < 16; asdf ++) {
			exeargs[asdf] = malloc(sizeof(char) * 128 + 1);
		}

		char * pointer;
		pointer = strtok(input, " ");
		while(pointer != NULL && !stopop) {

			cmdready = 1;
			iotyp = 0;
			pipetyp = 0;
			stopop = 0;
			amp = 0;

			if ((cp = strchr(pointer, cheque)) != NULL) {
				//end of current sequence, execute
				char stuff[256];
				int count = 0;
				while(count < 256) {
					if (pointer[count] != ';') {
						stuff[count] = pointer[count];
						count ++;
					} else {
						stuff[count] = '\0';
						if (count > 0) {
							strcpy(exeargs[cmdorarg], stuff);
							cmdorarg ++;
						}
						if (pointer[1] != '\0') {
							cp ++;
							strcpy(leftover, cp);
						} else {
							leftover[0] = '\0';
						}
						count = 256;
					}
				}

			} else if (strcmp("1>", pointer) == 0 || strcmp(">", pointer) == 0) {
				//redirect stdout of previous cmd to file specified after
				if (get_io_file(pointer, ioname)) {
					iotyp = 1;
				} else {
					printf("Error with output file, aborting op\n");
					stopop = 1;
				}
			} else if (strcmp("2>", pointer) == 0) {
				//redirect stderr of previous cmd to file specified after
				if (get_io_file(pointer, ioname)) {
					iotyp = 3;
				} else {
					printf("Error with output file, aborting op\n");
					stopop = 1;
				}
			} else if (strcmp("&>", pointer) == 0) {
				//redirect both stdout and stderr of previous cmd to file specified after
				if (get_io_file(pointer, ioname)) {
					iotyp = 4;
				} else {
					printf("Error with output file, aborting op\n");
					stopop = 1;
				}
			} else if (strcmp("<", pointer) == 0) {
				//redirect stdin of previous cmd to file specified after
				if (get_io_file(pointer, ioname)) {
					iotyp = 2;
				} else {
					printf("Error with input file, aborting op\n");
					stopop = 1;
				}
			} else if (strcmp("|", pointer) == 0) {
				//pipe output of previous cmd to cmd after
				nexthasin = 1;
				pipetyp = 1;
				if (!c) {
					c = 1;
				}
			} else if (strcmp("&", pointer) == 0) {
				//run previous cmd in the background
				amp = 1;
			} else {
				//part of a cmd, either the function or an arg
				cmdready = 0;
				strcpy(exeargs[cmdorarg], pointer);
				cmdorarg ++;
			}

			pointer = strtok(NULL, " ");
			if (cmdready || (pointer == NULL && !stopop)) {
				//current cmd has all parameters set, time to execute
				exeargs[cmdorarg] = '\0';
				if (currhasin) {
					currhasin = 0;
					if (pipetyp == 1) {
						pipetyp = 3;
					} else {
						pipetyp = 2;
					}
				}
				if (nexthasin) {
					nexthasin = 0;
					currhasin = 1;
				}
				exec_cmd(exeargs, ioname, iotyp, pipetyp, flip, amp);
				if (flip) {
					flip = 0;
				} else {
					flip = 1;
				}

				ifd = ofd;
				ofd = -1;

				//reset vars in case more cmds in the same line
				left = 0;
				cmdorarg = 0;
				int asdf;
				for (asdf = 0; asdf < 16; asdf ++) {
					exeargs[asdf] = malloc(sizeof(char) * 128 + 1);
				}

				//handle leftover stuff if last token had ';' in it
				if (leftover[0] != '\0') {
					strcpy(exeargs[0], leftover);
					cmdorarg ++;
					if (pointer == NULL) {
						exeargs[cmdorarg] = NULL;
						exec_cmd(exeargs, ioname, 0, 0, 0, 0);
					}
				}
			}
		}
	}
	if (c) {
			cleanstuff();
	}

	exit(0);
}