/*
 * simple-ls.c
 * Extremely low-power ls clone.
 * ./simple-ls .
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#define MAX_NAME_SIZE 256

int win_width = 80;

int
alpha_com(const void * a, const void * b)
{
	/* implement a compare function in alphabetic order */
	return 0;
}

void
mode_to_letter(int mode, char str[])
{
	strcpy(str, "----------");

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
	printf("%s\n", filename);
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
do_multi_columns (DIR* dirp, int max_name)
{
	struct winsize win;
	if (isatty(STDOUT_FILENO)) {
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == 0 && win.ws_col > 0)
			win_width = win.ws_col;
	}

	/* get the ceil of win_width/max_name */
	int columns = (win_width + max_name - 1)/max_name;

}

void
do_ls (char dirname[])
{
	int	i, max_name;
	DIR*	dp;
	struct	dirent *dirp;
	char	path_name[MAX_NAME_SIZE];
	char*	filenames;

	if ((dp = opendir(dirname)) == NULL ) {
		fprintf(stderr, "can't open '%s'\n", dirname);
	} else {
		for (i = 0, max_name = 0; (dirp = readdir(dp)) != NULL; i++ ) {
			sprintf(path_name, "%s/%s", dirname, dirp->d_name);
			do_stat(path_name);
			// filenames[i] = dirp->d_name;
			if (dirp->d_reclen > max_name)
				max_name = dirp->d_reclen;
		}
/*************************** marked ***************************/
	/*
		do_multi_columns(dp, max_name);
		filename = malloc(sizeof(char*) * --i);
		for (i = 0, max_name = 0; (dirp = readdir(dp)) != NULL; i++ ) {
			filenames[i] = dirp->d_name;
		}
		// sort files by names (alphabetic)
		qsort(filenames, --i, sizeof(char*), alpha_com);
		closedir(dp);
	*/
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
