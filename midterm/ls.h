#ifndef __CHILDREN_LIST_H__
	#define __CHILDREN_LIST_H__

	#include <sys/types.h>
	#include <fts.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>

	#ifndef DEFAULT_BLOCKSIZE
	#define DEFAULT_BLOCKSIZE (512)
	#endif

	#ifndef MAX_USER_NAME
	#define MAX_USER_NAME (32)
	#endif

	#ifndef MAX_GROUP_NAME
	#define MAX_GROUP_NAME (32)
	#endif


	/*
	 * children linked list
	 * used to store all information of children under a directory
	 */

	#define USED (0)
	#define UNUSED (1)
	#define HEADER (2)

	struct _file_node;

	typedef struct _file_node FNODE;
	typedef FNODE *FNODEP;

	struct _file_node{
		char username[MAX_USER_NAME];
		char grpname[MAX_GROUP_NAME];
		FTSENT *ftsentryptr;
		int flag;
		FNODEP next;  
	};

	extern void cldlist_init();
	extern FNODEP cldlist_get_list();
	extern void cldlist_push_back(FTSENT* ftentryp, char *uname, char *gname);
	extern void cldlist_reset();
	extern FNODEP cldlist_get_next();
	extern int cldlist_get_count();
	extern void cldlist_clear();
	extern void cldlist_reset_ptr();
	extern FNODEP cldlist_get(int index);

	/*
	 * compare functoin for sorting
	 * used in fts_open function
	 */
	extern int sortbyname(const FTSENT **obj1, const FTSENT **obj2);
	extern int sortbysize(const FTSENT **obj1, const FTSENT **obj2);
	extern int sortbytime(const FTSENT **obj1, const FTSENT **obj2);
	extern int sortbyrname(const FTSENT **obj1, const FTSENT **obj2);
	extern int sortbyrsize(const FTSENT **obj1, const FTSENT **obj2);
	extern int sortbyrtime(const FTSENT **obj1, const FTSENT **obj2);

#endif //__CHILDREN_LIST_H__
