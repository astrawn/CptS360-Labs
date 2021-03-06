/************** lab3base.c file **************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX 128

char gdir[MAX];    // gdir[ ] stores dir strings
char *dir[64];
int  ndir;

char gpath[MAX];   // gpath[ ] stores token strings
char *name[64];
int  ntoken;

int main(int argc, char *argv[], char *env[])
{
  int  i, r;
  int  pid, status;
  char *s, cmd[64], cmd2[64], line[MAX];
  char *filename[1];

  printf("************* Welcome to mysh **************\n");
  i = 0;
  while (env[i]){
    printf("env[%d] = %s\n", i, env[i]);
    
    // Looking for PATH=
    if (strncmp(env[i], "PATH=", 5)==0){
      printf("show PATH: %s\n", env[i]);

      printf("decompose PATH into dir strings in gdir[ ]\n");
      strcpy(gdir, &env[i][5]);

      /*************** 1 ******************************
      Write YOUR code here to decompose PATH into dir strings in gdir[ ]
      pointed by dir[0], dir[1],..., dir[ndir-1]
      ndir = number of dir strings
      print dir strings
      ************************************************/
      
      char* token;
      token = strtok(gdir, ":");
      
      while (token != NULL) {

        dir[ndir] = token;
        ndir++;

        token = strtok(NULL, ":");
      }
      
      
      break;
    }
    i++;
  }
  
  printf("*********** mysh processing loop **********\n");

  while(1){
     printf("mysh % : ");

     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;      // fgets() has \n at end

     if (line[0]==0)
       continue;
     printf("line = %s\n", line);   // print line to see what you got

     /***************** 2 **********************
      Write YOUR code here to decompose line into token strings in gpath[ ]
      pointed by name[0], name[1],..., name[ntoken-1]
      ntoken = number of token strings
      print the token strings
      ************************************************/
      
      char* token;
      token = strtok(line, " ");
      ntoken = 0;
      name[1] = '\0';
      
      while (token != NULL) {

        name[ntoken] = token;
        ntoken++;

        token = strtok(NULL, " ");
      }
      
      ntoken++;

      // check for pipe
      int ret = 0;
      int k = 0;
      while (name[k]){

        if(!strcmp(name[k], "|")){

          ret = 1;

        }

        k++;

      }
     
     // 3. Handle name[0] == "cd" or "exit" case
     if (!strcmp(name[0], "cd")){
     
       if (name[1]){
       
         int rval = chdir(name[1]);

        if (rval != 0){

          printf("Failed to change directory.\n");

        }
       
       }
       else {

         const char *homedir = getenv("HOME");
       
         chdir(homedir);

       }
     
     }
     else if (!strcmp(name[0], "exit")){
     
       exit(0);
     
     }
     else if(ret){
        int sout = dup(1);
        int sin = dup(0);
        int fd;
        int j = 0;
        int *ptr = &j;

        char *head[64] =  {NULL};
        char *tail[64] = {NULL};

        int i = 0;

        while(strcmp(name[i], "|")){

          head[i] = name[i];
          i++;

        }

        i++;
        int k = 0;

        while(name[i]){

          tail[k] = name[i];
          i++;
          k++;

        }

        i = 0;
        printf("head: ");
        while(head[i]){
          printf("%s ", head[i]);
          i++;
        }

        i = 0;
        printf("\ntail: ");
        while(tail[i]){
          printf("%s ", tail[i]);
          i++;
        }

        printf("\n");   

        // 4. name[0] is not cd or exit:

        pid = fork();   // fork a child sh

        // check for IO redirection
        int re = checkRedirect(ptr, filename);

        if (pid){
          dup2(sout, 1);
          dup2(sin, 0);
          printf("parent sh %d waits\n", getpid());
          pid = wait(&status);
          printf("child sh %d died : exit status = %04x\n", pid, status);
          continue;
        }
        else{

          int pd[2];
          pipe(pd);
          int pid2 = fork(); // create head and tail processes

          if(pid2){
            
            // execute head cmd
	          printf("Child sh %d begins\n", getpid());
            for (i=0; i<ndir; i++){
              
	            strcpy(cmd, dir[i]); strcat(cmd, "/"); strcat(cmd, head[0]);
	            printf("i=%d cmd=%s\n", i, cmd);

              if(i == 0){

                close(pd[0]);
                close(1);
              }

              if (re != -1){
                fd = redirect(re, filename);
              }

              dup(pd[1]);
              close(pd[1]);
              r = execve(cmd, head, env);
              dup(stdout);
              close(stdout);

	         }
	          printf("cmd %s not found, child sh exit\n", head[0]);
	          exit(123);   // die with value 123

          }
          else{

            close(pd[1]);
            close(0);
            dup(pd[0]);
            close(pd[0]);

            // execute tail cmd
            printf("child sh %d begins\n", getpid());
            for (i=0; i<ndir; i++){
	  	  
	            strcpy(cmd2, dir[i]); strcat(cmd2, "/"); strcat(cmd2, tail[0]);
	            printf("i=%d cmd=%s\n", i, cmd2);

              if (re != -1){
                fd = redirect(re, filename);
              }

              r = execve(cmd2, tail, env);
              //dup(stdin);
              //close(stdin);

	    }
            printf("cmd %s not found, child sh exit\n", tail[0]);
	    exit(123);   // die with value 123

          }

        }
    }
    else{ 

      int sout = dup(1);
      int sin = dup(0);
      int fd;
      int j = 0;
      int *ptr = &j;       

      // 4. name[0] is not cd or exit:

      pid = fork();   // fork a child sh

      // check for IO redirection
      int re = checkRedirect(ptr, filename);

      if (pid){
        dup2(sout, 1);
        dup2(sin, 0);
        printf("parent sh %d waits\n", getpid());
        pid = wait(&status);
        printf("child sh %d died : exit status = %04x\n", pid, status);
        continue;
      }
      else{
        printf("child sh %d begins\n", getpid());

        // if given filename, execute file
        if(strchr(name[0], '.') != NULL){
          
          char * str = getenv("HOME");
          strcat(str, "/");
          strcat(str, name[0]);
          r = execve(str);
          

        }
        else{

        // execute cmd
        for (i=0; i<ndir; i++){
	  	  
	        strcpy(cmd, dir[i]); strcat(cmd, "/"); strcat(cmd, name[0]);
	        printf("i=%d cmd=%s\n", i, cmd);

          if (re != -1){
            fd = redirect(re, filename);
          }

	        r = execve(cmd, name, env);
          dup2(sout, 1);
          dup2(sin, 0);
	      }
	      printf("cmd %s not found, child sh exit\n", name[0]);
	      exit(123);   // die with value 123
        }
      }
    }
  }
}

