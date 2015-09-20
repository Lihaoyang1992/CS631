/*
 * TITLE: CS631-APUE-HW02-tcp
 * AUTHOR: Chenyi Zhang
 * DATE: 09/20/2014
 * EMAIL: czhang42@stevens.edu
 *
 * TCP HW: Copy a regular file to target, using perror instead fprint stderr
 */

#ifndef BUFFSIZE
#define BUFFSIZE (4096)
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error(const char * error);
char * maskDir(char * DIR, char * file_from);

int
main(int argc,  char * argv[])
{
    int file_from_fd;
    int file_to_fd;
    size_t count;
    char * file_from = argv[1];
    char * file_to;
    char buff[BUFFSIZE];
    struct stat st;
    
    /*Check arguments numbers*/
    if (argc!=3) {
        error("Wrong Argument Numbers: ");
        return EXIT_FAILURE;
    }
    
    /*check open*/
    if ( stat(file_from, &st) < 0){
        perror("Fail to access source file!");
        exit(EXIT_FAILURE);
    }
    
    /*get file addres*/
    file_to = maskDir(argv[2], file_from);
    
    /* Open file */
    if ((file_from_fd = open(file_from, O_RDONLY))<0) {
        error("Fail to open from file ");
        return EXIT_FAILURE;
    }
    
    if ((file_to_fd = open(file_to, O_WRONLY|O_CREAT, O_EXCL|S_IRUSR|S_IWUSR))<0) {
        error("Fail to create the to file ");
        return EXIT_FAILURE;
    }
    
    /*read file to bufffer*/
    while ((count = read(file_from_fd, buff, BUFFSIZE)) > 0){
    /*write into file*/
        if (write(file_to_fd, buff, count)<count){
            (void)close(file_from_fd);
            (void)close(file_to_fd);
            error("Fail to write a file ");
        }
    }
    /* close file */
    (void)close(file_from_fd);
    (void)close(file_to_fd);
    return EXIT_SUCCESS;
}

/*Throw error*/
void
error(const char * error)
{
    perror(error);
    exit(EXIT_FAILURE);
}

/*Fix direct problem*/
char * maskDir(char * DIR, char * file_from)
{
    struct stat st;
    
    fopen(DIR,"a");
    
    if(stat(DIR,&st)<0)
    {
        error("File stat Error ");
    };
    switch (st.st_mode&S_IFMT) {
        case S_IFREG:
            /*This is a regular file*/
            break;
        case S_IFDIR:
            /*This is a direct, so we need to create a file using same nam*/
            strcat(DIR, file_from);
            break;
        case S_IFIFO:
            /*This is a pipe*/
        default:
            break;
    }
    return DIR;
}

void usage()
{
    fprintf(stderr, "Usage: Copy a file! If going to a direct without a name, we will create a file using same name.");
}