#ifndef _MSTDIO_H_
#define _MSTDIO_H_
#include <stddef.h>

#if (defined(_WIN32) || defined(WIN32) || 1 )
#include <windows.h>
extern const unsigned char ToLowerLUT[];
extern const unsigned char ToUpperLUT[];
typedef unsigned int mint_t;
typedef unsigned char muchar_t;
typedef struct _miobuf
{
    int cnt;
    mint_t *ptr;
    mint_t *base;
    size_t bssize;
    HANDLE sio;
} MFILE;
extern MFILE _mio[2];
mint_t mfill();
#define mstdin (&_mio[0])
#define mstdout (&_mio[1])
#define wctoutfllen(buf,len)WideCharToMultiByte(CP_UTF8,0,buf,len,0,0,0,0)
#define wctoutfconv(wbuf,wlen,cbuf,clen)WideCharToMultiByte(CP_UTF8,0,wbuf,wlen,cbuf,clen,0,0)
#define mbtowlen(buf) MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0)
#define mbtowconv(buf,dest_buf,n) MultiByteToWideChar(CP_UTF8, 0, buf, -1, dest_buf,n)
#endif
#define uGetByte(a,b) (*(((muchar_t*)(&a))+(b)))
#define uSetByte(a,b,value) (*(((muchar_t*)  (&a))+(b))=(value))
#define uByteCount(a) (uGetByte(a,0)?4:uGetByte(a,1)?3:uGetByte(a,2)?2:1)

#define trailingBytesForUTF8(a) ((unsigned char)(a)<0X80?0:(unsigned char)(a)<0XE0?1:(unsigned char)(a)<0XF0?2:3)

#define mgetchar() (mstdin->cnt--? *mstdin->ptr++:mfill())

#define mutftomint(a,mint) (mint=0,trailingBytesForUTF8(*(a))==3? (uSetByte(mint,0,*(a)),uSetByte(mint,1,*(a+1)),\
                                                                    uSetByte(mint,2,*(a+2)),uSetByte(mint,3,*(a+3)),mint):\
                                    trailingBytesForUTF8(*(a))==2?(uSetByte(mint,1,*(a)),uSetByte(mint,2,*(a+1)),\
                                                                    uSetByte(mint,3,*(a+2)),mint):\
                                    trailingBytesForUTF8(*(a))==1?(uSetByte(mint,2,*(a)),uSetByte(mint,3,*(a+1)),mint):\
                                    (uSetByte(mint,3,*(a)),mint))
#define uGetASCIIvalue(a) (uByteCount(a)==1?uGetByte(a,3):0XFF)
#define uTocharConcatAndEat(charptr,mint) ( uByteCount(mint)==4? ((*charptr++=uGetByte(mint,0)),(*charptr++=uGetByte(mint,1)  ),\
                                                                  (*charptr++=uGetByte(mint,2)),(*charptr++=uGetByte(mint,3))):\
                                           uByteCount(mint)==3? ((*charptr++=uGetByte(mint,1)),(*charptr++=uGetByte(mint,2)  ),\
                                                                  (*charptr++=uGetByte(mint,3))):\
                                           uByteCount(mint)==2? ((*charptr++=uGetByte(mint,2)),(*charptr++=uGetByte(mint,3)  )):\
                                                                  (*charptr++=uGetByte(mint,3)),charptr)
#define uGetUnicodeValue(chrptr) (trailingBytesForUTF8(*(chrptr))==0? ((mint_t) *(chrptr)) :trailingBytesForUTF8(*(chrptr))==1\
                             ?(( ((mint_t)*(chrptr)&0X1F)<<6)+( ((mint_t)*(chrptr+1)&0X3F))): trailingBytesForUTF8(*(chrptr))==2?  \
                                (( ((mint_t)*(chrptr)&0XF)<<12)+( ((mint_t)*(chrptr+1)&0X3F)<<6 )+( ((mint_t)*(chrptr+2)&0X3F))   ): \
                                  ((((mint_t)*(chrptr)&0X7)<<18)+( ((mint_t)*(chrptr+1)&0X3F)<<6 )+ ( ((mint_t)*(chrptr+2)&0X3F))+ ( ((mint_t)*(chrptr+3)&0X3F))))
#define mutftomintAndEat(a,mint) (mint=0,trailingBytesForUTF8(*(a))==3? (uSetByte(mint,0,*(a++)),uSetByte(mint,1,*(a++)),\
                                                                    uSetByte(mint,2,*(a++)),uSetByte(mint,3,*(a++)),mint):\
                                    trailingBytesForUTF8(*(a))==2?(uSetByte(mint,1,*(a++)),uSetByte(mint,2,*(a++)),\
                                                                    uSetByte(mint,3,*(a++)),mint):\
                                    trailingBytesForUTF8(*(a))==1?(uSetByte(mint,2,*(a++)),uSetByte(mint,3,*(a++)),mint):\
                                    (uSetByte(mint,3,*(a++)),mint))
#define umintGetUnicodeValue(mint) (uByteCount(mint)==1? uGetByte(mint,3) :uByteCount(mint)==2 \
                             ?(( (uGetByte(mint,2)&0X1F)<<6)+( (uGetByte(mint,3)&0X3F))): uByteCount(mint)==3?  \
                                (( (uGetByte(mint,1)&0XF)<<12)+( (uGetByte(mint,2)&0X3F)<<6 )+( (uGetByte(mint,3)&0X3F))   ): \
                                  (((uGetByte(mint,0)&0X7)<<18)+( (uGetByte(mint,1)&0X3F)<<6 )+ ( (uGetByte(mint,2)&0X3F))+ ( (uGetByte(mint,3)&0X3F))))
#define mtoupper(chrptr) ( uGetUnicodeValue(chrptr)<(1<<16)? *(mint_t*)(ToUpperLUT+(uGetUnicodeValue(chrptr)<<2)):*(mint_t*)chrptr)

#define mtolower(chrptr) ( uGetUnicodeValue(chrptr)<(1<<16)? *(mint_t*)(ToLowerLUT+(uGetUnicodeValue(chrptr)<<2)):*(mint_t*)chrptr)

#define mmovebck(chrptr) ( (chrptr)--, ( *(chrptr)&0XC0)!=0X80?(chrptr):\
                          ( (chrptr)--, ( *(chrptr)&0XC0)!=0X80?(chrptr):\
                           ( (chrptr)--, ( *(chrptr)&0XC0)!=0X80?(chrptr):--(chrptr))))
#define mmovefwd(chrptr) (chrptr+=trailingBytesForUTF8(*(chrptr))+1)
int mprintf( muchar_t* );
muchar_t * mgets(muchar_t*,size_t*,size_t *,size_t * );
mint_t mputchar(mint_t);
const mint_t*mstrchr(const mint_t*,mint_t);
mint_t minttoupper(mint_t);
mint_t minttolower(mint_t);
int u8_toucsalloc(mint_t **,size_t *, char *, int ,size_t *);

#endif // _MSTDIO_H_
