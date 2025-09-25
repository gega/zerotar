#include <stdio.h>
#include <stdlib.h>

#define ZEROTAR_IMPLEMENTATION
#include "zerotar.h"


#define BUFSIZE (13)


static int f_open(void *ud, const char *name, int size)
{
  printf("%d %s\n", size, name);
  return(0);
}

int main(int argc, char **argv)
{
  FILE *f;
  struct zerotar_s zts;
  unsigned char buf[BUFSIZE];
  int st;
  
  memset(&zts, 0, sizeof(zts));
  zts.f_open=f_open;
  zts.flags=0;
  if(argc<2)
  {
    printf("Usage: %s file.tar\n",argv[0]);
    exit(0);
  }
  f=fopen(argv[1],"rb");
  do
  {
    st=fread(buf,1,sizeof(buf),f);
    zerotar(&zts,buf,st);
  }
  while(st>0);
  fclose(f);
  return(0);
}
