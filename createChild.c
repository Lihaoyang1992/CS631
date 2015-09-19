#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
int main() {
	pid_t pid, w;
	int fd[2];
	int userInput = -1;
	int status;

	if (pipe(fd) < 0)
		perror("Error: pipe error");
	if ((pid = fork()) < 0) {
		perror("Error: can't create a child process");
		return 1;
	} else if (pid == 0) {
		// child: write only, so close the read-descriptor
		close(fd[0]);
		while (userInput > 255 || userInput < 0) {
			printf("Please input a valid integer (0 ~ 255): ");
			scanf("%d", &userInput);
		}
		
		write(fd[1], &userInput, sizeof(userInput));
		
		close(fd[1]);
	} else {
		// parent: read only, so close the write-descriptor
		close(fd[1]);
		w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1) {
			perror("Error: can't wait child");
		}
		if (WIFEXITED(status)) {
			printf("Child exited status = %d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("killed by sigal %d\n", WTERMSIG(status));
		} else if (WIFSTOPPED(status)) {
			printf("stopped by signal %d\n", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			printf("continued\n");
		}
		read(fd[0], &userInput, sizeof(userInput));
		printf("Hi, I'm parent, my child %d just pass me %d\n", pid, userInput);
		close(fd[0]);
	}
	return 0;
}
