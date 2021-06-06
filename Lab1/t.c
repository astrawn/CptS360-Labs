//Alex Strawn
//11632677

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct partition {
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
}PARTITION;

int read_sector(int fd, int sector, char *buf)
{
  int n;
  lseek(fd, sector*512, SEEK_SET);
  n = read(fd, buf, 512);
  if (n <= 0){
    printf("read failed\n");
    return -1;
  }
  return n;
}

PARTITION *p;

int fd;
char buf[512];

char *ctable = "0123456789ABCDEF";
int  BASE = 10; 

int rpu(u32 x)
{  
    char c;
    if (x){
       c = ctable[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

int printu(u32 x)
{
   (x==0)? putchar('0') : rpu(x);
   //putchar(' ');
}

// 2-1
void prints(char *s){
  int l = strlen(s);
  
  for(int i = 0; i < l; i++){
    putchar(s[i]);
  }
}

// 2-2
int printd(int x){

  if(x<0){
    putchar('-');
    printu(-x);
  }
  else{
    printu(x);
  }
}

int printx(u32 x){
  
  BASE = 16;
  putchar('0');
  putchar('x');
  printu(x);
  
}

int printo(u32 x){

  BASE = 8;
  putchar('0');
  printu(x);
  
}

int myprintf(char *fmt, ...){

  char *cp = fmt;
  int *ip = &fmt + 1;
  
  int i = 0;
  
  while(cp[i] != '\0'){
   
    if(cp[i] == '%'){
    	
      i++;
    
      if(cp[i] == 's'){
      
        prints(*ip);
      
      }
      else if (cp[i] == 'u'){
      
        printu(*ip);
      
      }
      else if (cp[i] == 'd'){
      
        printd(*ip);
      
      }
      else if (cp[i] == 'o'){
      
        printo(*ip);
      
      }
      else if (cp[i] == 'x'){
      
        printx(*ip);
      
      }
      else if (cp[i] == 'c'){
      
        putchar(*ip);
      
      }
      
      ip+=1;
      
    } 
    else if (cp[i] == '\\') {
      
      if(cp[i+1] == 'n'){
       
        i++;
        putchar('\n');
        putchar('\r');
      
      }
      
    } 
    else{
      
      putchar(cp[i]);
      
    }
    
    i++;
  
  }

}

int main(int argc, char *argv[], char *env[])
{
  fd = open("vdisk", O_RDONLY);  // open disk for READ

  read_sector(fd, 0, buf);   // READ sector 0 into buf[ ]

  p = (PARTITION *)(buf+0x1BE);      
  int end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  p = (PARTITION *)(buf+0x1CE);     
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
  
  p = (PARTITION *)(buf+0x1DE);      
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  p = (PARTITION *)(buf+0x1EE);     
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  int partition_end = end;
	 
  printf("-----EXTEND PARTITION-----\n");
	 
  read_sector(fd, 1440, buf); 
	 
  p = (PARTITION *)(buf+0x1BE);      
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  p = (PARTITION *)(buf+0x1CE);      
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  printf("-----EXTEND PARTITION-----\n");
	 
  read_sector(fd, 1817, buf);
  
  p = (PARTITION *)(buf+0x1BE);      
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  p = (PARTITION *)(buf+0x1CE);      
  end = p->start_sector + p->nr_sectors - 1;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);
	 
  printf("-----EXTEND PARTITION-----\n");
	 
  read_sector(fd, 2339, buf);
  
  p = (PARTITION *)(buf+0x1BE);      
  end = p->start_sector + p->nr_sectors;
  printf("start=%d end=%d number=%d type=%x\n",
	 p->start_sector, end, p->nr_sectors, p->sys_type);

  getchar();
	 
  prints("1... 2... 3... Hello!");

  getchar();

  printd(-1);
  
  getchar();
  
  printx(3000); //expect 0xBB8 output
  
  getchar();
  
  printo(1792); //expect 03400 output
  
  getchar();
  
  myprintf("argc=%d  argv=%s  env=%s", argc, argv[0], env[0]);
  
  getchar();
  
  myprintf("cha=%c string=%s      dec=%d hex=%x oct=%o neg=%d", 
	       'A', "this is a test", 100,    100,   100,  -100);
	       
  getchar();
  
  myprintf("%d, %x, %o, %u %s %c %s\n", -1, 2, 3, 4, "Fox", '>', "^..^");

  return 0;
 
}
