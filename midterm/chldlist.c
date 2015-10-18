#include "l.h"

static FNODEP header;
static FNODEP cur;
static FNODEP ptr;
static int count;

void cldlist_init(){
	header = (FNODEP)malloc(sizeof(FNODE));
	if (header == NULL){
		fprintf(stderr, "malloc error\n");
		exit(EXIT_FAILURE);
	}

	header->next = NULL;
	header->flag = HEADER;
	cur = header;
	ptr = header;
	count = 0;
}

FNODEP cldlist_get_list(){
	return header;
}

int cldlist_get_count(){
	return count;
}

void cldlist_reset(){
	FNODEP p = header->next;

	while( p != NULL){
		p->flag = UNUSED;
		p = p->next;
	}	

	cur = header;
	ptr = header;
	count = 0;
}

void cldlist_reset_ptr(){
	ptr = header;
}

FNODEP cldlist_get_next(){
	FNODEP ret = ptr->next;
	if (ret  == NULL){
		return NULL;
	}

	if ((ret->flag) == UNUSED){
		return NULL;
	}

	ptr = ptr->next;
	return ret;
}

/*
 * When push_back a Node
 * I donot free the memory but change it status flag
 * to avoid too many malloc/free operation
 */

void cldlist_push_back(FTSENT* ftentryp, char *uname, char *gname){
	FNODEP tmp;

	if (cur->next == NULL){
		tmp = (FNODEP)malloc(sizeof(FNODE));
		cur->next = tmp;
		tmp->next = NULL;
	}else{
		tmp = cur->next;
	}

	tmp->ftsentryptr = ftentryp;

	/* copy the user name */
	strncpy(tmp->username, uname, 20);

	/* copy the group name */
	strncpy(tmp->grpname, gname, 20);

	/* mark the flag */
	tmp->flag = USED;
	
	/* move the cursor */
	cur = tmp;
	count++;
}

void cldlist_clear(){
	FNODEP p;
	for (p = header; p != NULL;){
		FNODEP tmp;
		tmp = p;
		p = p->next;
		free(tmp);
	}
}

FNODEP cldlist_get(int index){
	FNODEP p = header;
	int i = 0;
	for (p = header->next; p != NULL;){
		if (i == index){
			return p;
		}

		i++;
		p = p->next;
	}

	return NULL;
}
