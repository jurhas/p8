#ifndef _TBDEF_H_
#define _TBDEF_H_

#define TBDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#include <sys/stat.h>
#ifndef __cplusplus
//#include <libpq-fe.h>
#endif


#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif // ERROR


#define PI 3.14159265358979323846

#define mabs(a) ((a)<0?-(a):(a))
#define feps_0(a) ((a)<0.0000000001  && (a)>-0.0000000001?0:(a))
#define sgn(a) ((a)<0?-1:1)
#ifdef max //each time is a lottery with max. You never know who define it
#undef max
#endif // max
#define max(a,b) ((a)<(b)?(b):(a))
#ifdef min
#undef min
#endif
#define min(a,b) ((a)>(b)?(b):(a))
#define YES 1
#define NO 0
#define TRUE 1
#define FALSE 0


typedef long long ll_64;
typedef unsigned long long ull_64;
//#if (defined MSC_VER)  || (defined __BORLANDC__)
//  typedef __int64 ll;
//  typedef unsigned __int64 ull;
//#else
//
//#endif

typedef ll_64 tbix_t ;
typedef double tbnumeric_t;
#if defined(_MSC_VER)
#pragma warning ( disable : 4996)
#define BNLONGLONGPATTERN "%lld"
#define BNULONGLONGPATTERN "%llu"

#else
#define BNLONGLONGPATTERN "%I64d"
#define BNULONGLONGPATTERN "%I64u"


#endif
#define TBNUMERICPATTERNSCANF "%lf"
#define TBNUMERICPATTERNPRINTF "%0.9lg"
typedef ll_64 tbinteger_t;
#define TBINTEGERPATTERN "%I64d"

#define TBLONGLONGPATTERN "%I64d"
#define TBULONGLONGPATTERN "%I64u"

#endif // _TBDEF_H_
