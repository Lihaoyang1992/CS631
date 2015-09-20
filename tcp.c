#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<string.h>

#define	NAMESIZE	1024
#define	BUFFERSIZE	4096
#define	COPYMODE	0644

/* void oops(char *, char *); */
int	create_copy(int, char *);
int	normal_copy(int, char *);

int
main(int argc, char const *argv[])
{
	struct stat	file_stat;
	char 	filename[NAMESIZE];
	int	in_fd;

	if (argc != 3)
	{
		fprintf(stderr, "usage: %s source destination\n", argv[0]);
		return 1;
	}

	if ((in_fd = open(argv[1], O_RDONLY)) == -1)
		/* oops("Cannot open", argv[1]); */
		return 2;

	stat(argv[2], &file_stat);
	if (S_ISDIR(file_stat.st_mode))
	{
		/* argv[2] is a dir */
		strcpy(filename, argv[1]);
		strcpy(filename, argv[2]);
		return create_copy(in_fd, filename);
	}
	else
	{
		/* argv[2] is not a dir */
		strcpy(filename, argv[2]);
		return normal_copy(in_fd, filename);
	}

	return 0;
}
/*
 * create a file in dir and copy
 */
int
create_copy(int in_fd, char *filename)
{
	int	out_fd,
		n_chars;
	char	buf[BUFFERSIZE];

	if ((out_fd = creat(filename, COPYMODE)) == -1)
		/* oops("Cannot create", argv[2]); */
			return 3;

	while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
		if (write(out_fd, buf, n_chars) != n_chars)
		/*	oops("Write error to", argv[2]); */
			return 4;

	if (n_chars == -1)
		/* oops("read error from", argv[1]); */
		return 5;

	if (close(in_fd) == -1 || close(out_fd) == -1)
		/* oops("Error closing files", ""); */
		return 6;

	return 0;
}

/*
 * copy file
 */
int
normal_copy(int in_fd, char *filename)
{
	int	out_fd,
		n_chars;
	char	buf[BUFFERSIZE];

	if ((out_fd = open(filename, COPYMODE)) == -1)
		/* oops("Cannot create", argv[2]); */
			return 2;

	while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
		if (write(out_fd, buf, n_chars) != n_chars)
		/*	oops("Write error to", argv[2]); */
			return 4;

	if (n_chars == -1)
		/* oops("read error from", argv[1]); */
		return 5;

	if (close(in_fd) == -1 || close(out_fd) == -1)
		/* oops("Error closing files", ""); */
		return 6;

	return 0;
}