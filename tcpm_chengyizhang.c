/*
 * TITLE: CS631-APUE-HW02EX-tcpm
 * AUTHOR: Chenyi Zhang
 * DATE: 09/20/2014
 * EMAIL: czhang42@stevens.edu
 *
 * TCPM HW: Copy a regular file to target, using memory map
 */
#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FAIL
#define FAIL (0)
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

char * maskDir(char * DIR, char * file_from);
void usage();

int
main(int argc,  char * argv[])
{
    int file_from_fd;
    int file_to_fd;
    char * file_from = argv[1];
    char * file_to;
    struct stat st;
    void * file_from_reader, *file_to_writer;
    size_t from_file_size;
    mode_t from_file_mode;
    
    
    /*Check arguments numbers*/
    if (argc!=3) {
        (void)fprintf(stderr, "Wrong Argument Numbers: \n");
        usage();
        return EXIT_FAILURE;
    }
    
    /*check file status*/
    if (stat(file_from, &st) < FAIL){
        (void)fprintf(stderr, "Fail to access source file! \n");
        return EXIT_FAILURE;
    }else{
        from_file_size = st.st_size;
        from_file_mode = st.st_mode;
    }
    
    if ((st.st_mode&S_IFMT)!=S_IFREG) {
        (void)fprintf(stderr, "From file must be a regular file!\n");
        return EXIT_FAILURE;
    }
    
    /*get file addres*/
    file_to = maskDir(argv[2], file_from);
    
    /* Open file test*/
    if ((file_from_fd = open(file_from, O_RDONLY))<FAIL) {
        (void)fprintf(stderr, "Fail to open source file \n");
        return EXIT_FAILURE;
    }
    
    if ((file_to_fd = open(file_to, O_RDWR|O_CREAT|O_TRUNC,from_file_mode))<FAIL) {
        (void)fprintf(stderr, "Fail to open target file\n");
        return EXIT_FAILURE;
    }
    /*lseek check*/
    if (lseek(file_to_fd, from_file_size -1, SEEK_SET)  == -1){
        (void)fprintf(stderr, "Fail to lseek target file\n");
        exit(EXIT_FAILURE);
    }
    
    /*Write file test*/
    if (write(file_to_fd,"",1)!=1) {
        (void)fprintf(stderr, "Fail to write the target file\n");
        return EXIT_FAILURE;
    }
    
    /*point source file to memory*/
    if ((file_from_reader = mmap(0,from_file_size, PROT_READ, MAP_SHARED,file_from_fd,0))==MAP_FAILED) {
        (void)fprintf(stderr, "MMAP reader error \n");
        return EXIT_FAILURE;
    }
    
    /*point target file to memory*/
    if ((file_to_writer = mmap(0, from_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_to_fd, 0)) == MAP_FAILED) {
        (void)fprintf(stderr, "MMAP writer error \n");
        return EXIT_FAILURE;

    }
    /*do the copy*/
    memcpy(file_to_writer , file_from_reader, from_file_size);
    (void)close(file_from_fd);
    (void)close(file_to_fd);
    return EXIT_SUCCESS;
}

/*Fix direct problem*/
char * maskDir(char * DIR, char * file_from)
{
    struct stat st;
    
    fopen(DIR,"a");
    
    if(stat(DIR,&st)<FAIL)
    {
        (void)fprintf(stderr, "File stat Error \n");
    };
    switch (st.st_mode&S_IFMT) {
        case S_IFREG:
            /*This is a regular file*/
            break;
        case S_IFDIR:
            /*This is a direct, so we need to create a file using same name*/
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
    fprintf(stderr, "Usage: tcmp source target;\nCopy a file using memory map! If going to a direct without a name, we will create a file using same name.\n");
}