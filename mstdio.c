#include "mstdio.h"
#include "stdio.h"
#include "assert.h"
#if (defined(_WIN32) || defined(WIN32) || 1 )
#include <windows.h>
MFILE _mio[2]={ {0,0,0,0,0}};
int u8_toucsalloc(mint_t **dest,size_t *dsz, char *src, int srcsz,size_t *len)
{
    mint_t  *ch;
    unsigned char mycode[4];
    char*src_end=src+srcsz;
    if(srcsz==0) return 0;
    int is_alloc=*dest==NULL ;
    if(! *dest)
    {
        *dsz=64;
        *dest=malloc(sizeof(mint_t)**dsz );
        if(!*dest) return 0;
    }
    register mint_t *curi =*dest;
    int nb;
    int i=0;
    *len=0;
    while( (is_alloc ||i< *dsz-1) && src<src_end)
    {
        nb = trailingBytesForUTF8(*src);
        ch=(mint_t*)mycode;

        *ch=0;
        if(is_alloc && i>=*dsz-1)
        {
            *dsz<<=1;
            curi=realloc(*dest,sizeof(mint_t)**dsz);
            if(!curi)
            {
                free(dest);
                return 0;
            }
            *dest=curi;
            curi+=i;
        }
        switch(nb)
        {
            case 3:mycode[0]=*src++;
            case 2:mycode[1]=*src++;
            case 1:mycode[2]=*src++;
            case 0:mycode[3]=*src++;
            break;
            default:assert(1);
        }
        i++;
        *curi++=*ch;
    }
    *curi=0;
    *len=i;
    return i;
}
const mint_t*mstrchr(const mint_t* arr,mint_t a )
{
    while(*arr && *arr!=a)
        arr++;
    return *arr? arr:NULL;
}
int u8_toucs(mint_t *dest, int sz, char *src, int srcsz)
{
    mint_t  *ch;
    unsigned char mycode[4];
    unsigned char *src_end=(unsigned char *)src+srcsz;
    int nb;
    int i=0;
    while(i<sz-1 && src< (char*)src_end)
    {
        nb = trailingBytesForUTF8((unsigned char)*src);
        ch=(mint_t*)mycode;
        *ch=0;
        switch(nb)
        {
            case 3:mycode[0]=*src++;
            case 2:mycode[1]=*src++;
            case 1:mycode[2]=*src++;
            case 0:mycode[3]=*src++;
            break;
            default:assert(1);
        }
        i++;
        *dest++=*ch;
    }
    *dest=0;
    return i;
}

mint_t mfill()
{
    DWORD cNumRead;


    static wchar_t * lbuf;
    static size_t sz;
    size_t n=0;
    static muchar_t *cbuf;
    static size_t chsz;
    size_t chn=0;
    wchar_t *wcurs;
    if (!sz)
    {
        sz=100;
        lbuf=calloc(sz,sizeof(wchar_t));
    }
    if (!mstdin->base)
    {
        mstdin->bssize=1024;
        mstdin->ptr=mstdin->base=malloc(mstdin->bssize*sizeof(mint_t));
        if(!mstdin->base )
            return 0;
        mstdin->sio=GetStdHandle(STD_INPUT_HANDLE);
        mstdout->sio=GetStdHandle(STD_OUTPUT_HANDLE);
        if (INVALID_HANDLE_VALUE==mstdin->sio)
        {
            printf("ERR:Cannot Get Stdin\n");
            return 0;
        }
    }
    int flag=1;
    wcurs=lbuf;
    DWORD rdcnt=sz;
    while(flag)
    {
        if(!ReadConsoleW(mstdin->sio,wcurs,rdcnt,&cNumRead,NULL)) return 0;
        if(cNumRead<2)
        {
            n+=cNumRead;
            flag=0;
        }
        else if(  wcurs[cNumRead-1]=='\n')
        {
            if(wcurs[cNumRead-2]=='\r' )
                wcurs[cNumRead---2]='\n';

            n+=cNumRead;
            flag=0;
        }
        else
        {
            rdcnt=sz;
            sz<<=1;
            wchar_t *tmp;
            tmp=realloc(lbuf,sz*sizeof(wchar_t));
            if(!tmp) return 0;
                lbuf=tmp;
            n+=cNumRead;
            wcurs=lbuf+n;

        }
    }
    chn=wctoutfllen(lbuf,n);
    if(chn>=chsz)
    {
        chsz=chsz+chn+1;
        cbuf=realloc(cbuf,sizeof(muchar_t)*chsz );
        if(!cbuf) return 0;
    }

    mstdin->cnt=n;
    if(mstdin->cnt>=mstdin->bssize)
    {
        mstdin->bssize+=mstdin->bssize+mstdin->cnt;
        mint_t *tmp=realloc(mstdin->base,sizeof(mint_t)*mstdin->bssize);
        if(!tmp) return 0;
        mstdin->base=tmp;
    }
    mstdin->ptr=mstdin->base;
    int cnvcnt=wctoutfconv(lbuf,n,(LPSTR)cbuf,chn);
    if( cnvcnt!=chn)
    {
        return 0;
    }
    u8_toucs(mstdin->base, mstdin->bssize,(char*) cbuf, chn);
    mstdin->cnt--;
    return  *mstdin->ptr++;

}

