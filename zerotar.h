#ifndef ZEROTAR_H
#define ZEROTAR_H

#include <string.h>
#include <stdint.h>


#define ZEROTAR_CORE_HDR_CHUNK (136)
#define ZEROTAR_MAGIC_OFFSET (257)
#define ZEROTAR_TOTAL_HDR (512)
#define ZEROTAR_TAIL_HDR_CHUNK (ZEROTAR_TOTAL_HDR-ZEROTAR_CORE_HDR_CHUNK)
#define ZEROTAR_SIZE_OFFSET (124)
#define ZEROTAR_ALIGNMENT (512)


struct zerotar_s
{
  int mode;
  uint8_t hdr_raw[ZEROTAR_CORE_HDR_CHUNK];
  uint8_t hdr_pnt;
  void *userdata;
  int skip;
  int size;
  int basename;
  int (*f_open)(void *, const char *);
  int (*f_write)(void *, uint8_t *, int);
  int (*f_close)(void *);
};

static void zerotar(struct zerotar_s *zt, uint8_t *buf, int len);

#ifdef ZEROTAR_IMPLEMENTATION

#define ROUND_UP(n,incr) ((n) + ((incr) - (n) % (incr)) % (incr))

static inline int zerotar_parse_octal(const char *oct)
{
  int value=0;
  char c;

  while((c=*oct++)!='\0')
  {
    if(c<'0'||c>'7') return(-1);
    value=(value<<3)+(c-'0');
  }

  return(value);
}


void zerotar(struct zerotar_s *zt, uint8_t *buf, int len)
{
  static const char magic[]={'u','s','t','a','r'};
  static const int magic_offset=ZEROTAR_TAIL_HDR_CHUNK-ZEROTAR_MAGIC_OFFSET;
  int fill=0,n=0;
  
  if(NULL==zt) return;
  if(zt->size<0)
  {
    zt->skip=0;
    zt->hdr_pnt=0;
    zt->size=0;
    zt->mode=0;
    return;
  }
  while(len>0)
  {
    switch(zt->mode)
    {
      case 0: // hdr
        fill=MIN(len,(sizeof(zt->hdr_raw)-zt->hdr_pnt));
        memcpy(&zt->hdr_raw[zt->hdr_pnt],buf,fill);
        zt->hdr_pnt+=fill;
        len-=fill;
        if(zt->hdr_pnt>=sizeof(zt->hdr_raw))
        {
          const char *name=(const char *)zt->hdr_raw;
          if(zt->basename)
          {
            const char *p,*s=name-1;
            for(p=name;*p!='\0';p++) if(*p=='/') s=p;
            name=s+1;
          }
          zt->size=0;
          if(*name!='\0')
          {
            if(NULL!=zt->f_open) zt->f_open(zt->userdata, name);
            zt->size=parse_octal((const char *)&zt->hdr_raw[ZEROTAR_SIZE_OFFSET]);
          }
          zt->skip=ZEROTAR_TAIL_HDR_CHUNK;
          zt->hdr_pnt=0;
          zt->mode=1;
        }
        break;
      case 1: // skip1
        n=MIN(len,zt->skip);
        zt->skip-=n;
        len-=n;
        buf+=n;
        fill+=n;
        if(zt->skip<magic_offset&&n>sizeof(magic))
        {
          if(memcmp(buf-magic_offset-zt->skip,magic,sizeof(magic))!=0)
          {
            // MAGIC mismatch
            zt->skip=0;
            zt->mode=0;
            break;
          }
        }
        if(zt->skip<=0)
        {
          zt->skip=ROUND_UP(zt->size,ZEROTAR_ALIGNMENT)-zt->size;
          zt->mode=2;
        }
        break;
      case 2: // write
        n=MIN(len,zt->size);
        zt->size-=n;
        if(NULL!=zt->f_write) zt->f_write(zt->userdata, buf, n);
        len-=n;
        buf+=n;
        fill+=n;
        if(zt->size<=0)
        {
          if(NULL!=zt->f_close) zt->f_close(zt->userdata);
          zt->mode=3;
          zt->size=0;
        }
        break;
      case 3: // skip2
        n=MIN(len,zt->skip);
        zt->skip-=n;
        fill+=n;
        len-=n;
        buf+=n;
        if(zt->skip<=0)
        { 
          zt->mode=0;
          zt->skip=0;
        }
        break;
    }
  }
}

#endif
#endif
