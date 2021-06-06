#include <stdio.h>             // for I/O
#include <stdlib.h>            // for lib functions
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node {
    char  name[64];       // node's name string
    char  type;
    struct node* child, * sibling, * parent;
}NODE;


NODE* root, * cwd, * start;
char command[16], pathname[64];

char gpath[128];               // global gpath[] to hold token strings
char* names[64];                // token string pointers
int  n;                        // number of token strings

char dname[64], bname[64];     // dirname, basename of pathname

//               0         1       2      3     4       5       6     7       8         9       10
char* cmd[] = { "mkdir", "rmdir", "cd", "ls", "pwd", "creat", "rm", "save", "reload", "menu", "quit", 0 };

int findCmd(char* command)
{
    int i = 0;
    while (cmd[i]) {
        if (strcmp(command, cmd[i]) == 0)
            return i;
        i++;
    }
    return -1;
}

NODE* search_child(NODE* parent, char* name)
{
    NODE* p;
    printf("search for %s in parent DIR\n", name);
    p = parent->child;
    if (p == 0)
        return 0;
    while (p) {
        if (strcmp(p->name, name) == 0)
            return p;
        p = p->sibling;
    }
    return 0;
}

int insert_child(NODE* parent, NODE* q)
{
    NODE* p;
    printf("insert NODE %s into parent child list\n", q->name);
    p = parent->child;
    if (p == 0)
        parent->child = q;
    else {
        while (p->sibling)
            p = p->sibling;
        p->sibling = q;
    }
    q->parent = parent;
    q->child = 0;
    q->sibling = 0;
}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char* pathname)
{
    NODE* p, * q;
    printf("mkdir: pathname=%s\n", pathname);

    // write YOUR code to not allow mkdir of /, ., ./, .., or ../
    if(strcmp(pathname, "/") == 0 || strcmp(pathname, ".") == 0 || strcmp(pathname, "./") == 0 || strcmp(pathname, "..") == 0 || strcmp(pathname, "../") == 0){
      printf("%s is not a valid name for a new directory\n", pathname);
      return -1;
    }

    tokenize(pathname);

    if (names[1] == NULL) {
        start = cwd;
    }
    else {
        int complete = -1;
        start = root;

        NODE* parent = start;
        char* child = names[0];
        int x = 0;
        while (complete != 0) {

            parent = search_child(parent, child);
            if (!parent || parent->type == 'F') {

                printf("Invalid pathname\n");
                return -1;

            }
            else if (strcmp(parent->name, names[n - 1]) == 0){

                complete = 0;
                start = parent;

            }

            x++;
            child = names[x];
        }
    }

    

    char* name = names[n];

    printf("check whether %s already exists\n", name);
    p = search_child(start, name);
    if (p) {
        printf("name %s already exists, mkdir FAILED\n", name);
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to mkdir %s\n", name);
    q = (NODE*)malloc(sizeof(NODE));
    q->type = 'D';
    strcpy(q->name, name);
    insert_child(start, q);
    printf("mkdir %s OK\n", name);
    printf("--------------------------------------\n");

    return 1;
}

int rmdir(char* pathname) {


    if (strcmp(pathname, "/") == 0) {

        printf("cannot remove root directory\n");
        return -1;

    }

    NODE* p, * q;
    printf("rmdir: pathname=%s\n", pathname);

    tokenize(pathname);

    if (names[1] == NULL) {
        start = cwd;
    }
    else {
        int complete = -1;
        start = root;

        NODE* parent = start;
        char* child = names[0];
        int x = 0;
        while (complete != 0) {

            parent = search_child(parent, child);
            if (!parent || parent->type == 'F') {

                printf("Invalid pathname\n");
                return -1;

            }
            else if (strcmp(parent->name, names[n - 1]) == 0) {

                complete = 0;
                start = parent;

            }

            x++;
            child = names[x];
        }
    }

    char* name = names[n];

    printf("check whether %s exists\n", name);
    p = search_child(start, name);
    if (!p || p->type == 'F') {
        printf("directory %s does not exist, rmdir FAILED\n", name);
        return -1;
    }

    if (p->child) {

        printf("cannot remove this directory, there are directories below it\ndirectory removal FAILED\n");
        return -1;
    }

    printf("--------------------------------------\n");
    printf("ready to rmdir %s\n", name);

    if (start->child == p && !p->sibling) {

        start->child = NULL;

    }
    else if (start->child == p && p->sibling) {

        start->child = p->sibling;

    }
    else if (start->child != p && p->sibling) {

        q = start->child;

        while (q->sibling != p) {

            q = q->sibling;

        }

        q->sibling = p->sibling;

    }
    else if (start->child != p && !p->sibling) {

        q = start->child;

        while (q->sibling != p) {

            q = q->sibling;

        }

        q->sibling = NULL;

    }

    free(p);
    printf("rmdir %s OK\n", name);
    printf("--------------------------------------\n");

}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char *pathname)
{

    NODE* pd = cwd; // keep track of where cwd was when this function was called

    if (strcmp(pathname, "") != 0) {

        NODE* dir;

        tokenize(pathname);

        if (strcmp(pathname, "/") == 0) {

            cwd = root;
            printf("cwd changed to %s\n", pathname);

        }
        else if (names[1] == NULL) {

            dir = search_child(cwd, names[0]);

            if (dir) {

                cwd = dir;

            }
            else {

                printf("Invalid pathname, could not find directory.\n");

            }
        }
        else {

            int complete = -1;

            NODE* parent = root;
            char* child = names[0];
            int x = 0;
            while (complete != 0) {

                parent = search_child(parent, child);
                if (!parent) {

                    printf("Invalid pathname\n");

                    cwd = pd;

                    return -1;

                }
                else if (strcmp(parent->name, names[n]) == 0) {

                    complete = 0;
                    cwd = parent;

                }

                x++;
                child = names[x];
            }
        }

    }

    NODE* p = cwd->child;
    printf("cwd contents = ");
    while (p) {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");

    cwd = pd; //reset cwd to previous cwd

}

// enables user to change the current working directory (cwd)
int cd(char *pathname) {

    NODE* dir;

    tokenize(pathname);

    if (strcmp(pathname, "") == 0) {

        return -1;

    }
    else if (strcmp(pathname, "/") == 0) {

        cwd = root;
        printf("cwd changed to %s\n", pathname);

    }
    else if (names[1] == NULL && pathname[0] != '/') {

        dir = search_child(cwd, names[0]);

        if (dir && dir->type != 'F') {

            printf("cwd changed to %s\n", pathname);

            cwd = dir;

        }
        else if (strcmp(names[0], "..") == 0 && cwd->parent) {

            cwd = cwd->parent;

        }
        else {

            printf("Invalid pathname, could not change directories.\n");

        }
    }
    else {

        int complete = -1;

        NODE* parent = root;
        char* child = names[0];
        int x = 0;
        while (complete != 0) {


            if (strcmp(child, "..") != 0) {

                parent = search_child(parent, child);

                if (!parent) {

                    printf("Invalid pathname\n");
                    return -1;

                }
                else if (strcmp(parent->name, names[n]) == 0 && parent->type != 'F') {

                    complete = 0;
                    cwd = parent;
                    printf("cwd changed to %s\n", pathname);

                    return 0;

                }
                else if (strcmp(parent->name, names[n]) == 0 && parent->type == 'F') {

                    printf("Invalid pathname, cannot treat a file as a directory\n");
                    return -1;

                }
            }
            else {

                if (cwd->parent) {
                    cwd = cwd->parent;
                }
            }

            x++;
            child = names[x];

            if (child == NULL) {

                return 0;

            }

        }
    }


}

// displays the full cwd pathname
int pwd() {

    NODE* dir = cwd;

    char* dirs[128];

    int x = 0;

    while (dir != root) {

        dirs[x] = malloc(strlen(dir->name) + 1);

        strcpy(dirs[x], dir->name);

        dir = dir->parent;

        x++;

    }

    printf("cwd: ");
    
    if (cwd == root){
    
       printf("/");
    
    }
    

    x--;

    while (x >= 0) {

        printf("/%s", dirs[x]);

        x--;
    }

    printf("\n");

    return 0;

}

int creat(char *pathname) {

    NODE* p, * q;
    printf("creat: pathname=%s\n", pathname);

    if (strcmp(pathname, "/") == 0 || strcmp(pathname, ".") == 0 || strcmp(pathname, "./") == 0 || strcmp(pathname, "..") == 0 || strcmp(pathname, "../") == 0) {
        printf("%s is not a valid name for a new file\n", pathname);
        return -1;
    }

    tokenize(pathname);

    if (names[1] == NULL) {
        start = cwd;
    }
    else {
        int complete = -1;
        start = root;

        NODE* parent = start;
        char* child = names[0];
        int x = 0;
        while (complete != 0) {

            parent = search_child(parent, child);
            if (!parent || parent->type == 'F') {

                printf("Invalid pathname\n");
                return -1;

            }
            else if (strcmp(parent->name, names[n - 1]) == 0) {

                complete = 0;
                start = parent;

            }

            x++;
            child = names[x];
        }
    }

    char* name = names[n];

    printf("check whether %s already exists\n", name);
    p = search_child(start, name);
    if (p) {
        printf("name %s already exists, creat FAILED\n", name);
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to creat %s\n", name);
    q = (NODE*)malloc(sizeof(NODE));
    q->type = 'F';
    strcpy(q->name, name);
    insert_child(start, q);
    printf("mkdir %s OK\n", name);
    printf("--------------------------------------\n");

    return 1;

}

int rm(char* pathname) {

    if (strcmp(pathname, "/") == 0) {

        printf("cannot remove root directory\n");
        return -1;

    }

    NODE* p, * q;
    printf("rm: pathname=%s\n", pathname);

    tokenize(pathname);

    if (names[1] == NULL) {
        start = cwd;
    }
    else {
        int complete = -1;
        start = root;

        NODE* parent = start;
        char* child = names[0];
        int x = 0;
        while (complete != 0) {

            parent = search_child(parent, child);
            if (!parent || parent->type == 'F') {

                printf("Invalid pathname\n");
                return -1;

            }
            else if (strcmp(parent->name, names[n - 1]) == 0) {

                complete = 0;
                start = parent;

            }

            x++;
            child = names[x];
        }
    }

    char* name = names[n];

    printf("check whether %s exists\n", name);
    p = search_child(start, name);
    if (!p || p->type == 'D') {
        printf("file %s does not exist, rmdir FAILED\n", name);
        return -1;
    }

    printf("--------------------------------------\n");
    printf("ready to rmdir %s\n", name);

    if (start->child == p && !p->sibling) {

        start->child = NULL;

    }
    else if (start->child == p && p->sibling) {

        start->child = p->sibling;

    }
    else if (start->child != p && p->sibling) {

        q = start->child;

        while (q->sibling != p) {

            q = q->sibling;

        }

        q->sibling = p->sibling;

    }
    else if (start->child != p && !p->sibling) {

        q = start->child;

        while (q->sibling != p) {

            q = q->sibling;

        }

        q->sibling = NULL;

    }

    free(p);
    printf("rm %s OK\n", name);
    printf("--------------------------------------\n");


}


// save tree to file
int save(char *filename) {

    // create/open file to save to
    FILE* fp;

    if (strcmp(filename, "") == 0) {

        strcpy(filename, "myfile");

    }

    fp = fopen(filename, "w");

    if (fp == NULL) {

        printf("ERROR: the file could not be found.\n");

        return -1;

    }

    // print lines to text file
    NODE* node = root;
    write_to_file(fp, node);

    printf("save successful!\n");

    // close file
    fclose(fp);


}

// helper function for saving tree
int write_to_file(FILE* fp, NODE* node) {

    NODE* dir = node;

    char* dirs[128];

    int x = 0;

    if (node) {

        fprintf(fp, "%c ", node->type);

        if (dir == root) {

            fprintf(fp, "/");
        }

        while (dir != root) {

            dirs[x] = malloc(strlen(dir->name) + 1);

            strcpy(dirs[x], dir->name);

            dir = dir->parent;

            x++;

        }

        x--;

        while (x >= 0) {

            fprintf(fp, "/%s", dirs[x]);

            x--;
        }

        fprintf(fp,"\n");

    }
    else {

        return;

    }

    if (node->child) {

        write_to_file(fp, node->child);

    }

    if (node->sibling) {

        write_to_file(fp, node->sibling);

    }

    return 0;

}

// load tree from file
int reload(char *filename) {

    // open file to reload
    FILE* fp;
    fp = fopen(filename, "r");

    if (fp == NULL) {

        printf("ERROR: the file could not be found.\n");

        return -1;

    }

    // read contents of file and construct tree
    char str[128] = { "\0" };
    char substr[128] = { "\0" };
    char* token;

    while (fgets(str, 128, fp) != NULL) {

        strncpy(substr, str + 2, strlen(str) - 2);
        token = strtok(substr, "\n");
        strcpy(substr, token);

        if (str[0] == 'D' && strcmp(substr, "/") != 0){
        
            mkdir(substr);

        }
        else if (str[0] == 'F') {

            creat(substr);

        }

    }

    printf("load successful!\n");

    // close file
    fclose(fp);

    return 0;

}

// print menu
int menu() {

    printf("\n-----------menu-----------\nmkdir: create a directory\nrmdir: delete a directory\n");
    printf("cd: move to different directory\nls: show contents of current working directory (cwd)\n");
    printf("pwd: printf the full pathname of the cwd\ncreat: creat a file\nrm: delete a file\n");
    printf("save: save directory tree to file\nreload: load a directory tree from a file\n");
    printf("menu: display this menu\nquit: exit the program and save directory tree\n");
    printf("--------------------------\n\n");

}

int quit()
{
    save("myfile");
    printf("Program exit\n");
    exit(0);

    // improve quit() to SAVE the current tree as a Linux file
    // for reload the file to reconstruct the original tree
}

int initialize()  // create / node, set root and cwd pointers
{
    root = (NODE*)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}

int main()
{
    int index;
    char line[128];

    initialize();

    printf("NOTE: commands = [mkdir|rmdir|cd|ls|pwd|creat|rm|save|reload|menu|quit]\n");

    while (1) {
        printf("Enter command line : ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        sscanf(line, "%s %s", command, pathname);
        printf("command=%s pathname=%s\n", command, pathname);

        if (command[0] == 0)
            continue;

        index = findCmd(command);

        switch (index) {
        case 0: mkdir(pathname);  break;
        case 1: rmdir(pathname);  break;
        case 2: cd(pathname);     break;
        case 3: ls(pathname);     break;
        case 4: pwd();            break;
        case 5: creat(pathname);  break;
        case 6: rm(pathname);     break;
        case 7: save(pathname);   break;
        case 8: reload(pathname); break;
        case 9: menu();           break;
        case 10: quit();          break;
        }

        strcpy(pathname, ""); //reset pathname

    }
}


int tokenize(char* pathname)
{
    memset(names, '\0', sizeof(names));
    n = 0;
    char path[128] = { "" };

    if (pathname[0] == '/') {
        strncpy(path, pathname + 1, strlen(pathname) - 1);
    }
    else {
        strcpy(path, pathname);
    }
    char* token;
    token = strtok(path, "/");

    while (token != NULL) {

        strcpy(gpath, token);
        names[n] = malloc(strlen(gpath) + 1);
        strcpy(names[n], gpath);
        n++;

        token = strtok(NULL, "/");
    }

    n--;

    return 0;
}



NODE *path2node(char *pathname)
{
   // return pointer to the node of pathname, or NULL if invalid
   if (pathname[0] == '/'){
      start = root;
   }
   else{
      start = cwd;
   }

   tokenize(pathname);
   NODE *node = start;

   for (int i=0; i<n; i++){
       node = search_child(node, names[i]);

       if(!node){
          return NULL;
       }
   }
   return node;
}


int dir_base_name(char *pathname)
{
    // divide pathname into dirname in dname[], basename in bname[]

    tokenize(pathname);

    strcpy(dname, names[n]);

    for (int i = 0; i < n; i++) {

        strcat(bname, "/");
        strcat(bname, names[i]);

    }
}
