#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define DEF_PAGER	"/bin/more"
#define MAXLINE 4096

int
main(int argc, char *argv[])
{
	int n;
	int fd[2];
	pid_t pid;
	char *pager, *argv0;
	char line[MAXLINE];
	FILE *fp;

	if (argc != 2) {
		perror("usage: pager <pathname>");
		exit(1);
	}

	if ((fp = fopen(argv[1], "r")) == NULL) {
		char error_code[50] ;
		sprintf(error_code, "can't open %s", argv[1]);
		perror(error_code);
	}
	if (pipe(fd) < 0)
		perror("pipe error");

	if ((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid > 0) {
		close(fd[0]);
		
		/* parent copy argv[1] to pipe */
		while (fgets(line, MAXLINE, fp) != NULL) {
			n = strlen(line);
			if (write(fd[1], line, n) != n )
				perror("write error to pipe");
		}
		if (ferror(fp))
			perror("fgets error");

		close(fd[1]);

		if (waitpid(pid, NULL, 0) < 0)
			perror("waitpid error");
		exit(0);
	} else {
		close(fd[1]);
		if (fd[0] != STDIN_FILENO) {
			if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
				perror("dup2 error to stdin");
			close(fd[0]);
		}
		if ((pager = getenv("PAGER")) == NULL)
			pager = DEF_PAGER;
		if ((argv0 = strrchr(pager, '/')) != NULL)
			argv0++;
		else
			argv0 = pager;

		if (execl(pager, argv0, (char *)0) < 0) {
			char error_code[50];
			sprintf(error_code, "execl error for %s", pager);
			perror(error_code);
		}
	}
	exit(0);
} 
