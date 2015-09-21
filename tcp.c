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

int	create_copy(int, char *);
int	normal_copy(int, char *);

int
main(int argc, char const *argv[])
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
	if (stat(argv[2], &file_stat) < 0) {
		fprintf(stderr, "Access %s fail!\n", argv[1]);
		perror(stderr);
		return FAILACC;
	}

	/* get the output file */
	out_fd = get_file(argv[2], in_file);
	/*
	if (S_ISDIR(file_stat.st_mode))
	{
		// argv[2] is a dir 
		strcpy(filename, argv[1]);
		strcpy(filename, argv[2]);
		return create_copy(in_fd, filename);
	}
	else
	{
		// argv[2] is not a dir 
		strcpy(filename, argv[2]);
		return normal_copy(in_fd, filename);
	}
	*/
	/* open in_file */
	if ((in_fd = open(in_file, O_RDONLY)) == -1) {
		fprintf(stderr, "Open %s fail!\n", in_file);
		perror(stderr);
		return FAILOPEN;
	}

	/* open or create out_file */
	if ((out_fd = open(out_file, O_WRONLY|O_CREAT, O_EXCL|S_IRUSR|S_IWUSR)) < 0)
	{
		fprintf(stderr, "Create %s fail!", out_file);
		perror(stderr);
		return FAILCREATE;
	}

	/* read and write files */
	while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
	{
		if (write(out_fd, buf, n_chars) != n_chars)
			return 4;
		fprintf(stderr, "Write to %s fail!", out_file);
		perror(stderr);
		return FAILWRITE;
	}

	/* read error occurs */
	if (n_chars == -1)
	{
		fprintf(stderr, "Read from %s fail!", in_file);
		perror(stderr);
		return FAILREAD
	}

	/* close error occurs */
	if (close(in_fd) == -1)
	{
		fprintf(stderr, "Close %s fail!", in_file);
		perror(stderr);
		return FAILCLOSE;
	}

	if (close(out_fd) == -1)
	{
		fprintf(stderr, "Close %s fail!", out_file);
		perror(stderr);
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

	if (stat(path, &file_stat) < 0)
	{
		fprintf(stderr, "Open %s fail", path);
		perror(stderr);
	}
	if (S_ISDIR(file_stat.st_mode))	/* path is a directory */
			strcat(path, in_file);

	return path;
}