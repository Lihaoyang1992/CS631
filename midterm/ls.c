/*
 *	Stevens CS-631 Midterm
 *	Created by Haoyang Li (hli40@stevens.edu)
 */

//#include <bsd/stdlib.h>
//#include <bsd/string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <fts.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "l.h"


#ifndef S_IFWHT
	#define S_IFWHT 0170000
#endif


// define flags
int Aflag;
int aflag;
int cflag;
int dflag;
int Fflag;
int fflag;
int Rflag;
int iflag;
int lflag;
int nflag;
int sflag;
int Sflag;
int qflag;
int wflag;
int tflag;
int cflag;
int uflag;
int xflag;
int Cflag;
int kflag;
int hflag;
int rflag;
int oneflag;

int outputnum;		/*all output number including error message*/
int outputentnum;	/*all valid output number*/

char path[PATH_MAX];
			
// for -k/-h
int divide_term = 1;
char token[6] = {'B', 'K', 'M', 'G', 'T', 'P'};

// used for output format (-ln)
ino_t max_st_ino;
nlink_t max_st_nlink;
blkcnt_t max_st_blocks;
off_t max_st_size;
int max_major_num;
int max_minor_num;

int max_un_len;
int max_gn_len;
int max_ln_len;
int max_sz_len;
int max_major_len;
int max_minor_len;
int max_in_len;	
int max_blk_len;
int max_fn_len;
int totalblknum;

int win_width;

int has_blk_or_char;

// output format setting
blksize_t blocksize;
int totalblknum;

void 
usage()
{
	fprintf(stderr, "%s [ −AacdFfhiklnqRrSstuw1Cx ][file ...]\n", getprogname());
	exit(0);
}


void print_name(const char* name)
{
	const char *p = name;
	if (qflag)
	{
		while( (*p) != '\0') {
			if (isgraph(*p) != 0) {
				(void)putchar(*p);
			} else {
				(void)putchar('?');
			}
			p++;
		}
	} else {
		printf("%s", name);
	}
}


/*
 * output each entries based on the configuration
 */
