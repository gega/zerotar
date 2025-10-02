#ifndef ZEROTAR_H
#define ZEROTAR_H

#include <string.h>
#include <stdint.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define ZEROTAR_CORE_HDR_CHUNK (136)
#define ZEROTAR_MAGIC_OFFSET (257)
#define ZEROTAR_TOTAL_HDR (512)
#define ZEROTAR_TAIL_HDR_CHUNK (ZEROTAR_TOTAL_HDR-ZEROTAR_CORE_HDR_CHUNK)
#define ZEROTAR_SIZE_OFFSET (124)
#define ZEROTAR_ALIGNMENT (512)
#define ZEROTAR_MAGIC_BACKOFFSET (ZEROTAR_TOTAL_HDR-ZEROTAR_MAGIC_OFFSET)
#define ZEROTAR_MAGIC_TAIL_OFFS (ZEROTAR_MAGIC_OFFSET-ZEROTAR_CORE_HDR_CHUNK)

#define ZEROTAR_FLAGS_FLATTEN (1<<6)
#define ZEROTAR_FLAGS_MAGIC_MASK (0x7)
#define ZEROTAR_FLAGS_EOF (1<<7)

#define ZEROTAR_ERR_INVALID_ARG (-1)
#define ZEROTAR_ERR_MAGIC (-2)

struct zerotar_s
{
  uint8_t mode;
  uint8_t hdr_pnt;
  int16_t skip;
  uint8_t hdr_raw[ZEROTAR_CORE_HDR_CHUNK];
  int32_t size;
  uint16_t flags;
  void *userdata;
  int (*f_open)(void *, const char *, int);
  int (*f_write)(void *, uint8_t *, int);
  int (*f_close)(void *);
};

static int zerotar(struct zerotar_s *zt, uint8_t *buf, int len);


#ifdef ZEROTAR_IMPLEMENTATION


#define ROUND_UP(n,incr) ((n) + ((incr) - (n) % (incr)) % (incr))


static inline int64_t zerotar_parse_octal(const char *oct)
{
  int64_t value=0;

  for(int c=0;c<12;c++,oct++)
  {
    if(*oct==' ') continue;
    if(*oct<'0'||*oct>'7') return(value);
    value=(value<<3)+(*oct-'0');
  }

  return(value);
}


int zerotar(struct zerotar_s *zt, uint8_t *buf, int len)
{
  static const char magic[]="ustar";
  int fill=0,n=0;
  
  if(NULL==zt) return(ZEROTAR_ERR_INVALID_ARG);
  if(len<0)
  {
    zt->skip=0;
    zt->hdr_pnt=0;
    zt->size=0;
    zt->mode=0;
    return(0);
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
        buf+=fill;
        if(zt->hdr_pnt>=sizeof(zt->hdr_raw))
        {
          const char *name=(const char *)zt->hdr_raw;
          if(zt->flags&ZEROTAR_FLAGS_FLATTEN)
          {
            const char *p,*s=name-1;
            for(p=name;*p!='\0';p++) if(*p=='/') s=p;
            name=s+1;
          }
          zt->size=0;
          if(*name!='\0')
          {
            zt->size=zerotar_parse_octal((const char *)&zt->hdr_raw[ZEROTAR_SIZE_OFFSET]);
            if(NULL!=zt->f_open) zt->f_open(zt->userdata, name, zt->size);
          }
          else zt->flags|=ZEROTAR_FLAGS_EOF;
          zt->skip=ZEROTAR_TAIL_HDR_CHUNK;
          zt->hdr_pnt=0;
          zt->mode=1;
        }
        break;
      case 1: // skip1
        n=MIN(len,zt->skip);
        if((zt->flags&ZEROTAR_FLAGS_MAGIC_MASK)!=0)
        {
          // remainder magic
          int l=MIN(n,(zt->flags&ZEROTAR_FLAGS_MAGIC_MASK));
          if(0!=memcmp(buf,&magic[sizeof(magic)-1-l],l))
          {
            zt->skip=zt->mode=0;
            return(ZEROTAR_ERR_MAGIC);
          }
          zt->flags&=~ZEROTAR_FLAGS_MAGIC_MASK;
        }
        zt->skip-=n;
        len-=n;
        buf+=n;
        fill+=n;
        if(0==(zt->flags&ZEROTAR_FLAGS_EOF)&&zt->skip<ZEROTAR_MAGIC_BACKOFFSET&&n>(sizeof(magic)/2))
        {
          int offs=ZEROTAR_MAGIC_TAIL_OFFS-(ZEROTAR_TAIL_HDR_CHUNK-zt->skip-n);
          if(offs>0&&offs<n)
          {
            int l=MIN(n-offs,sizeof(magic)-1);
            if(0!=memcmp(buf-n+offs, magic, l))
            {
              zt->skip=zt->mode=0;
              return(ZEROTAR_ERR_MAGIC);
            }
            if(l<sizeof(magic)-1) zt->flags+=sizeof(magic)-1-l;
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
          if(!(zt->flags&ZEROTAR_FLAGS_EOF)&&NULL!=zt->f_close) zt->f_close(zt->userdata);
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
  return(0);
}

#endif
#endif
