#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main() {
	int fd;
	pid_t pid, w;
	int userInput = -1;
	int status;
	
	char* myfifo = "/tmp/myfifo";

	if ((pid = fork()) < 0) {
		perror("Error: can't create a child process");
		return 1;
	} else if (pid == 0) {
		// make a fifo
		mkfifo(myfifo, 0666);

		// child: write only, so close the read-descriptor
		while (userInput > 255 || userInput < 0) {
			printf("Please input a valid integer (0 ~ 255): ");
			scanf("%d", &userInput);
		}

		if ((fd = open(myfifo, O_WRONLY | O_NONBLOCK)) < 0)
        		perror("Error: fifo error");
		write(fd, &userInput, sizeof(userInput));
		close(fd);
		unlink(myfifo);
	} else {
		// make a fifo
		mkfifo(myfifo, 0666);
		// parent: read only, so close the write-descriptor
		if ((fd = open(myfifo, O_RDONLY)) < 0)
			perror("Error: fifo error");
		read(fd, &userInput, sizeof(userInput));
		w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1) {
			perror("Error: can't wait child");
		}
		if (WIFEXITED(status)) {
			printf("Child %d exited correctly\n", pid);
		} else if (WIFSIGNALED(status)) {
			printf("killed by sigal %d\n", WTERMSIG(status));
		} else if (WIFSTOPPED(status)) {
			printf("stopped by signal %d\n", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			printf("continued\n");
		}
		printf("Hi, I'm parent, my child %d just pass me %d\n", pid, userInput);
		close(fd);
	}
	return 0;
}
