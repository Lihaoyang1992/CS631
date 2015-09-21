#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<string.h>

#define	NAMESIZE	1024
#define	BUFFERSIZE	4096
#define	COPYMODE	0644
#define	ILLARGUMENT	1
#define	FAILACC	2
#define	FAILOPEN	3
#define	FAILCREATE	4
#define	FAILREAD	5
#define	FAILWRITE	6
#define	FAILCLOSE	7

char * get_file(char *, char *);

int
main(int argc, char *argv[])
{
	struct stat	file_stat;
	char	buf[BUFFERSIZE];
	char *	in_file = argv[1];
	char *	out_file;
	int	n_chars,
		in_fd,
		out_fd;

	/* check arguments */
	if (argc != 3)
	{
		perror("Usage: tcp [file source] [file destination]");
		return ILLARGUMENT;
	}

	/* check status */
	if (stat(in_file, &file_stat) < 0)
	{
		perror("Access file fail!");
		return FAILACC;
	}

	/* get the output file */
	out_file = get_file(argv[2], in_file);

	/* open in_file */
	if ((in_fd = open(in_file, O_RDONLY)) == -1) {
		perror("Open file fail!");
		return FAILOPEN;
	}

	/* open or create out_file */
	if ((out_fd = open(out_file, O_WRONLY|O_CREAT, O_EXCL|S_IRUSR|S_IWUSR)) < 0)
	{
		perror("Create file fail!");
		return FAILCREATE;
	}

	/* read and write files */
	while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
	{
		if (write(out_fd, buf, n_chars) != n_chars)
		{
			perror("Write to file fail!");
			return FAILWRITE;
		}
	}

	/* read error occurs */
	if (n_chars == -1)
	{
		perror("Read from file fail!");
		return FAILREAD;
	}

	/* close error occurs */
	if (close(in_fd) == -1 || close(out_fd) == -1)
	{
		perror("Close file fail!");
		return FAILCLOSE;
	}

	return 0;
}

/*
 * get the file path and name
 */
char *
get_file(char *path, char * in_file)
{
	struct stat	file_stat;

	fopen(path, "a");

	/* check status */
	if (stat(path, &file_stat) < 0)
		perror("Open file fail!");

	/* check whether path is a directory */
	if (S_ISDIR(file_stat.st_mode))
			strcat(path, in_file);

	return path;
}