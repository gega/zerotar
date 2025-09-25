#include <stdio.h>
#include <stdlib.h>

#define ZEROTAR_IMPLEMENTATION
#include "zerotar.h"


#define BUFSIZE (17)


struct ud_s
{
  FILE *f;
};


static int f_open(void *ud, const char *name, int size)
{
  struct ud_s *u=(struct ud_s *)ud;
  u->f=fopen(name,"wb");
  return(0);
}


int f_write(void *ud, uint8_t *b, int l)
{
  struct ud_s *u=(struct ud_s *)ud;
  fwrite(b,1,l,u->f);
  return(0);
}


int f_close(void *ud)
{
  struct ud_s *u=(struct ud_s *)ud;
  fclose(u->f);
  return(0);
}


int main(int argc, char **argv)
{
  FILE *f;
  struct zerotar_s zts;
  unsigned char buf[BUFSIZE];
  struct ud_s uds;
  int st;
  
  memset(&zts, 0, sizeof(zts));
  zts.f_open=f_open;
  zts.f_write=f_write;
  zts.f_close=f_close;
  zts.flags=ZEROTAR_FLAGS_FLATTEN;
  zts.userdata=&uds;
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
