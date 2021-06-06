#include "util.c"
#include <time.h>
/************* cd_ls_pwd.c file **************/

int chdir(char *pathname)   
{
  printf("chdir %s\n", pathname);
  int ino = getino(pathname);
  if(ino == 0){
    printf("Error: no such directory\n");
    return -1;
  }

  MINODE *mip = iget(dev, ino);
  
  if(!S_ISDIR(mip->INODE.i_mode)){
    printf("Error: this is not a directory\n");
    return -1;
  }

  iput(running->cwd);
  running->cwd = mip;
  printf("changed directory to: %s\n", pathname);
  return 0;
}

int ls_file(MINODE *mip, char *name)
{

  // format time
  int links = mip->INODE.i_links_count;
  int uid = mip->INODE.i_uid;
  int gid = mip->INODE.i_gid;
  int size = mip->INODE.i_size;
  u16 md = mip->INODE.i_mode;

  int sec = mip->INODE.i_ctime;
  time_t time = (time_t)sec;
  char *str = ctime(&time);
  str[strlen(str) -1] = 0;
  char str2[5];
  char str3[4];
  sprintf(str2, "%3o", md);
  str3[0] = str2[2];
  str3[1] = str2[3];
  str3[2] = str2[4];
  str3[3] = '\0';
  char *moderef = "rwxrwxrwx";
  char bin[10] = "";

  int len = strlen(str3);
  for(int i = 0; i < len; i++){
    if(str3[i] == '0'){
      strcat(bin, "000");
    }
    else if(str3[i] == '1'){
      strcat(bin, "001");
    }
    else if(str3[i] == '2'){
      strcat(bin, "010");
    }
    else if(str3[i] == '3'){
      strcat(bin, "011");
    }
    else if(str3[i] == '4'){
      strcat(bin, "100");
    }
    else if(str3[i] == '5'){
      strcat(bin, "101");
    }
    else if(str3[i] == '6'){
      strcat(bin, "110");
    }
    else if(str3[i] == '7'){
      strcat(bin, "111");
    }
  }

  char mode[11];

  if (( md & 0xF000) == 0x8000) // if (S_ISREG()) 
    strcpy(mode, "-");
  if (( md & 0xF000) == 0x4000) // if (S_ISDIR()) 
    strcpy(mode, "d");
  if (( md & 0xF000) == 0xA000) // if (S_ISLNK()) 
    strcpy(mode, "l");
  
  len = strlen(bin);
  for(int i = 0; i < len; i++){
    if(bin[i] == '1'){
      mode[i+1] = moderef[i];
    }
    else{
      mode[i+1] = '-';
    }
  }

  mode[10] = '\0';

  printf("%s  ", mode);
  printf("%d  ", links);
  printf("%d  ", uid);
  printf("%d  ", gid);
  printf("%d  ", size);
  printf("%s  ", str);
  printf("%s\n", name);

  int guard = 0; // prevent stack smashing

  return guard;
  
}

int ls_dir(MINODE *mip)
{
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  for (int i = 0; i < 12; i++){
    if (mip->INODE.i_block[i] == 0){
      return 0;
    }
    // Assume DIR has only one data block i_block[0]
    get_block(mip->dev, mip->INODE.i_block[0], buf); 
    dp = (DIR *)buf;
    cp = buf;
  
    while (cp < (buf + BLKSIZE)){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      MINODE *mip = iget(dev, dp->inode);
      ls_file(mip, temp);
      iput(mip);
    
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }

}

int ls(char *pathname)  
{
  printf("ls %s\n", pathname);
  int ino;
  if(!strcmp(pathname, "")){
    // for cwd
    ls_dir(running->cwd);
  }
  else{
    ino = getino(pathname);
    MINODE *mip = iget(dev, ino);
    ls_dir(mip);
  }

}

char *pwd(MINODE *wd)
{
  if (wd == root){
    printf("cwd: /");
    return;
  }
  else{
    int my_ino = wd->ino;
    int parent_ino = findino(wd, my_ino);
    MINODE *pip = iget(dev, parent_ino);
    char myname[256];
    findmyname(pip, my_ino, myname);
    pwd(pip);
    if(pip != root){
      printf("/%s", myname);
    }
    else{
      printf("%s", myname);
    }
    iput(pip);
  }
}



