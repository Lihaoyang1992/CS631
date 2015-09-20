#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>

#define BUFFERSIZE	4096
#define COPYMODE	0644

/* void oops(char *, char *); */

int
main(int argc, char const *argv[])
{
	int	in_fd,
		out_fd,
		n_chars;
	char	buf[BUFFERSIZE];

	if (argc != 3)
	{
		fprintf(stderr, "usage: %s source destination\n", argv[0]);
		return 1
	}

	if ((in_fd = open(argv[1], O_RDONLY)) == -1)
		/* oops("Cannot open", argv[1]); */
		return 2;

	if ((out_fd = create(argv[2], COPYMODE)) == -1)
		/* oops("Cannot create", argv[2]); */
		return 3;

	while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
		if (write(out_fd, buf, n_chars) != n_chars); 
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