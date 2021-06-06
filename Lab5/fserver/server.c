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

#include <errno.h>
#include <sys/syscall.h>

#define MAX   256
#define BLK  1024

int server_sock, client_sock;
char *serverIP = "127.0.0.1";      // hardcoded server IP address
int serverPORT = 1234;             // hardcoded server port number

struct sockaddr_in saddr, caddr;   // socket addr structs

struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------"; 
char *t2 = "----------------"; 

int ls_file(char *fname) { 
    struct stat fstat, *sp; 
    int r, i; 
    char ftime[64];
    char str[MAX];
    char temp[MAX];
    sp = &fstat; 
    if ( (r = lstat( fname, &fstat)) < 0){
        strcpy(str, "can't stat "); 
        strcat(str, fname);
        strcat(str, "\n"); 
        write(client_sock, str, MAX);
        exit( 1); 
    }
 

    if (( sp-> st_mode & 0xF000) == 0x8000) // if (S_ISREG()) 
        strcpy(str, "-"); 
        //write(client_sock, str, MAX);
    if (( sp-> st_mode & 0xF000) == 0x4000) // if (S_ISDIR()) 
        strcpy(str, "d"); 
        //write(client_sock, str, MAX);
    if (( sp-> st_mode & 0xF000) == 0xA000) // if (S_ISLNK()) 
        strcpy(str, "l"); 
        //write(client_sock, str, MAX);
    for (i = 8; i >= 0; i--){ 
        if (sp-> st_mode & (1 << i)){ // print r|w|x
            sprintf(temp, "%c", t1[i]); 
            //write(client_sock, str, MAX);
        }
        else{
            sprintf(temp, "%c", t2[i]); 
            //write(client_sock, str, MAX); // or print -
        }
        strcat(str, temp);
    }
        sprintf(temp, " %4d ", sp-> st_nlink); // link count 
        strcat(str, temp);
        //write(client_sock, str, MAX); 
        sprintf(temp, " %4d ", sp-> st_gid); // gid 
        strcat(str, temp);
        //write(client_sock, str, MAX); 
        sprintf(temp, " %4d ", sp-> st_uid); // uid 
        strcat(str, temp);
        //write(client_sock, str, MAX);
        sprintf(temp, " %8d ", sp-> st_size); // file size 
        strcat(str, temp);
        //write(client_sock, str, MAX);
        // print time 
        strcpy( ftime, ctime(& sp-> st_ctime) ); // print time in calendar form 
        ftime[ strlen( ftime)-1] = 0; // kill \n at end 
        sprintf(temp, "%s ", ftime); 
        strcat(str, temp);
        //write(client_sock, str, MAX);
        // print name 
        sprintf(temp, "%s", basename(fname) ); // print file basename 
        strcat(str, temp);
        //write(client_sock, str, MAX);
        // print -> linkname if symbolic file 
        if (( sp-> st_mode & 0xF000) == 0xA000){ 
            // use readlink() to read linkname 
            char *linkname;
            readlink(fname, linkname, sizeof(fname));
            sprintf(temp, " -> %s", linkname ); // print linked name 
            strcat(str, temp);
            //write(client_sock, str, MAX);
        }
        strcat(str, "\n");
        write(client_sock, str, MAX);
    
}

