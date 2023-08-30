#ifndef _DATASTRUCTURES_H_
#define _DATASTRUCTURES_H_
#include "tbdef.h"
#include "mstdio.h"
#include <string.h>

#define USE_EXTERN_DEFINITIONS 1

#if (USE_EXTERN_DEFINITIONS)

typedef long long ll;
typedef unsigned long long ull;
#else
#include <stdio.h>
#include<stddef.h>
#include<stdlib.h>
#ifdef  defined ((_MSC_VER) || defined(__BORLANDC__))
  typedef __int64 ll;
  typedef unsigned __int64 ull;
#else
typedef long long int ll;
typedef unsigned long long int ull;
#endif

#endif

#define MSBUFSZ 4096
#define LEFTBUF 20

typedef struct mstring
{
	char * str;
	size_t  b;
	size_t e;
	size_t s;
	char* (*to_string)(struct mstring*);
	void(*destroy)(struct mstring*);
	char* (*concats)(struct mstring *, char*);
	char* (*concatm)(struct mstring *, struct mstring *);
	char* (*concatc)(struct mstring *, char);
	char* (*concatf)(struct mstring *, double);
	char* (*concati)(struct mstring *, ll_64);
	char* (*prepends)(struct mstring *, char*);
	void(*reset)(struct mstring *);
	int(*compare)(struct mstring *, struct mstring *);

}*ptbString;
char *concat(struct mstring * , const char* , size_t );
char *concatc(struct mstring *, char);
char *concatf(struct mstring *, double);
char *concati(struct mstring *, ll_64);
char *concatmint(struct mstring *s, mint_t );
char * prepend(struct mstring *, char *);
char * prependf(struct mstring *, double);
char * setvalue(struct mstring *, char*, unsigned);


int compare(struct mstring *, struct mstring *);
char * prependi(struct mstring *s, ll_64 u);
int checkBuffer(struct mstring *, size_t, char);
struct mstring *new_mstring();
void destroy_mstring(struct mstring*);

//#define msreset(g)
#define msreset(g) ((g)->s=LEFTBUF,(g)->e=LEFTBUF,(g)->str[(g)->s]='\0')
#define concatm(a,b)concat(a,(b)->str+(b)->s,(b)->e-(b)->s)
#define concats(a,b)concat(a,b,strlen(b))
#define substring(g) &g->str[g->s]
#define mstostring(g) ((g)->str+(g)->s)
#define mslen(g) ( ((g)->e)-((g)->s))
#endif // _DATASTRUCTURES_H_
