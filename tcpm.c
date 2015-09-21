/* 
 * tcpm.c
 * Homework #2 for Stevens CS-631
 * Advanced Programming in Unix System
 * Written by Haoyang Li
 * September 20, 2015
 */

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<sys/types.h>

#define	ILLARGUMENT	1
#define	FAILACCE	2
#define	FAILOPEN	3
#define	FAILCREATE	4
#define	FAILREAD	5
#define	FAILWRITE	6
#define	FAILCLOSE	7
#define	LSEEKFAIL	8
#define	MMAPFAIL	9

char * get_file(char *, char *);

int
main(int argc, char *argv[])
{
	struct stat	file_stat;
	char *	in_file = argv[1];
	char *	out_file;
	void *	file_read_buf;
	void *	file_write_buf;
	int	in_fd,
		out_fd;
	size_t	file_size;
	mode_t	file_mode;

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
		return FAILACCE;
	}
	/* get file size and mode */
	file_size = file_stat.st_size;
	file_mode = file_stat.st_mode;

	/* get the output file */
	out_file = get_file(argv[2], in_file);

	/* open in_file */
	if ((in_fd = open(in_file, O_RDONLY)) < 0) {
		perror("Open file fail!");
		return FAILOPEN;
	}

	/* open or create out_file */
	if ((out_fd = open(out_file, O_RDWR|O_CREAT|O_TRUNC, file_mode)) < 0)
	{
		perror("Create file fail!");
		return FAILCREATE;
	}

	/* lseek out_file */
	if (lseek(out_fd, file_size - 1, SEEK_SET) < 0)
	{
		perror("Lseek file fail!");
		return LSEEKFAIL;
	}

	/* truncate destination file before mmap it*/
	ftruncate(out_fd, file_size);

	/* map in_file to memory*/
	if ((file_read_buf = 
		mmap(0, file_size, PROT_READ, MAP_SHARED, in_fd, 0)) == MAP_FAILED)
	{
        perror("Mmap source file fail!");
        return MMAPFAIL;
    }

	/* map out_file to memory*/
	if ((file_write_buf = 
		mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0)) == MAP_FAILED)
	{
        perror("Mmap destination file fail!");
        return MMAPFAIL;
    }

    /* memory copy from in_file to out_file */
    memcpy(file_write_buf, file_read_buf, file_size);

	/* close error occurs */
	if (close(in_fd) < 0 || close(out_fd) < 0)
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
