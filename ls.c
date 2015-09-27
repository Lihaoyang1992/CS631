/*
 * simple-ls.c
 * Extremely low-power ls clone.
 * ./simple-ls .
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>


void
mode_to_letter(int mode, char str[])
{
	strcpy(str, "---------");

	if(S_ISDIR(mode)) str[0] = 'd';
	if(S_ISCHR(mode)) str[0] = 'c';
	if(S_ISBLK(mode)) str[0] = 'b';

	if(mode & S_IRUSR) str[1] = 'r';
	if(mode & S_IWUSR) str[2] = 'w';
	if(mode & S_IXUSR) str[3] = 'x';

	if(mode & S_IRUSR) str[4] = 'r';
	if(mode & S_IWUSR) str[5] = 'w';
	if(mode & S_IXUSR) str[6] = 'x';

	if(mode & S_IRUSR) str[7] = 'r';
	if(mode & S_IWUSR) str[8] = 'w';
	if(mode & S_IXUSR) str[9] = 'x';
}

char* 
uid_to_name(uid_t uid)
{
	struct passwd *	pw_ptr;
	static char	numstr[10];

	if ((pw_ptr = getpwuid(uid)) == NULL) {
		sprintf(numstr, "%d", uid);
		return numstr;
	}
	
	return pw_ptr->pw_name;
}

char*
gid_to_name(gid_t gid)
{
	struct group *	grp_ptr;
	static char numstr[10];

	if ((grp_ptr = getgrgid(gid)) == NULL) {
		sprintf(numstr, "%d", gid);
		return numstr;
	}
	
	return grp_ptr->gr_name;
}

void show_file_info(char* filename, struct stat * info_p) {
	char	modestr[11];

	mode_to_letter(info_p->st_mode, modestr);

	printf("%s", modestr);
	printf("%4d ", (int)info_p->st_nlink);
	printf("%-8s ", uid_to_name(info_p->st_uid));
	printf("%-8s ", gid_to_name(info_p->st_gid));
	printf("%8ld ", (long)info_p->st_size);
	printf("%.12s ", 4 + ctime(&info_p->st_mtime));
	printf("%s\n ", filename);
}

void do_stat(char* filename)
{
	struct	stat info;

	if (stat(filename, &info) == -1)
		perror(filename);
	else
		show_file_info(filename, &info);
}

void
do_ls (char dirname[])
{
	DIR*	dp;
	struct	dirent *dirp;

	if ((dp = opendir(dirname)) == NULL ) {
		fprintf(stderr, "can't open '%s'\n", dirname);
	} else {
		while ( (dirp = readdir(dp)) != NULL )
			do_stat(dirp->d_name);
		closedir(dp);
	}
}

int
main(int argc, char **argv)
{
	if (argc > 2) {
		fprintf(stderr, "usage: %s dir_name\n", argv[0]);
		exit(1);
	}

	if (argc == 1) {
		do_ls(".");
	} else {
		while (--argc) {
			printf("%s:\n", * ++argv);
			do_ls(*argv);
		}
	}

	return 0;
}