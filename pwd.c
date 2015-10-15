#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

ino_t	get_inode(char *);
void	printpathto(ino_t);
void	inum_to_name(ino_t, char *);

int main()
{
	printpathto( get_inode(".") );	/* print path to here */
	putchar('\n');
	return 0;
}

void printpathto(ino_t this_inode)
{
	ino_t	my_inode;
	char	its_name[BUFSIZ];
	if (get_inode("..") != this_inode) {
		chdir("..");	/* enter parent dir */
		inum_to_name(this_inode, its_name);
		my_inode = get_inode(".");
		printpathto(my_inode);
		printf("/%s", its_name);
	}
}

void inum_to_name(ino_t inode_to_find, char * namebuf)
{
	DIR	*dir_ptr;
	struct dirent	*direntp;
	dir_ptr = opendir(".");
	if (dir_ptr == NULL) {
		perror(".");
		exit(1);
	}
	while ((direntp = readdir(dir_ptr)) != NULL)
		if (direntp->d_ino == inode_to_find) {
			strcpy(namebuf, direntp->d_name);
			closedir(dir_ptr);
			return;
		}
	fprintf(stderr, "error looking for inum %lld\n", inode_to_find);
	exit(1);
}

ino_t get_inode(char * fname)
{
	struct stat	info;
	if (stat(fname, &info) == -1) {
		fprintf(stderr, "Cannot stat");
		perror("fname");
		exit(1);
	}
	return info.st_ino;
}