/*********************** 5 *********************
Write your code to do I/O redirection:
Example: check any (name[i] == ">"). 
         If so, set name[i] = 0; 
                redirecct stdout to name[i+1] 
********************************************/

// remove "<", ">", ">>", and filename strings when present
char *moveArrayData(int *ptr, char *name[64]){

  char *filename;

  filename = name[*ptr + 1];

  for (int i = *ptr; i < 62; i++){

    name[i] = name[i+2];

  }

  return filename;
}

int checkRedirect(int *ptr, char *filename[1]){

  *ptr = 0;
  int re = -1;

  // check for IO redirection
  while (name[*ptr]){

    if (!strcmp(name[*ptr], ">")){

      re = 0;
      filename[0] = moveArrayData(ptr, name);
      return re;

    }
    else if(!strcmp(name[*ptr], "<")){

      re = 1;
      filename[0] = moveArrayData(ptr, name);
      return re;

    }
    else if (!strcmp(name[*ptr], ">>")){

      re = 2;
      filename[0] = moveArrayData(ptr, name);
      return re;

    }

    *ptr = *ptr + 1;

  }

  return re;


}

// redirect IO if necessary
int redirect(int re, char *filename[1]){

  int fd;

  if (re == 0){

    close(1);
    fd = open(filename[0], O_WRONLY|O_CREAT);

  }
  else if(re == 1){

    close(0);
    fd = open(filename[0], O_RDONLY);

  }
  else if (re == 2){

    close(1);
    fd = open(filename[0], O_WRONLY|O_CREAT|O_APPEND);

  }

  return fd;

}
