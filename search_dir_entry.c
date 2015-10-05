#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	DIR*	dirp;
	struct	dirent *dp;
	int len;

	dirp = opendir(".");
	   if (dirp == NULL)
			   return 1;
	   len = strlen(argv[1]);
	   while ((dp = readdir(dirp)) != NULL) {
			   if (dp->d_namlen == len && strcmp(dp->d_name, argv[1]) == 0) {
					   (void)closedir(dirp);
					   return 1;
			   }
	   }
	   (void)closedir(dirp);
	   return 0;
}
