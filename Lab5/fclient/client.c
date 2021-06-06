#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------"; 
char *t2 = "----------------"; 

int ls_file(char *fname) { 
    struct stat fstat, *sp; 
    int r, i; 
    char ftime[64]; 
    sp = &fstat; 
    if ( (r = lstat( fname, &fstat)) < 0){ 
        printf("can't stat %s\n", fname); 
        exit( 1); 
    }
 

    if (( sp-> st_mode & 0xF000) == 0x8000) // if (S_ISREG()) 
        printf(" %c", '-'); 
    if (( sp-> st_mode & 0xF000) == 0x4000) // if (S_ISDIR()) 
        printf(" %c", 'd'); 
    if (( sp-> st_mode & 0xF000) == 0xA000) // if (S_ISLNK()) 
        printf(" %c", 'l'); 
    for (i = 8; i >= 0; i--){ 
        if (sp-> st_mode & (1 << i)){ // print r|w|x
            printf("%c", t1[i]);
        }
        else{
            printf("%c", t2[i]); // or print -
        }
    }
        printf(" %4d ", sp-> st_nlink); // link count 
        printf(" %4d ", sp-> st_gid); // gid 
        printf(" %4d ", sp-> st_uid); // uid 
        printf(" %8d ", sp-> st_size); // file size 
        // print time 
        strcpy( ftime, ctime(& sp-> st_ctime) ); // print time in calendar form 
        ftime[ strlen( ftime)-1] = 0; // kill \n at end 
        printf("% s ", ftime); 
        // print name 
        printf("% s ", basename(fname) ); // print file basename 
        // print -> linkname if symbolic file 
        if (( sp-> st_mode & 0xF000) == 0xA000){ 
            // use readlink() to read linkname 
            char *linkname;
            readlink(fname, linkname, sizeof(fname));
            printf(" -> %s", linkname ); // print linked name 
        }
        printf(" \n");
    
}

int ls_dir( char *dname) { 
    // use opendir(), readdir(); then call ls_file( name)
    DIR *dir = opendir(dname);
    struct dirent *d = readdir(dir);

    while(d != NULL){
        ls_file(d->d_name);
        d = readdir(dir);
    }

    closedir(dir); 
} 

int lsfunct( int argc, char *argv[]) { 
    struct stat mystat, *sp = &mystat; 
    int r; 
    char *filename, path[ 1024], cwd[ 256]; 
    filename = "./"; // default to CWD 
    if (argc > 1){ 
    filename = argv[ 1]; // if specified a filename 
    }
    if (r = lstat( filename, sp) < 0){ 
        printf(" no such file %s\n", filename); 
        exit( 1); 
    } 
    strcpy( path, filename); 
    if (path[ 0] != '/'){ // filename is relative : get CWD path    
        getcwd(cwd, 256);
        strcpy(path, cwd); 
        strcat(path, "/");
        strcat(path, filename);
    }
    if (S_ISDIR(sp->st_mode)){
        ls_dir(path);
    }
    else{
        ls_file(path);
    }
}


int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
}

