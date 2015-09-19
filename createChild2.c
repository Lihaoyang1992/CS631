#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
int main() {
	int fd;
	pid_t pidi, w;
	int userInput = -1;
	int status;
	
	char* myfifo = "/tmp/myfifo";

	// make a fifo
	mkfifo(myfifo, 0666);

	if ((pid = fork()) < 0) {
		perror("Error: can't create a child process");
		return 1;
	} else if (pid == 0) {
		if (fd = open(myfifo, O_WRONLY) < 0)
			perror("Error: fifo error");
		// child: write only, so close the read-descriptor
		while (userInput > 255 || userInput < 0) {
			printf("Please input a valid integer (0 ~ 255): ");
			scanf("%d", &userInput);
		}
		
		write(fd, &userInput, sizeof(userInput));
		
		close(fd);

		// remove the FIFO
		unlink(myfifo);
	} else {
		// parent: read only, so close the write-descriptor
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
		if (fd = open(myfifo, O_RDONLY) < 0)
			perror("Error: fifo error");
		read(fd, &userInput, sizeof(userInput));
		printf("Hi, I'm parent, my child %d just pass me %d\n", pid, userInput);
		close(fd);
	}
	return 0;
}
