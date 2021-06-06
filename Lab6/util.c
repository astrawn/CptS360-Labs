#include "global.c"

/*********** util.c file ****************/
MINODE *mialloc(){
  int i;
  for (i = 0; i < NMINODE; i++){
    MINODE *mp = &minode[i];
    if(mp->refCount == 0){
      mp->refCount = 1;
      return mp;
    }
  }
  printf("FS panic: out of minodes\n");
  return 0;
}

int midalloc(MINODE *mip){
  mip->refCount = 0;
}

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  // copy pathname into gpath[]; tokenize it into name[0] to name[n-1]
  // Code in Chapter 11.7.2
  char *s;
  char gpath[128];
  strcpy(gpath, pathname);
  n = 0;
  s = strtok(gpath, "/");
  while(s){
     strcpy(name[n++], s);
     s = strtok(0, "/");
  } 
}


MINODE *iget(int dev, int ino)
{
  // return minode pointer of loaded INODE=(dev, ino)
  // Code in Chapter 11.7.2
  MINODE *mip;
  MTABLE *mp;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  
  for (i = 0; i < NMINODE; i++){
  	MINODE *mip = &minode[i];
  	if (mip->refCount && (mip->dev==dev) && (mip->ino==ino)){
  	
  		mip->refCount++;
  		return mip;
  	}
  }
  
  mip = mialloc();
  mip->dev = dev;
  mip->ino = ino;
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  int iblock = gp->bg_inode_table;

  block = (ino-1)/8 + iblock;
  offset = (ino - 1)%8;
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip;

  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mptr = 0;
  return mip;
  
}

void iput(MINODE *mip)
{
  // dispose of minode pointed by mip
  // Code in Chapter 11.7.2
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];

  if (mip==0){
    return;
  }
  mip->refCount--;

  if (mip->refCount > 0){
    return;
  }

  if (mip->dirty == 0){
    return;
  }

  block = (mip->ino - 1) / 8 + mip->mptr->iblock;
  offset = (mip->ino - 1) % 8;

  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset;
  *ip = mip->INODE;
  put_block(mip->dev, block, buf);
  midalloc(mip);

} 



int search(MINODE *mip, char *name)
{
  // search for name in (DIRECT) data blocks of mip->INODE
  // return its ino
  // Code in Chapter 11.7.2
  int i;
  char *cp, temp[256], temp2[256], sbuf[BLKSIZE];
  DIR *dp;
  for (i = 0; i < 12; i++){
    if (mip->INODE.i_block[i] == 0){
      return 0;
    }

    strncpy(temp2, name, strlen(name));
    temp2[strlen(name)] = 0;

    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
      if(strcmp(temp2, temp) == 0){

        printf("found %s : inumber = %d\n", temp2, dp->inode);
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int getino(char *pathname)
{
  // return ino of pathname
  // Code in Chapter 11.7.2
  MINODE *mip;
  int i, ino;
  if(strcmp(pathname, "/") == 0){
    return 2;
  }
  else{

    if(pathname[0] == '/'){
      mip = root;
    }
    else{
      mip = running->cwd;
    }

    mip->refCount++;
    tokenize(pathname);
    for (i = 0; i < n; i++){
      if(!S_ISDIR(mip->INODE.i_mode)){
        printf("%s is not a directory\n", name[i]);
        iput(mip);
        return 0;
      }
      char *nm = name[i];
      ino = search(mip, nm);
      if(!ino){
        printf("no such component name %s\n", name[i]);
        iput(mip);
        return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
    }
    iput(mip);
    return ino;
  }
}

int findmyname(MINODE *parent, u32 myino, char myname[256]) 
{
  // WRITE YOUR code here:
  // search parent's data block for myino;
  // copy its name STRING to myname[ ];
  int i;
  int temp;
  char *cp, sbuf[BLKSIZE];
  DIR *dp;
  for (i = 0; i < 12; i++){
    if (parent->INODE.i_block[i] == 0){
      return 0;
    }

    get_block(parent->dev, parent->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while(cp < sbuf + BLKSIZE){
      temp = dp->inode;
      
      printf("%8d%8d%8u %s\n", temp, dp->rec_len, dp->name_len, dp->name);
      if(temp == myino){

        printf("found inode %d : name = %s\n", myino, dp->name);
        strcpy(myname, dp->name);
        if(myname[strlen(dp->name)-1] == '\f' || myname[strlen(dp->name)-1] == '\n' || myname[strlen(dp->name)-1] == '\r'){
          myname[strlen(dp->name)-1] = 0; // cut off garbage return characters from name
        }
        return 0;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
  // mip->a DIR minode. Write YOUR code to get mino=ino of .
  //                                         return ino of ..

  char buf[BLKSIZE];
  int ino;
  DIR *dp;
  char *cp;

  get_block(mip->dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;

  while(cp < buf + BLKSIZE){

    if(!strcmp(dp->name, "..")){
      ino = dp->inode;
      return ino;
    }

    cp += dp->rec_len;
    dp = (DIR *) cp;

  }

  return 0;
}