void cook_output(FNODE *node)
{
	char c;				/*used for -F*/
	char modestr[12];
	struct tm *tmptr;
	ssize_t npath;
	FTSENT* ent = node->ftsentryptr;
	struct stat *statp = ent->fts_statp;
	mode_t mode = statp->st_mode;
	dev_t rdev = statp->st_rdev;
	// MON DD HH:MM
	char outtime[14];
	char buf[PATH_MAX];
	int blklen;
	time_t output_time;

	if (iflag) {
		(void)printf("%*ld ", max_in_len, statp->st_ino);
	}

	if (sflag) {
		blklen = ceil((double)statp->st_blocks*512 / (double)blocksize);
		(void)printf("%*d ", max_blk_len, blklen);
	}

	if (lflag || nflag) {
		strmode(mode ,modestr);
		modestr[10] = '\0';
		(void)printf("%s ", modestr);

		(void)printf("%*ld ", max_ln_len, statp->st_nlink);

		(void)printf("%*s ", -max_un_len, node->username);

		(void)printf("%*s ", -max_gn_len, node->grpname);

		//output size or major/minor
		if (S_ISBLK(mode) || S_ISCHR(mode)) {
			if (max_sz_len > max_major_len + max_minor_len + 2) {
				max_major_len = max_sz_len - 2 - max_minor_len;
			}

			(void)printf("%*d, %*d ", max_major_len, major(rdev), max_minor_len, minor(rdev));

		} else {
			if (kflag) {
				(void)printf("%*llu ", max_sz_len, (long long unsigned int)ceil((double)statp->st_size/divide_term));
			
			} else if (hflag) {
				
				double ret = statp->st_size;
				int step = 0;
				while (ret > 1024 && step < 6) {
					ret = ret/1024;
					step++;
				}

				if (step == 0) {

					(void)printf("%*d%c ", max_sz_len-1, (int)ret, token[0]);
				} else {
					(void)printf("%*.1f%c ", max_sz_len-1, ret, token[step]);
				}

			} else {
				(void)printf("%*ld ", max_sz_len, statp->st_size);
			}
		}


		if (cflag) {
			output_time = statp->st_ctime;
		} else if (uflag) {
			output_time = statp->st_atime;
		} else {
			output_time = statp->st_mtime;
		}

		tmptr = localtime(&output_time);
		strftime(outtime, 14, "%b %e %H:%M", tmptr);
		(void)printf("%s ", outtime);
	}

	print_name(ent->fts_name);

	if (Fflag) {
		c = '\0';
		switch(mode & S_IFMT) {
			case S_IFDIR:
				c = '/';
				break;
			case S_IFLNK:
				if (lflag||nflag) {
					break;
				}
				c = '@';
				break;
			case S_IFSOCK:
				c = '=';
				break;
			case S_IFIFO:
				c = '|';
				break;
			case S_IFWHT:
				c = '%';
				break;
			case S_IFREG:
				if (mode&(S_IXUSR | S_IXGRP | S_IXOTH)) {
					c = '*';
				}
				break;
		}
		(void)putchar(c);
	}

	//special case for symlink
	if (S_ISLNK(mode) && (lflag || nflag)) {
		if (snprintf(path, PATH_MAX, "%s/%s", ent->fts_path, ent->fts_name) < 0) {
			fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		npath = readlink(path, buf, PATH_MAX);
		buf[npath] = '\0';
		(void)printf(" -> %s ", buf);
	}

	(void)putchar('\n');
}


void print_col(FNODEP ent, int width)
{
	int blklen;
	char c;
	struct stat *statp = ent->ftsentryptr->fts_statp;
	mode_t mode = statp->st_mode;
	char *name = ent->ftsentryptr->fts_name;
	char *p = name;
	

	if (iflag) {
		(void)printf("%*ld ", max_in_len ,statp->st_ino);
	}

	if (sflag) {
		blklen = ceil((double)statp->st_blocks*512 / (double)blocksize);
		(void)printf("%*d ", max_blk_len ,blklen);
	}

	if (Fflag) {
		c = ' ';
		switch(mode & S_IFMT) {
			case S_IFDIR:
				c = '/';
				break;
			case S_IFLNK:
				if (lflag) {
					break;
				}
				c = '@';
				break;
			case S_IFSOCK:
				c = '=';
				break;
			case S_IFIFO:
				c = '|';
				break;
			case S_IFWHT:
				c = '%';
				break;
			case S_IFREG:
				if (mode&(S_IXUSR | S_IXGRP | S_IXOTH)) {
					c = '*';
				}
				break;
		}
	}

	while( (*p) != '\0') {
		if (qflag) {
			if (isgraph(*p) != 0) {
				(void)putchar(*p);
			} else {
				(void)putchar('?');
			}
		} else {
			(void)putchar(*p);
		}
		p++;
	}
	if (Fflag) {
		putchar(c);
	}
	printf("%*s", (int)(-width + (p - name)),"");
	printf("  ");
}


int compare_num (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


void cook_multi_cols()
{
	int i = 0;
	FNODEP p;
	int col = 0;
	int row = 0;
	int count = cldlist_get_count();
	int *arr = (int *)calloc(count, sizeof(int));
	int *arr_copy = (int *)calloc(count, sizeof(int));
	int *name_reserve_arr = (int *)calloc(count, sizeof(int));
	//test for -c
	int *visit_order = (int *)calloc(count, sizeof(int));

	cldlist_reset_ptr();

	while((p = cldlist_get_next()) != NULL) {
		int len = strlen(p->ftsentryptr->fts_name);

		if (iflag) {
			arr[i] += max_in_len + 1;
		}

		if (sflag) {
			arr[i] += max_blk_len + 1;
		}

		arr[i] += len;

		if (Fflag) {
			arr[i]++;
		}
		arr[i] += 2;

		arr_copy[i] = arr[i];

		name_reserve_arr[i] = len;

		i++;
	}

	qsort(arr, count, sizeof(int), compare_num);

	int min = count;
	int j;
	for (i = 0; i < count; ++i) {
		int w = 0;
		int overflow = 0;
		int gap = 0;

		for(j = i; j < count; ++j) {
			w += arr[j];
			if (w > win_width) {
				j--;
				overflow = 1;
				break;
			}
		}

		if (overflow) {
			gap = j-i+1;
			if (gap < min) {
				min = gap;
			}
		}
	}
	
	col = min;
	row = ceil((double)count/(double)col);
	col = ceil((double)count/(double)row);

	if (Cflag) {
		for (i = 0; i < count; ++i) {
			int c = i/row; 
			int r = i%row;
			int index = r*col + c;
			visit_order[index] = i;
		}
	} else {
		for (i = 0; i < count; ++i) {
			visit_order[i] = i;
		}
	}

	int *col_width = (int *)calloc(col, sizeof(int));

	for (i = 0; i < row; ++i) {
		for (j = 0; j < col; ++j) {
			int index = i*col + j;
			if (index < count) {
				if (name_reserve_arr[visit_order[index]] > col_width[j]) {
					col_width[j] = name_reserve_arr[visit_order[index]];
				}
			}
		}
	}

	for (i = 0; i < row; ++i) {
		for (j = 0; j < col; ++j) {
			int index = i*col + j;
			if (index < count) {
				FNODEP fnode = cldlist_get(visit_order[index]);
				if (fnode != NULL) {
					print_col(fnode, col_width[j]);
				}
			}
		}
		putchar('\n');
	}
}


void traverse_children(FTSENT* parent)
{

	FNODE* cur;

	/*handle root level children*/
	if (parent == NULL) {
		if (cldlist_get_count() == 0)
			return;
		/*for the non-directory files in argv list*/
		cldlist_reset_ptr();
		while( (cur = cldlist_get_next()) != NULL ) {
			cook_output(cur);
		}
		return;
	}

	/*seprate a line and display the path name*/
	if (outputentnum > 0) {
		(void)putchar('\n');
	}

	if (outputnum > 1 || Rflag) {
		(void)printf("%s:\n", parent->fts_path);
	}

	outputentnum++;
	outputnum++;

	if ( lflag || nflag || (sflag && isatty(STDOUT_FILENO))) {
		(void)printf("total %d\n", totalblknum);
	}
	
	if ( lflag || nflag || oneflag) {
		cldlist_reset_ptr();
		while( (cur = cldlist_get_next()) != NULL ) {
			cook_output(cur);
		}
		return;
	}

	if (xflag || Cflag) {
		cook_multi_cols();
		return;
	}
}


/*
 * process each entry in the children linked list
 * get the format setting for the output
 */
void cook_entry(FTSENT* ent)
{
	char uname[MAX_USER_NAME], gname[MAX_GROUP_NAME];
	int unlen, gnlen;
	struct stat *statp = ent->fts_statp;
	mode_t mode = statp->st_mode;
	dev_t rdev = statp->st_rdev;
	struct passwd *pwdp;
	struct group *grpp;

	if (strlen(ent->fts_name) > max_fn_len) {
		max_fn_len = strlen(ent->fts_name);
	}

	if (iflag) {
		if (statp->st_ino > max_st_ino) {
			max_st_ino = statp->st_ino;
		}
	}

	if (sflag || lflag || nflag) {
		if (statp->st_blocks > max_st_blocks) {
			max_st_blocks = statp->st_blocks;
		}
		totalblknum += statp->st_blocks;
	}

	if (lflag || nflag) {
		/*
		 * handle the special case for CHR/BLK 
		 */
		if (S_ISBLK(mode) || S_ISCHR(mode)) {
			has_blk_or_char = 1;

			int major_num = major(rdev);

			int minor_num = minor(rdev);

			if (major_num > max_major_num) {
				max_major_num = major_num;
			}

			if (minor_num > max_minor_num) {
				max_minor_num = minor_num;
			}

		} else if (statp->st_size > max_st_size) {
			max_st_size = statp->st_size;
		}
		
		if (statp->st_nlink > max_st_nlink) {
			max_st_nlink = statp->st_nlink;
		}

		//get user name of the file
		errno = 0;
		pwdp = getpwuid(statp->st_uid);
		if (pwdp == NULL && errno != 0) {
			(void)fprintf(stderr, "%s: getpwuid error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (pwdp == NULL || pwdp->pw_name == NULL || nflag) {
			if ((unlen = snprintf(uname, 20, "%u", statp->st_uid)) < 0) {
				(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
				exit(EXIT_FAILURE);
			}

		} else if ((unlen = snprintf(uname, 20, "%s", pwdp->pw_name)) < 0) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (unlen > max_un_len) {
			max_un_len = unlen;
		}

		//get group name of the file
		errno = 0;
		grpp = getgrgid(ent->fts_statp->st_gid);
		if (grpp == NULL && errno != 0) {
			(void)fprintf(stderr, "%s: getgrgid error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (grpp == NULL || grpp->gr_name == NULL || nflag) {
			
			if ((gnlen = snprintf(gname, 20, "%u", statp->st_gid)) < 0) {
				(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
				exit(EXIT_FAILURE);
			}

		} else if ((gnlen = snprintf(gname, 20, "%s", grpp->gr_name)) < 0) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (gnlen > max_gn_len) {
			max_gn_len = gnlen;
		}
	}

	cldlist_push_back(ent, uname, gname);
}



/*
 * use the information after iterating all the entries in the list
 * get all field max width and set the output configuration
 */
void set_output_config()
{
	if (iflag) {
		if ( (max_in_len = snprintf(NULL, 0, "%ld", max_st_ino)) < 0) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (sflag || lflag || nflag) {
		if ( (max_blk_len = snprintf(NULL, 0, "%ld", max_st_blocks)) < 0 ) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}
		totalblknum = ceil( (double)totalblknum*512/(double)blocksize);
	}

	if (lflag || nflag) {
		
		if ( (max_major_len = snprintf(NULL, 0, "%d", max_major_num)) < 0 ) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ( (max_minor_len = snprintf(NULL, 0, "%d", max_minor_num)) < 0 ) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (kflag) {
			if ( (max_sz_len = snprintf(NULL, 0, "%llu", (long long unsigned int)ceil((double)max_st_size/divide_term))) < 0 ) {
				(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
				exit(EXIT_FAILURE);
			}
		} else if (hflag) {

			double ret = max_st_size;
			int step = 0;
			while(ret>1024 && step < 6) {
				ret = ret/1024;
				step++;
			}

			if (step == 0) {
				if ( (max_sz_len = snprintf(NULL, 0, "%d%c", (int)ret, token[step])) < 0 ) {
					(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
					exit(EXIT_FAILURE);
				}
			} else {
				if ( (max_sz_len = snprintf(NULL, 0, "%.1f%c", ret, token[step])) < 0 ) {
					(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			
		} else if ( (max_sz_len = snprintf(NULL, 0, "%ld", max_st_size)) < 0 ) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ( has_blk_or_char == 1 && max_major_len+max_minor_len+2 > max_sz_len ) {
			max_sz_len = max_major_len+max_minor_len+2;
		}


		if ( (max_ln_len =  snprintf(NULL, 0, "%ld", max_st_nlink)) < 0 ) {
			(void)fprintf(stderr, "%s: snprintf error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}


void reset_output_config()
{
	max_st_ino = 0;
	max_st_nlink = 0;
	max_st_blocks = 0;
	max_st_size = 0;
	max_major_num = 0;
	max_minor_num = 0;

	max_un_len = 0;
	max_gn_len = 0;
	max_ln_len = 0;
	max_sz_len = 0;
	max_major_len = 0;
	max_minor_len = 0;
	max_in_len = 0;
	max_blk_len = 0;
	max_fn_len = 0;

	has_blk_or_char = 0;
}


/*
 * iterate all the entries in the children
 * set the output configuration
 */
void config_output(FTSENT* children)
{
	FTSENT *p;

	reset_output_config();

	if (children == NULL) {
		return;
	}

	cldlist_reset();

	for (p = children; p != NULL; p = p->fts_link) {
		if (p->fts_info == FTS_ERR || p->fts_info == FTS_NS) {
			warnx("can't access %s: %s",p->fts_name, strerror(p->fts_errno));
			continue;
		}

		if (!Aflag && !aflag && p->fts_name[0] == '.') {
			continue;
		}

		//output
		cook_entry(p);
	}

	set_output_config();
}


int sortbyname(const FTSENT **obj1, const FTSENT **obj2)
{
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;

	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	return strcmp( (*obj1)->fts_name, (*obj2)->fts_name);
}

int sortbysize(const FTSENT **obj1, const FTSENT **obj2)
{
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;
	struct stat *obj1statp = (*obj1)->fts_statp;
	struct stat *obj2statp = (*obj2)->fts_statp;
	
	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	if (obj1fts_info == FTS_NS || obj2fts_info == FTS_NS) {
		return 0;
	}
	
	
	if (obj1statp == NULL || obj2statp == NULL) {
		return 0;
	}

	off_t obj1size = (*obj1)->fts_statp->st_size;
	off_t obj2size = (*obj2)->fts_statp->st_size;

	if (obj1size > obj2size) {
		return -1;
	} else if (obj1size < obj2size) {
		return 1;
	} else {
		return sortbyname(obj1, obj2);
	}
}

int sortbytime(const FTSENT **obj1, const FTSENT **obj2)
{
	double diff;
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;
	struct stat *obj1statp = (*obj1)->fts_statp;
	struct stat *obj2statp = (*obj2)->fts_statp;
	
	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	if (obj1fts_info == FTS_NS || obj2fts_info == FTS_NS) {
		return 0;
	}
	
	
	if (obj1statp == NULL || obj2statp == NULL) {
		return 0;
	}

	if (cflag) {
		diff = difftime((*obj1)->fts_statp->st_ctime, (*obj2)->fts_statp->st_ctime);
	} else if (uflag) {
		diff = difftime((*obj1)->fts_statp->st_atime, (*obj2)->fts_statp->st_atime);
	} else {
		diff = difftime((*obj1)->fts_statp->st_mtime, (*obj2)->fts_statp->st_mtime);
	}
		
	if (diff > 0) {
		return -1;
	} else if (diff < 0) {
		return 1;
	} else {
		return sortbyname(obj1, obj2);
	}
}

int sortbyrname(const FTSENT **obj1, const FTSENT **obj2)
{
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;

	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	return strcmp( (*obj2)->fts_name, (*obj1)->fts_name);
}

int sortbyrsize(const FTSENT **obj1, const FTSENT **obj2)
{
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;
	struct stat *obj1statp = (*obj1)->fts_statp;
	struct stat *obj2statp = (*obj2)->fts_statp;
	
	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	if (obj1fts_info == FTS_NS || obj2fts_info == FTS_NS) {
		return 0;
	}
	
	
	if (obj1statp == NULL || obj2statp == NULL) {
		return 0;
	}

	off_t obj1size = (*obj1)->fts_statp->st_size;
	off_t obj2size = (*obj2)->fts_statp->st_size;

	if (obj1size > obj2size) {
		return 1;
	} else if (obj1size < obj2size) {
		return -1;
	} else {
		return sortbyname(obj1, obj2);
	}
}

int sortbyrtime(const FTSENT **obj1, const FTSENT **obj2)
{
	double diff;
	unsigned short obj1fts_info = (*obj1)->fts_info;
	unsigned short obj2fts_info = (*obj1)->fts_info;
	struct stat *obj1statp = (*obj1)->fts_statp;
	struct stat *obj2statp = (*obj2)->fts_statp;
	
	if (obj1fts_info == FTS_ERR || obj2fts_info == FTS_ERR) {
		return 0;
	}

	if (obj1fts_info == FTS_NS || obj2fts_info == FTS_NS) {
		return 0;
	}
	
	if (obj1statp == NULL || obj2statp == NULL) {
		return 0;
	}

	if (cflag) {
		diff = difftime((*obj1)->fts_statp->st_ctime, (*obj2)->fts_statp->st_ctime);
	} else if (uflag) {
		diff = difftime((*obj1)->fts_statp->st_atime, (*obj2)->fts_statp->st_atime);
	} else {
		diff = difftime((*obj1)->fts_statp->st_mtime, (*obj2)->fts_statp->st_mtime);
	}
		
	if (diff > 0) {
		return 1;
	} else if (diff < 0) {
		return -1;
	} else {
		return sortbyname(obj1, obj2);
	}
}


int 
main(int argc, char **argv)
{
	int c;
	char *blkszptr;
	char **path_argv; 
	char *default_dot_path[] = {".", NULL};
	struct winsize win;	
	FTS* ftsptr;
	FTSENT *argv_list;
	FTSENT *cur;
	FTSENT *read;
	int fts_options = FTS_PHYSICAL;
	int (* comp)(const FTSENT** obj1, const FTSENT** obj2);

	/*
	 * get term width
	 * refers to NetBSD's implementation
	 */
	
	if (isatty(STDOUT_FILENO)) {
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == 0 &&win.ws_col > 0) {
			win_width = win.ws_col;			
		}
	}
	comp = sortbyname;
	
	setprogname(argv[0]);

	/* some default setting */
	if (isatty(STDOUT_FILENO)) {
		qflag = 1;
		Cflag = 1;
	} else {
		wflag = 1;
		oneflag = 1;
	}

	/*-A always set for super user*/
	if (getuid() == 0) {
		Aflag = 1;
	}

	while( (c = getopt(argc, argv, "AacdFfhiklnqRrSstuw1xC")) != -1 ) {
		switch(c) {
		case 'A':
			Aflag = 1;
			break;
		case 'a':
			aflag = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 'f':
			comp = NULL;
			fflag = 1;
			break;
		case 'F':
			Fflag = 1;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'l':
			//overwrite
			nflag = 0;
			oneflag  = 0;
			Cflag = 0;
			xflag = 0;

			lflag = 1;
			break;
		case 'n':
			//overwrite
			lflag = 0;
			oneflag = 0;
			Cflag = 0;
			xflag = 0;

			nflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 'S':
			comp = sortbysize;
			Sflag = 1;
			break;
		case 'q':
			wflag = 0;
			qflag = 1;
			break;
		case 'w':
			qflag = 0;
			wflag = 1;
			break;
		case 't':
			comp = sortbytime;
			tflag = 1;
			break;
		case '1':
			lflag = 0;
			nflag = 0;
			oneflag = 1;
			break;
		case 'c':
			uflag = 0;
			cflag = 1;
			break;
		case 'x':
			//overwirte
			oneflag = 0;
			lflag = 0;
			nflag = 0;
			Cflag = 0;

			xflag = 1;
			break;
		case 'C':
			//overwrite
			oneflag = 0;
			lflag = 0;
			nflag = 0;
			xflag = 0;

			Cflag = 1;
			break;
		case 'u':
			cflag = 0;
			uflag = 1;
			break;
		case 'k':
			hflag = 0;
			kflag = 1;
			break;
		case 'h':
			kflag = 0;
			hflag = 1;
			break;
		case 'r':
			rflag = 1;
			break;
		default:
			usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;
	
	cldlist_init();

	path_argv = (argc == 0) ? default_dot_path : argv;

	/*set block size, blksize_t shall be signed integer*/
	if ( (blkszptr = getenv("BLOCKSIZE")) != NULL ) {
		blocksize = atoi(blkszptr);
	} else {
		blocksize = DEFAULT_BLOCKSIZE;
	}

	if (kflag) {
		divide_term = 1024;
	}
	
	if (aflag) {
		fts_options =  fts_options | FTS_SEEDOT;
	}


	if (rflag) {
		if (Sflag) {
			comp = sortbyrsize;
		} else if (tflag) {
			comp = sortbyrsize;
		} else {
			comp = sortbyname;
		}
	}

	if ( (ftsptr = fts_open(path_argv, fts_options, comp)) == NULL) {
		if (errno != 0) {
			(void)fprintf(stderr, "%s: fts_open error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/*the special use case of fts_children, go through all files in the argv list*/
	if ( (argv_list = fts_children(ftsptr, 0)) == NULL ) {
		if (errno != 0) {
			(void)fprintf(stderr, "%s: fts_open error %s\n", getprogname(), strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/*
	 * refer to NetBSD implementation
	 * here we handle all `non-directory` files and error,
	 * leave the directory for the fts_read below 
	 */
	for (cur = argv_list; cur != NULL; cur = cur->fts_link) {

		if (cur->fts_info == FTS_ERR || cur->fts_info == FTS_NS || cur->fts_info == FTS_DNR) {
			warnx("can't access %s: %s",cur->fts_name, strerror(cur->fts_errno));
			/*
			 * because error message also consider as an output
			 * `outputnum++` will make the directory to output its path name
			 */
			outputnum++;
			continue;
		}
		
		if (!dflag && cur->fts_info == FTS_D) {
			/*leave the directory to the fts_read below*/
			outputnum++;
			continue;
		}
		outputentnum++;
		outputnum++;

		cook_entry(cur);
	}
	//Parent = NULL means it's at root level
	traverse_children(NULL);
	

	do{
		if (dflag) {
			break;
		}
		/*
		 * handle the directory entry
		 */
		while( (read = fts_read(ftsptr)) != NULL) {
			
			unsigned short info = read->fts_info;
			
			if (info == FTS_DC) {
				warnx("%s causes a cycle in the tree ",read->fts_path);
				continue;
			}

			if (info == FTS_DNR || info == FTS_ERR) {
				warnx("can't access %s: %s",read->fts_path, strerror(read->fts_errno));
				continue;
			}

			if (info == FTS_D) {
				FTSENT* childrenptr;
				childrenptr = fts_children(ftsptr,0);
				
				if (childrenptr == NULL && errno != 0) {	
					/*
					 * if error heppen we don't do anything this moment
					 * error will be printed at the postorder visit
					 */
					continue;
					
				}

				config_output(childrenptr);
				// cook_multi_cols();
				traverse_children(read);
	
				if (!Rflag && childrenptr != NULL) {
					if (fts_set(ftsptr, read, FTS_SKIP) != 0) {
						(void)fprintf(stderr, "%s: fts_set error %s\n", getprogname(), strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}while(0);

	if (read == NULL && errno != 0) {
		(void)fprintf(stderr, "%s: fts_read error %s\n", getprogname(), strerror(errno));
		exit(EXIT_FAILURE);
	}

	cldlist_clear();
	return 0;
}
