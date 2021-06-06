/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

// (1). Write C code to print values of argc and argv[] entries

  printf("argc=%x ", argc);
  for(int i = 0; i < argc; i++){
    printf("argv[%x]=%x ", i, argv[i]);
  }

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  printf("&d=%8x &e=%8x &f=%8x\n", &d, &e, &f);
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  printf("&g=%8x &h=%8x &i=%8x\n", &g, &h, &i);
  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  printf("&u=%8x &v=%8x &w=%8x &i=%8x &p=%8x\n", &u, &v, &w, &i, &p);
  u=10; v=11; w=12; i=13;

  FP = (int *)getebp();  // FP = stack frame pointer of the C() function
  printf("FP = %x\n", FP);

// (2). Write C code to print the stack frame link list.
  p = (int *)FP;
  
  printf("Stack Frame Linked List\n");
  while(p){
    printf("%x ->", p);
    p = *p;
  }
  
  printf("NULL\n");

  p = (int *)&p;
  p = p - 4;  // back up by 4 frames to see variables
// (3). Print the stack contents from p to the frame of main()
//      YOU MAY JUST PRINT 128 entries of the stack contents.
  for (int i = 0; i < 128; i++){
    printf("%x     %x\n", p, *p);
    p = p + 1;
  }

//(4). On a hard copy of the print out, identify the stack contents
//     as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
}



