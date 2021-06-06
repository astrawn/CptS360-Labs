#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

MINODE minode[NMINODE];
MTABLE mtable[NMTABLE];
OFT oft[NOFT];
PROC proc[NPROC];
PROC *running;
MINODE *root;

char gpath[128]; // global for tokenized components
char name[32][128];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;