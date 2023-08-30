#line 2 "datastructures.c"
#include "datastructures.h"
#include "mstdio.h"
int checkBuffer(struct mstring *res, size_t bytes, char side)
{
	if (side == 'r')
	{
		if (bytes + res->e + 1 >= res->b)
		{
			size_t m = ((bytes + 1) / MSBUFSZ) + 1;
			res->b += (m*MSBUFSZ) * 2;
			char * temp = realloc(res->str, res->b);
			if (!temp)
				return 0;
			res->str = temp;
		}
	}
	else
	{
		if (res->s > bytes)
			return 1;
		size_t n = MSBUFSZ*((res->e - res->s + bytes + 1 + LEFTBUF) / MSBUFSZ + 1);
		char *temp = malloc(n);
		if (!temp)
			return 0;
		strcpy(temp + LEFTBUF + bytes, res->str + res->s);
		res->e += LEFTBUF + bytes - res->s;
		res->s = LEFTBUF + bytes;
		free(res->str);
		res->str = temp;
	}
	return 1;
}
char *concat(struct mstring * s, const char* nv, size_t len)
{
	if (!len)
		return s->str + s->s;

	if (s->e + len + 1 >= s->b)
		if (!checkBuffer(s, len, 'r'))
			return NULL;
	memcpy(&s->str[s->e], nv, len + 1);
	s->e += len;
	*(s->str + s->e) = '\0';

	return &s->str[s->s];
}

//#define concatc(g,n)  if(g->e+1>=g->b) checkBuffer(g,1,'r') ;g->str[g->e++]=n;g->str[g->e]='\0';
char *concatc(struct mstring * s, char nv)
{
	if (s->e + 2 >= s->b)
		if (!checkBuffer(s, 2, 'r'))
			return NULL;
	s->str[s->e++] = nv;
	s->str[s->e] = '\0';

	return &s->str[s->s];
}
char *concatf(struct mstring * s, double f)
{
	char   temp[30];
	sprintf(temp, TBNUMERICPATTERNPRINTF, f);
	size_t i = strlen(temp);
	if (s->e + i + 1 >= s->b)
		if (!checkBuffer(s, i, 'r'))
			return NULL;
	strcat(s->str + s->s, temp);
	s->e = s->e + i;
	return s->str + s->s;
}

char *concati(struct mstring * s, ll_64 f)
{
	char  temp[30];
	sprintf(temp, TBLONGLONGPATTERN, f);

	size_t i = strlen(temp);
	if (s->e + i + 1 >= s->b)
		if (!checkBuffer(s, i, 'r'))
			return NULL;
	strcat(&s->str[s->s], temp);
	s->e += i;
	return &s->str[s->s];
}
char *concatmint(struct mstring *s, mint_t m)
{
    char *curs,tmp[4]={0};
    int bc=uByteCount(m);
    curs=tmp;
    curs=uTocharConcatAndEat(curs,m);
    return concat(s,tmp,bc);

}


char * prepend(struct mstring *s, char *nv)
{
	size_t i = strlen(nv);
	if (--i > s->s)
		if (!checkBuffer(s, i, 'l'))
			return NULL;
	while (i >= 0)
		s->str[--s->s] = nv[i--];
	return &s->str[s->s];
}
char * prependf(struct mstring *s, double f)
{
	char   temp[30];
	sprintf(temp, TBNUMERICPATTERNPRINTF, f);

	return prepend(s, temp);

}
char * prependi(struct mstring *s, ll_64 u)
{
	char   temp[30];
	sprintf(temp, TBLONGLONGPATTERN, u);

	return prepend(s, temp);

}
char * setvalue(struct mstring * s, char* nv, unsigned len)
{
	s->s = LEFTBUF;
	s->e = LEFTBUF;
	s->str[s->e] = '\0';
	concat(s, nv, len);
	return &s->str[s->s];
}

void destroy_mstring(struct mstring * toDestroy)
{
	//printf("%d\n",--ninchia);
	if (toDestroy->str)
		free(toDestroy->str);
	free(toDestroy);
}
int compare(struct mstring * a, struct mstring *b)
{
	return strcmp(a->to_string(a), b->to_string(b));
}
struct mstring *new_mstring()
{
	//printf("%d\n",++ninchia);
	struct mstring *res = (struct mstring*) malloc(sizeof(struct mstring));
	if (!res)
		return NULL;
	res->str = (char*)malloc(sizeof(char)*MSBUFSZ);
	if (!res->str)
	{
		free(res);
		return NULL;
	}
	res->s = LEFTBUF;
	res->e = LEFTBUF;
	res->b = MSBUFSZ;
	res->str[res->e] = '\0';
	res->destroy = destroy_mstring;
	//res->to_string=substring;
	//res->concatc=concatc;
	res->concatf = concatf;
	res->concati = concati;
	res->prepends = prepend;
	// res->reset=reset;
	res->compare = compare;

	return res;
}