int printMenu(){

    printf("\n**************************menu***************************\n");
    printf("*  get   put  ls    cd    pwd    mkdir    rmdir    rm   *\n");
    printf("*  lcat       lls   lcd   lpwd   lmkdir   lrmdir   lrm  *\n");
    printf("*********************************************************\n");
}


  
int main(int argc, char *argv[], char *env[]) 
{ 
    int  n;
    char line[MAX], ans[MAX];
    char *cmd[2];
    int ncmd;
    char cwd[MAX];
    FILE *fptr;
    char str[MAX];

    init();
  
    while (1){
        // print menu
        printMenu();
        printf("input a line : ");
        fgets(line, MAX, stdin);
        line[strlen(line)-1] = 0;       // kill <CR> at end

        char *token = strtok(line, " ");
        ncmd = 0;
        while(token != NULL && ncmd < 3){
            cmd[ncmd] = token;
            ncmd++;
            token = strtok(NULL, " ");
        }


        if (line[0]==0 || !strcmp(cmd[0], "exit")){                  // exit if NULL line
            exit(0);
        }
        else if (!strcmp(cmd[0], "lcd")){   // change directory
      	    int e = chdir(cmd[1]);
            if(e != 0){
                printf("Error: could not change directories.\n");
            }
            else{
                getcwd(cwd, MAX);
                printf("Changed directory to: %s\n", cwd);
            }
        }
        else if (!strcmp(cmd[0], "lpwd")){
            if(getcwd(cwd, MAX)){
                printf("cwd: %s\n", cwd);
            }
            else{
                printf("Error: failed to print current working directory.\n");
            }
        }
        else if (!strcmp(cmd[0], "lmkdir")){
            int e = mkdir(cmd[1], ACCESSPERMS);
            if(e == 0){
                printf("Directory %s created.\n", cmd[1]);
            }
            else{
                printf("Error: failed to create directory.\n");
            }
        }
        else if (!strcmp(cmd[0], "lrmdir")){
            int e = rmdir(cmd[1]);
            if(e == 0){
                printf("Successfully removed directory %s\n", cmd[1]);
            }
            else{
                printf("Error: failed to remove directory.\n");
            }
        }
        else if (!strcmp(cmd[0], "lrm")){
            int e = remove(cmd[1]);
            if(e == 0){
                printf("Successfully removed file %s\n", cmd[1]);
            }
            else{
                printf("Error: failed to remove file %s\n", cmd[1]);
            }
        }
        else if (!strcmp(cmd[0], "lls"))
        {
            lsfunct(argc, argv);
        }
        else if (!strcmp(cmd[0], "lcat")){
            fptr = fopen(cmd[1], "r");

            if (fptr != 0){

                printf("File contents:\n");

                while(fgets(str, MAX, fptr) != NULL){

                    printf("%s", str);
                }

                printf("\n");
            }
            else{
                printf("Error: could not open specified file.\n");
            }
        }
        else if (!strcmp(cmd[0], "get")){
            
            // transfer file from server to client
            n = write(sock, cmd[0], MAX);
            n =  write(sock, cmd[1], MAX);

            // open or create file
            FILE *fd;
            fd = fopen(cmd[1], "a+");

            n = read(sock, line, MAX);
            char *str = line;

            if(strcmp(line, "ERR")){

                while(strcmp(line, "done")){
                    str = line;

                    // write data to file
                    fprintf(fd, "%s", str);

                    // read data from client
                    n = read(sock, line, MAX);
                }
                fclose(fd);
                printf("Successfully copied file from server.\n");
            
            }
            else{

                printf("Error: could not retrieve file from server.\nFile may not exist on server or could be corrupted.\n");

            }

        }
        else if (!strcmp(cmd[0], "put")){

            // transfer file from client to server
            n = write(sock, cmd[0], MAX);
            n =  write(sock, cmd[1], MAX);

            // open file
            FILE *fd;
            fd = fopen(cmd[1], "r");

            if(fd != NULL){
                char *n;
                n = fgets(line, MAX, fd);
                
                while(n != NULL){

                    // write data to client
                    write(sock, line, MAX);
                    n = fgets(line, MAX, fd);

                }
                fclose(fd);
                strcpy(line, "done");
                write(sock, line, MAX);
                printf("Successfully copied file to server.\n");
            }
            else{

                printf("Error: failed to open file.\n");
                strcpy(line, "ERR");
                write(sock, line, MAX);

            }

        }
        else{

            // Send ENTIRE line to server
            n = write(sock, cmd[0], MAX);
            n = write(sock, cmd[1], MAX);
            printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

            // Read a line from sock and show it
            bzero(ans, MAX);
            n = read(sock, ans, MAX);

            while(strcmp(ans, "done")){
                
                if(n < 0){
                    printf("error reading\n");
                    break;
                }
                else{

                printf("%s", ans);
                bzero(ans, MAX);
                n = read(sock, ans, MAX);

                }
            }
        }
    }
}


