#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHECK_SUCCESS 0
#define CHECK_FAILURE 1
#define FIFO_PATH "/tmp/myfifo"
#define FIFO_MODE 0666

void usage(void)
{
	fprintf(stderr, "usage: ./primes <increasing positive integers>\n");
}
int check_arg(int argc, char** argv)
{
	if (argc < 2){
		usage();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
int wait_error_check(int pid, int primes_num) {
	int w, status;
	w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
	if (w == -1) {
		perror("Error: can't wait child");
	}
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == primes_num)
			printf("child %d exited correctly\n", getpid());
	} else if (WIFSIGNALED(status)) {
		printf("killed by sigal %d\n", WTERMSIG(status));
	} else if (WIFSTOPPED(status)) {
		printf("stopped by signal %d\n", WSTOPSIG(status));
	} else if (WIFCONTINUED(status)) {
		printf("continued\n");
	}
	else {
		return CHECK_FAILURE;
	}
	return CHECK_SUCCESS;
}
int find_and_submit_primes(int wfd, int bot, int top) {
	int flag = 0;
	int primes_num = 0;
	if (top < 2)
		return CHECK_SUCCESS;
	if (bot == 2)
	{
		if (write(wfd, &bot, sizeof(bot)) == -1) {
			perror("child can't write");
			return EXIT_FAILURE;
		}
		primes_num++;
	}
	if ((bot & 1) == 0)
		bot++;
	int i;
	for (i = bot; i <= top; i += 2)
	{
		flag = 0;
		int j;
		for (j = 2; j <= i / 2; j++)
		{
			if ((i % j) == 0)
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			if (write(wfd, &i, sizeof(i)) == -1) {
				perror("child can't write");
				return EXIT_FAILURE;
			}
			primes_num++;
		}
	}
	return primes_num;
}
int write_to_pipe(const int *wfd, int bot, int top)
{
	int primes_num;
	close(wfd[0]);
	primes_num = find_and_submit_primes(wfd[1], bot, top);
	return primes_num;
}
int write_to_fifo(const int wfd, int bot, int top)
{
	int primes_num;
	primes_num = find_and_submit_primes(wfd, bot, top);
	close(wfd);
	return primes_num;
}
int main(int argc, char* argv[])
{
	/* pipe used by odd child */
	int ctop_fd[argc / 2][2];
	/* fd and myfifo used by even child */
	int write_fd;
	char* myfifo = (char*)malloc(sizeof(FIFO_PATH));

	int i;
	int num;
	int read_fd, max_fd;
	pid_t child_pid[argc - 1];
	int prime;
	fd_set call_set;

	for (i = 0; i < argc - 1; ++i)
	{
		int bot = 2;
		int top = (atoi)(argv[i + 1]);
		if (i != 0)
			bot = (atoi)(argv[i]) + 1;

		if ((i & 1) == 0)
			/* odd child use pipe */
			pipe(ctop_fd[i / 2]);
		else
		{
			/* even child use fifo */
			sprintf(myfifo, "%s%2d", FIFO_PATH, i);
			if (mkfifo(myfifo, FIFO_MODE) == -1)
				perror("make fifo error");
		}
				
		if ((child_pid[i] = fork()) == -1)
		{
			perror("Fork error");
			return EXIT_FAILURE;
		}
		/* child to write primes*/
		else if (child_pid[i] == 0)
		{
			int primes_num = 0;
			printf("child %d: bottom=%d, top=%d\n", getpid(), bot, top);
			/* odd child write to pipe */
			if ((i & 1) == 0)
				primes_num = write_to_pipe(ctop_fd[i / 2], bot, top);
			/* even child write to fifo */
			else 
			{
				if ((write_fd = open(myfifo, O_WRONLY | O_NONBLOCK)) == -1)
					perror("open fifo error");
				primes_num = write_to_fifo(write_fd, bot, top);
				unlink(myfifo);
			}

			return primes_num;
		}
		/* parent receive primes */
		else {
			int primes_num = 0;
			/* odd child set read_fd, max_fd */
			if ((i & 1) == 0)
			{
				close(ctop_fd[i / 2][1]);
				read_fd = ctop_fd[i / 2][0];
				max_fd = ctop_fd[i / 2][0];
			}
			/* even child set read_fd, max_fd */
			else
			{
				if ((read_fd = open(myfifo, O_RDONLY)) == -1)
					perror("read fifo error");
				max_fd = read_fd;
			}

			/* parent use select to read */
			FD_ZERO(&call_set);
			FD_SET(read_fd, &call_set);
			num = select(max_fd + 1, &call_set, NULL, NULL, NULL);
			if (num == -1)
			{
				perror("select error");
				return EXIT_FAILURE;
			}
			if (FD_ISSET(read_fd, &call_set))
			{
				for (;;)
				{
					int read_status = read(read_fd, &prime, sizeof(prime));
					if (read_status == -1)
					{
						perror("Error: read error");
						return EXIT_FAILURE;
					}
					else if (read_status == 0) {
						break;
					}
					else {
						printf("%d is prime\n", prime);
						primes_num++;
					}
				}
			}
			if (wait_error_check(child_pid[i], primes_num))
			{
				perror("Error: can't get child wait information");
			}
		}
	}
	free(myfifo);
	return EXIT_SUCCESS;
}