int mprintf( muchar_t* str )
{
    if(!mstdout->sio)
    {
        mstdout->sio=GetStdHandle(STD_OUTPUT_HANDLE);
        if (INVALID_HANDLE_VALUE==mstdout->sio)
        {
            printf("ERR:Cannot get stdout\n");
            mstdout->sio=NULL;
            return 0;
        }
    }
    size_t ln=mbtowlen((LPCSTR)str);
    wchar_t * tmp=malloc(sizeof(wchar_t)*ln);
    mbtowconv((LPCSTR)str,tmp,ln);
    DWORD some;
    WriteConsoleW(mstdout->sio,tmp,(DWORD)ln,&some,NULL);
    return 1;

}

static muchar_t *reallocutfbuf(muchar_t* buf, size_t *sz)
{
    *sz=((*sz>>3)+1)<<4;
    muchar_t *tmp=realloc(buf,sizeof(muchar_t)**sz );
    if(!tmp) return NULL;
    return tmp;
}

/// lnbuf reallocate if length, sz update if reallocate, n return the actually count of charachters, len the utf8 length
muchar_t * mgets(muchar_t* lnbuf,size_t* sz,size_t *n,size_t *len)
{
    *n=0;
    *len=0;
    if(lnbuf==NULL)
    {
        *sz=((mstdin->cnt>>3)+1)<<4;
        lnbuf=malloc(sizeof(muchar_t)**sz );
        if(!lnbuf) return NULL;
    }
    mint_t  tmp;

    int bcnt;
    int flag=1;
    while( flag)
    {
        tmp=mgetchar();
        if(!tmp) return NULL;

        (*len)++;

        bcnt=uByteCount(tmp);

        if(*n+bcnt+1>=*sz)
        {
            lnbuf=reallocutfbuf(lnbuf,sz);
            if(!lnbuf) return NULL;
        }
        switch(bcnt)
        {
        case 4:
            lnbuf[(*n)++]=uGetByte(tmp,0);
        case 3:
            lnbuf[(*n)++]=uGetByte(tmp,1);
        case 2:
            lnbuf[(*n)++]=uGetByte(tmp,2);
        case 1:
            if ( ((lnbuf[(*n)++]=uGetByte(tmp,3))=='\n')  && bcnt==1) flag=0 ;
            break;
        default :
            assert(1);

        }

    }
    lnbuf[*n]='\0';
    return lnbuf;
}
#else
muchar_t * mgets(muchar_t* lnbuf,size_t* sz,size_t *n,size_t *len)
{
}

#endif

mint_t minttoupper(mint_t mn)
{
    mint_t uv,res;
    uv=umintGetUnicodeValue(mn);
    if(uv<(1<<16) )
        return mutftomint(ToUpperLUT+(uv<<2),res);
    else
        return mn;
}
mint_t minttolower(mint_t mn)
{
    mint_t uv,res;
    uv=umintGetUnicodeValue(mn);
    if(uv<(1<<16) )
        return mutftomint(ToUpperLUT+(uv<<2),res);
    else
        return mn;
}