int ls_dir( char *dname) { 
    // use opendir(), readdir(); then call ls_file( name)
    DIR *dir = opendir(dname);
    struct dirent *d = readdir(dir);

    while(d != NULL){
        ls_file(d->d_name);
        d = readdir(dir);
    }

    char str[MAX];

    strcpy(str, "done");

    write(client_sock, str, MAX);
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
    printf("1. create a socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
     saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    
    printf("3. bind socket to server\n");
    if ((bind(server_sock, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
    printf("4. server listen with queue size = 5\n");
    if ((listen(server_sock, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    printf("5. server at IP=%s port=%d\n", serverIP, serverPORT);
}
  
int main(int argc, char *argv[], char *env[]) 
{
    int n, length;
    char line[MAX];
    char cwd[MAX];
    char *cmd[2];
    int ncmd = 0;
    char token1[MAX];
    char token2[MAX];
    char *root;
    
    init();

    getcwd(cwd, MAX);
    root = cwd;   

    while(1){
       printf("server: try to accept a new connection\n");
       length = sizeof(caddr);
       client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
       if (client_sock < 0){
          printf("server: accept error\n");
          exit(1);
       }
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

        // Processing loop
        while(1){
            printf("server ready for next request ....\n");
            n = read(client_sock, token1, MAX);
            n = read(client_sock, token2, MAX);
            if (n==0){
                printf("server: client died, server loops\n");
                close(client_sock);
                break;
            }

            strcpy(cmd[0], token1);
            strcpy(cmd[1], token2);

            // execute command
            if (!strcmp(cmd[0], "cd")){
                getcwd(cwd, MAX);
                printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);
                
                if (!(!strcmp(root, cwd) && !strcmp(cmd[1], "../"))){
                    int e = chdir(cmd[1]);
                    if (e == 0){

                        strcpy(line, "Server: Successfully changed directory to ");
                        strcat(line, cmd[1]);
                        strcat(line, "\n");
                    }
                    else{

                        strcpy(line, "Error: failed to change directory.\n");
                    }
                }
                else{

                    strcpy(line, "Server: directory at root.\n");

                }

                // send response to client
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; cmd=[%s %s]\n", n, cmd[0], cmd[1]);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
            else if (!strcmp(cmd[0], "pwd")){
            	printf("server: read  n=%d bytes; line=[%s]\n", n, cmd[0]);
                
                if (getcwd(cwd, MAX)){

                    char *temp = "/";
                    if(strcmp(cwd, root)){

                        char *token = strtok(cwd, root);
                        while(token != NULL){
                            token = strtok(NULL, root);
                        }
                        strcat(temp, token);
                    }

                    strcpy(line, "Server: cwd: ");
                    strcat(line, temp);
                    strcat(line, "\n");

                }
                else{
                    strcpy(line, "Error: failed to print current working directory.\n");
                }

                // send response to client
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; cmd=[%s]\n", n, cmd[0]);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
            else if (!strcmp(cmd[0], "mkdir")){
            	printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);
                int e = mkdir(cmd[1], ACCESSPERMS);
                if (e == 0){
                    strcpy(line, "Server: Successfully created directory ");
                    strcat(line, cmd[1]);
                    strcat(line, "\n");
                }
                else{
                    strcpy(line, "Error: failed to created directory.\n");
                }

                // send response to client
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; cmd=[%s %s]\n", n, cmd[0], cmd[1]);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
            else if (!strcmp(cmd[0], "rmdir")){
            	printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);
                int e = rmdir(cmd[1]);
                if (e == 0){
                    strcpy(line, "Server: Successfully removed directory ");
                    strcat(line, cmd[1]);
                    strcat(line, "\n");
                }
                else{
                    strcpy(line, "Error: failed to remove directory.\n");
                }

                // send response to client
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; cmd=[%s %s]\n", n, cmd[0], cmd[1]);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
            else if (!strcmp(cmd[0], "rm")){
            	printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);
                int e = remove(cmd[1]);
                if (e == 0){
                    strcpy(line, "Server: Successfully removed file ");
                    strcat(line, cmd[1]);
                    strcat(line, "\n");
                }
                else{
                    strcpy(line, "Error: failed to remove file.\n");
                }

                // send response to client
                n = write(client_sock, line, MAX);
                printf("server: wrote n=%d bytes; cmd=[%s %s]\n", n, cmd[0], cmd[1]);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
            else if (!strcmp(cmd[0], "get")){
                // transfer file from server to client
                printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);

                // open file
                FILE *fd;
                fd = fopen(cmd[1], "r");

                if(fd != NULL){
                    char *n;
                    n = fgets(line, MAX, fd);
                
                    while(n != NULL){

                        // write data to client
                        write(client_sock, line, MAX);
                        n = fgets(line, MAX, fd);

                    }
                    fclose(fd);
                    strcpy(line, "done");
                    write(client_sock, line, MAX);
                    printf("Successfully copied file to client.\n");
                }
                else{

                    printf("Error: failed to open file.\n");
                    strcpy(line, "ERR");
                    write(client_sock, line, MAX);

                }

            }
            else if (!strcmp(cmd[0], "put")){
                // transfer file from client to server
                printf("server: read  n=%d bytes; line=[%s %s]\n", n, cmd[0], cmd[1]);

                // open or create file
                FILE *fd;
                fd = fopen(cmd[1], "a+");

                n = read(client_sock, line, MAX);

                if(strcmp(line, "ERR")){

                    while(strcmp(line, "done")){

                        // write data to file
                        fprintf(fd, "%s", line);

                        // read data from client
                        n = read(client_sock, line, MAX);
                    }
                    fclose(fd);
                    strcpy(line, "Successfully copied file from client.\n");
                    write(client_sock, line, MAX);
                }
                else{

                    strcpy(line, "Error: could not retrieve file from client.\nFile may not exist on client or could be corrupted.\n");
                    write(client_sock, line, MAX);

                }

            }
            else if (!strcmp(cmd[0], "ls")){
		printf("server: read  n=%d bytes; line=[%s]\n", n, cmd[0]);
                lsfunct(argc, argv);
                printf("server: wrote n=%d bytes; cmd=[%s]\n", n, cmd[0]);

            }
            else{
                strcpy(line, cmd[0]);
                strcat(line, " is not a valid command!\n");
                write(client_sock, line, MAX);
                strcpy(line, "done");
                write(client_sock, line, MAX);
            }
        }
    }
}



