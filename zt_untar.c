/*
    BSD 2-Clause License

    Copyright (c) 2025, Gergely Gati

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
