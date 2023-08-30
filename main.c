#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "windows.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "time.h"
#include "datastructures.h"
#include "libpq-fe.h"
#include <windows.h>


//#include <libpq-fe.h>
typedef struct _sp8
{
    char host[100];
    char port[10];
    char user[100];
    char db[100];
    char *pwd;
    PGconn * pg;
    PGresult *pgr;
    FILE *out;
    int is_custom_out_file;
    struct mstring *s;

}sp8;
typedef enum _cmd_out
{
    CMD_quit,
    CMD_load_input_file,
    CMD_default
}CMD_OUT;


char *tb_read_next(struct mstring *s,char *prompt)
{
    mint_t c;
    msreset(s);
    int status = 0;
    printf("%s>",prompt);

    char *u8s;

    while ((c = mgetchar()) != EOF && !(status&& uGetASCIIvalue(c) == '\n'))
    {
        checkBuffer(s,uByteCount(c),'r');
        u8s=s->str+s->e;
        (void)uTocharConcatAndEat(u8s,c);
        *u8s='\0';
        s->e+=uByteCount(c);

        status = (uGetASCIIvalue(c) == '\n');// ||uGetASCIIvalue(c) == '\r') ;
        if(status)
            printf("%s>",prompt);
    }
    if(s->e>s->s)
    {
        while(s->e>s->s && isspace(*(s->str+s->e-1) ))
        {
            s->e--;
            s->str[s->e]='\0';
        }

    }

    return mstostring(s);
}
void disp_tup(sp8 * p8)
{
    int i,j, *lens,n,m,len,mx=0;
    PGresult *res=p8->pgr;
    FILE *f =p8->out;
    //PQprintTuples(res,p8->out,1,1,0);

    n=PQnfields(res);
    lens=calloc(n,sizeof(int));
    for(j=0;j<n;j++)
    {
        len=mbtowlen(PQfname(res,j));
        lens[j]=lens[j]<len ?len: lens[j];
    }
    m=PQntuples(res);
    for (i=0;i<n;i++ )
        for(j=0;j<m;j++)
        {
            //len=strlen(PQgetvalue(res,j,i));
            len=mbtowlen(PQgetvalue(res,j,i));

            lens[i]=lens[i]<len?len: lens[i];
        }
    mx=0;
    for(i=0;i<n;i++)
        mx+=lens[i];
  for(i=0;i<n;i++)
  {
      len=mbtowlen(PQfname(res,i));
      fprintf(f,PQfname(res,i));
      for(j=len;j<lens[i];j++)
        fprintf(f," ");
    fprintf(f,"|");
  }
  fprintf(f,"\n");
 for(i=0;i<mx;i++)
    fprintf(f,"-");
//printf("\n");
    for (j=0;j<m;j++)
  {
      fprintf(f,"\n");
        for (i=0;i<n;i++)
        {
            int k;
            //len=strlen(PQgetvalue(res,j,i));
            len=mbtowlen(PQgetvalue(res,j,i));
            fprintf(f,PQgetvalue(res,j,i));
            for(k=len;k<lens[i];k++)
                fprintf(f," ");
            fprintf(f,"|");

        }
  }

  fprintf(f,"\n");
   for(i=0;i<mx;i++)
    fprintf(f,"-");
  fprintf(f,"\n");
  fprintf(f,"(%d records)",PQntuples(res));
  fprintf(f,"\n");
  if(p8->is_custom_out_file)
    fflush(f);
  free(lens);


}

char *
simple_prompt(const char *prompt, int maxlen, char echo)
{
	int			length;
	char	   *destination;
	FILE	   *termin,
			   *termout;

#ifdef HAVE_TERMIOS_H
	struct termios t_orig,
				t;
#else
#ifdef WIN32
	HANDLE		t = NULL;
	LPDWORD		t_orig = NULL;
#endif
#endif

	destination = (char *) malloc(maxlen + 1);
	if (!destination)
		return NULL;

#ifdef WIN32

	/*
	 * A Windows console has an "input code page" and an "output code page";
	 * these usually match each other, but they rarely match the "Windows ANSI
	 * code page" defined at system boot and expected of "char *" arguments to
	 * Windows API functions.  The Microsoft CRT write() implementation
	 * automatically converts text between these code pages when writing to a
	 * console.  To identify such file descriptors, it calls GetConsoleMode()
	 * on the underlying HANDLE, which in turn requires GENERIC_READ access on
	 * the HANDLE.  Opening termout in mode "w+" allows that detection to
	 * succeed.  Otherwise, write() would not recognize the descriptor as a
	 * console, and non-ASCII characters would display incorrectly.
	 *
	 * XXX fgets() still receives text in the console's input code page.  This
	 * makes non-ASCII credentials unportable.
	 */
	termin = fopen("CONIN$", "r");
	termout = fopen("CONOUT$", "w+");
#else

	/*
	 * Do not try to collapse these into one "w+" mode file. Doesn't work on
	 * some platforms (eg, HPUX 10.20).
	 */
	termin = fopen("/dev/tty", "r");
	termout = fopen("/dev/tty", "w");
#endif
	if (!termin || !termout
#ifdef WIN32

	/*
	 * Direct console I/O does not work from the MSYS 1.0.10 console.  Writes
	 * reach nowhere user-visible; reads block indefinitely.  XXX This affects
	 * most Windows terminal environments, including rxvt, mintty, Cygwin
	 * xterm, Cygwin sshd, and PowerShell ISE.  Switch to a more-generic test.
	 */
		|| (getenv("OSTYPE") && strcmp(getenv("OSTYPE"), "msys") == 0)
#endif
		)
	{
		if (termin)
			fclose(termin);
		if (termout)
			fclose(termout);
		termin = stdin;
		termout = stderr;
	}

#ifdef HAVE_TERMIOS_H
	if (!echo)
	{
		tcgetattr(fileno(termin), &t);
		t_orig = t;
		t.c_lflag &= ~ECHO;
		tcsetattr(fileno(termin), TCSAFLUSH, &t);
	}
#else
#ifdef WIN32
	if (!echo)
	{
		/* get a new handle to turn echo off */
		t_orig = (LPDWORD) malloc(sizeof(DWORD));
		t = GetStdHandle(STD_INPUT_HANDLE);

		/* save the old configuration first */
		GetConsoleMode(t, t_orig);

		/* set to the new mode */
		SetConsoleMode(t, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
	}
#endif
#endif

	if (prompt)
	{
		fputs(prompt, termout);
		fflush(termout);
	}

	if (fgets(destination, maxlen + 1, termin) == NULL)
		destination[0] = '\0';

	length = strlen(destination);
	if (length > 0 && destination[length - 1] != '\n')
	{
		/* eat rest of the line */
		char		buf[128];
		int			buflen;

		do
		{
			if (fgets(buf, sizeof(buf), termin) == NULL)
				break;
			buflen = strlen(buf);
		} while (buflen > 0 && buf[buflen - 1] != '\n');
	}

	if (length > 0 && destination[length - 1] == '\n')
		/* remove trailing newline */
		destination[length - 1] = '\0';

#ifdef HAVE_TERMIOS_H
	if (!echo)
	{
		tcsetattr(fileno(termin), TCSAFLUSH, &t_orig);
		fputs("\n", termout);
		fflush(termout);
	}
#else
#ifdef WIN32
	if (!echo)
	{
		/* reset to the original console mode */
		SetConsoleMode(t, *t_orig);
		fputs("\n", termout);
		fflush(termout);
		free(t_orig);
	}
#endif
#endif

	if (termin != stdin)
	{
		fclose(termin);
		fclose(termout);
	}

	return destination;
}
char* p8_get_schema_list(sp8 * p8,  struct mstring * s)
 {
        char *ch_t;
        PGresult *pgr;
        msreset(s);

        pgr=PQexec(p8->pg,"SHOW search_path");
        ch_t=PQgetvalue(pgr,0,0);
        while(*ch_t)
        {
            concatc(s,'\'');
            while(*ch_t && *ch_t !=',')
            {
                concatc(s,*ch_t);
                ch_t++;
            }
            concatc(s,'\'');
            if(*ch_t)
                concatc(s,',');
            while(isspace(*ch_t)|| *ch_t==',')
                ch_t++;

        }
        PQclear(pgr);
return mstostring(s);

 }
CMD_OUT cmd_c(sp8 *p8)
{
    char cn_str[1000],*cmd;
    CMD_OUT res=CMD_default;
    PGconn *pg;
    cmd=mstostring(p8->s)+2;
    while(isspace(*cmd) && *cmd!=0)
        cmd++;
    if(*cmd=='\0')
    {
        printf("No database specified\n");
        return res;
    }

    sprintf(cn_str,"host=%s port=%s dbname=%s user=%s password=%s",
            p8->host,p8->port,cmd,p8->user,p8->pwd);
    pg=PQconnectdb(cn_str);
    if (PQstatus(pg) != CONNECTION_OK)
    {
            printf("\n ERR:%s\n Previous connection kept\n",PQerrorMessage(pg));
            PQfinish(pg);
    }
    else
    {
        PQfinish(p8->pg);
        p8->pg=pg;
        strcpy(p8->db,cmd);
    }

    return res;
}
CMD_OUT cmd_df(sp8 *p8)
{
    char *cmd;
    struct mstring *tmp=new_mstring();
    cmd= mstostring(p8->s)+3;
    while(isspace(*cmd) &&*cmd!='\n' )
        cmd++;
    if(*cmd=='\0')
    {
        concats(tmp,"SELECT proname Name, pg_get_function_result( oid ) \"Result Data Type\"," );
        concats(tmp,"pg_get_function_arguments(oid) \"Argument Data Types\"\n" );
        concats(tmp,"FROM pg_proc WHERE pronamespace='public'::regnamespace ORDER BY proname" );

        p8->pgr=PQexec(p8->pg,mstostring(tmp));
        disp_tup(p8);
        PQclear(p8->pgr);
        p8->pgr=NULL;
        destroy_mstring(tmp);
        return CMD_default;
    }
    concats(tmp,"SELECT proname Name, pg_get_function_result( oid ) \"Result Data Type\"," );
    concats(tmp,"pg_get_function_arguments(oid) \"Argument Data Types\"\n" );
    concats(tmp,"FROM pg_proc WHERE pronamespace='public'::regnamespace AND proname='");
    concats(tmp,cmd);
     concats(tmp,"' ORDER BY proname" );

    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    disp_tup(p8);
    PQclear(p8->pgr);
    p8->pgr=NULL;
    destroy_mstring(tmp);
    return CMD_default;


}
CMD_OUT cmd_d(sp8 *p8)
{
    char *cmd;
    struct mstring *tmp=new_mstring() ,*t2;

    cmd= mstostring(p8->s)+2;
    switch(*cmd)
    {
        case 'f':
            return cmd_df(p8);
        case ' ' :
        case '\0':
            break;
        default:
            printf("Command not recognized\n");
            return CMD_default;

    }
    while (isspace(*cmd) && *cmd!='\0')
        cmd++;
    if(*cmd=='\0')
    {
        msreset(p8->s);
        p8_get_schema_list(p8,tmp);
        //concats(p8->s,  )
        concats(p8->s, "SELECT n.nspname \"Schema\" , c.relname Name, CASE c.relkind WHEN 'r' THEN 'Table'\n");
        concats(p8->s, "WHEN 'v' THEN 'View'\n");
        concats(p8->s, "WHEN 'S' THEN 'Sequence'\n");
        concats(p8->s, "END \"Type\"\n");
        concats(p8->s, "FROM pg_class AS c\n");
        concats(p8->s, "INNER JOIN pg_namespace AS n ON c.relnamespace=n.oid\n");
        concats(p8->s, "WHERE n.nspname in (");
        concat(p8->s,mstostring(tmp),tmp->e-tmp->s);
        concats(p8->s, ") AND c.relkind in ('r','v','S') ORDER BY relname ASC");
        p8->pgr=PQexec(p8->pg,mstostring(p8->s));
        disp_tup(p8);
        PQclear(p8->pgr);
        destroy_mstring(tmp);
        return CMD_default;

    }
    msreset(tmp);
    concats(tmp,"SELECT a.attname as Name, t.typname as Type ,CASE a.attnotnull WHEN true then 'Not Null' ELSE '' END as Nullable,\n");
    concats(tmp,"pg_get_expr(d.adbin,d.adrelid) as Default,col_description(a.attrelid,a.attnum )\n");
    concats(tmp,"FROM pg_attribute a\n");
    concats(tmp,"INNER JOIN pg_class c ON a.attrelid=c.oid\n");
    concats(tmp,"INNER JOIN pg_namespace n ON c.relnamespace=n.oid\n");
    concats(tmp,"INNER JOIN pg_type t ON a.atttypid=t.oid \n");
    concats(tmp,"LEFT JOIN pg_attrdef d ON d.adrelid=c.oid AND d.adnum=a.attnum\n");
    concats(tmp,"WHERE attnum>0 AND upper(c.relname)=upper(trim('");
    concats(tmp,cmd);
    concats(tmp,"')) AND NOT attisdropped \n");
    concats(tmp,"ORDER BY a.attnum");
    //fprintf(p8->out,mstostring(tmp));
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQresultStatus(p8->pgr)!=PGRES_TUPLES_OK )
    {
        printf("%s\n",PQresultErrorMessage(p8->pgr) );

    }else
        disp_tup(p8);
    PQclear(p8->pgr);
    p8->pgr=NULL;
    t2=new_mstring();
    msreset(tmp);
    p8_get_schema_list(p8,t2);
    concats(tmp,"SELECT schemaname,indexdef FROM pg_indexes WHERE schemaname IN (");
    concat(tmp,mstostring(t2),mslen(t2));
    concats(tmp,") AND tablename ='");
    concats(tmp,cmd);
    concatc(tmp,'\'');
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQntuples(p8->pgr)>0)
    {
        printf("Indexes:\n");
        disp_tup(p8);
    }
    PQclear(p8->pgr);
    msreset(tmp);
    concats(tmp,"SELECT conname, pg_catalog.pg_get_constraintdef(r.oid, true) as condef\n");
    concats(tmp,"FROM pg_catalog.pg_constraint r\n");
    concats(tmp,"WHERE r.conrelid = '");
    concats(tmp,cmd);
    concats(tmp,"'::regclass AND r.contype = 'f'");
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQntuples(p8->pgr)>0)
    {
        printf("Foreign Key Constraints:\n");
        disp_tup(p8);
    }
    PQclear(p8->pgr);
    msreset(tmp);
    concats(tmp,"SELECT conname, pg_catalog.pg_get_constraintdef(r.oid, true) as condef\n");
    concats(tmp,"FROM pg_catalog.pg_constraint r\n");
    concats(tmp,"WHERE r.confrelid = '");
    concats(tmp,cmd);
    concats(tmp,"'::regclass AND r.contype = 'f'");
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQntuples(p8->pgr)>0)
    {
        printf("Referenced by:\n");
        disp_tup(p8);
    }
    PQclear(p8->pgr);
    msreset(tmp);
    concats(tmp,"SELECT pg_get_triggerdef(oid)\n");
    concats(tmp,"FROM pg_trigger WHERE tgrelid='");
    concats(tmp,cmd);
    concats(tmp,"'::regclass AND tgisinternal=false");
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQntuples(p8->pgr)>0)
    {
        printf("Triggers:\n");
        disp_tup(p8);
    }

    PQclear(p8->pgr);
    p8->pgr=NULL;
    destroy_mstring(tmp);
    destroy_mstring(t2);

    return CMD_default;
}
CMD_OUT cmd_i(sp8 *p8)
{
    char * cmd;
    FILE * fin;
    size_t f_len;
    cmd= mstostring(p8->s)+2;
    while (isspace(*cmd) && *cmd!='\0')
        cmd++;
    if(*cmd=='\0')
    {
        printf("No file specified\n");
        return CMD_default;
    }
    fin=fopen(cmd,"rb");
    if(!fin)
    {
        printf("Could not open the specified file\n");
        return CMD_default;
    }
    fseek(fin,0,SEEK_END);
    f_len=ftell(fin);
    if(! checkBuffer(p8->s,f_len,'r'))
    {
        printf("Not enough memory\n");
        return CMD_default;
    }
    fseek(fin,0,SEEK_SET);
    if( fread(mstostring(p8->s),sizeof(char),f_len, fin)!=f_len)
    {

    }
    p8->s->e=p8->s->s+f_len;
    p8->s->str[p8->s->e]='\0';
    fclose(fin);
    return CMD_load_input_file;
}
CMD_OUT cmd_o(sp8 *p8 )
{
    char * cmd;
    FILE * fout;
    cmd= mstostring(p8->s)+2;
    while (isspace(*cmd) && *cmd!='\0')
        cmd++;
    if(*cmd=='\0')
    {
       if(p8->is_custom_out_file)
       {
           fclose(p8->out);
       }
        p8->is_custom_out_file=0;
        p8->out=stdout;
        return CMD_default;
    }
    fout=fopen(cmd," wb");
    if(!fout)
    {
        printf("Could not open the specified file\n");
        return CMD_default;
    }
    p8->out=fout;
    p8->is_custom_out_file=1;
    return CMD_default;

}
CMD_OUT cmd_s(sp8 *p8 )
{
    CMD_OUT res=CMD_default;
    char * cmd=mstostring(p8->s)+2, ch;
    struct mstring *tmp=new_mstring();
    ch=*cmd;
    if(ch=='\0')
    {
        printf("Command not recognized\n");
        return res;
    }
    cmd++;
    while(isspace(*cmd) && *cmd!= '\0')
        cmd++;
    if(*cmd=='\0')
    {
        printf("Command not recognized\n");
        return res;
    }
    switch(ch)
    {
        case 'v':
            concats(tmp,"SELECT pg_get_viewdef('");
            concats(tmp,cmd);
            concats(tmp,"'::regclass)");
            break;
        case 'f':
            concats(tmp," select pg_get_functiondef('");
            concats(tmp,cmd);
            concats(tmp,"'::regproc)");
            break;
        default:
            destroy_mstring(tmp);
            printf("Command not recognized\n");
            return res;
    }
    p8->pgr=PQexec(p8->pg,mstostring(tmp));
    if(PQresultStatus(p8->pgr)!= PGRES_TUPLES_OK)
    {
        printf("%s\n",PQresultErrorMessage(p8->pgr));
    }else
    {
        fprintf(p8->out,"%s\n", PQgetvalue(p8->pgr,0,0));

    }
    PQclear(p8->pgr);
    destroy_mstring(tmp);
    return res;
}
CMD_OUT db_cmd(sp8 *p8 )
{
    CMD_OUT res=CMD_default;

    char *cmd=p8->s->str+(p8->s->s+1);
    switch(*cmd)
    {
        case 'c':

            return printf("Ok\n"),cmd_c(p8);
        case 'd':
            return cmd_d(p8);
        case 'i':
            return printf("Ok\n"),cmd_i(p8);
        case 'o':
            return printf("Ok\n"),cmd_o(p8);
        case 'q':
            return CMD_quit;
        case 's':
            return cmd_s(p8);
        case '?':
            printf("\\c db name\t\tConnect Database\n");
            printf("\\d        \t\tList tables,sequences and views\n");
            printf("\\d tblname\t\tDetails of the table\n");
            printf("\\i tblname\t\tExecute commands from file\n");
            printf("\\o [FILE] \t\tSend all query results to file\n");
            printf("\\q        \t\tQuit\n");
            printf("\\sf fname \t\tShow a function's definition\n");
            printf("\\sv vname \t\tShow a view's definition\n");
        break;
        default:
            printf("Option not valid\n");
            return res;
    }
    return res;
}

void db_prompt()
{
    SetConsoleOutputCP(CP_UTF8);
    sp8 p8;
    ExecStatusType est;
    char  cn_str[1000];
    int i=0;

    strcpy(p8.host,"localhost");
    strcpy(p8.port,"5432");
    p8.s=new_mstring();
    p8.out=stdout;
    p8.is_custom_out_file=0;
CONN_ERR:
    printf("User:");
    scanf("%s",p8.user);
    printf("Database:");
    scanf("%s",p8.db);
    p8.pwd= simple_prompt("Password:",100,0);
    sprintf(cn_str,"host=%s port=%s dbname=%s user=%s password=%s",
            p8.host,p8.port,p8.db,p8.user,p8.pwd);

    p8.pg=PQconnectdb(cn_str);
    if (PQstatus(p8.pg) != CONNECTION_OK)
    {
        char exit;
        printf("\n ERR:%s\n",PQerrorMessage(p8.pg));
        printf ("Digit e to exit\n");
        while(getchar()!='\n')
            ;

        exit=getchar();

        if(exit=='e')
            return ;
        while(getchar()!='\n')
            ;
        PQfinish(p8.pg);
        free(p8.pwd);
        goto CONN_ERR;


    }

    while(1)
    {
        tb_read_next(p8.s,p8.db);
        if( *mstostring(p8.s)=='\\' )
        {
            switch (db_cmd(&p8))
            {
                case CMD_quit:
                    return;
                    break;
                case CMD_load_input_file:
                    break;
                default:
                    continue;
            }
        }

        p8.pgr=PQexec(p8.pg,mstostring(p8.s));
        est=PQresultStatus(p8.pgr);
        switch(est)
        {
            case PGRES_COMMAND_OK:
                printf("%s rows affected\n",PQcmdTuples(p8.pgr) );
                break;
            case PGRES_TUPLES_OK:
            {
                disp_tup(&p8);
                break;
            }
//                FILE *fot;
//                fot=fopen("fout.txt","w");
//                PQdisplayTuples(pgr,fot,1,"|",1,0);
//                fclose(fot);
//                break;
//
//                fot=fopen("fout.txt","w");
//                PQprintTuples(pgr,fot,1,1,0);
//                fclose(fot);
//                break;
//
//                //FILE *fot;
//                fot=fopen("fout.txt","w");
//                char * col [3]={"a","b","c"};
//
//                PQprintOpt popt;
//                popt.header=1;
//                popt.align=1;
//                popt.standard=0;
//                popt.html3=0;
//                popt.expanded=0;
//                popt.pager=0;
//                popt.fieldSep="|";
//                popt.tableOpt="ner";
//                popt.caption="parc" ;
//                popt.fieldName=col;
//                PQprint(fot,pgr,&popt);
//                fclose(fot);
//                break;
//
//
//                int i,j;
//                for(i=0;i<PQnfields(pgr);i++)
//                    printf("%s\t",PQfname(pgr,i));
//                printf ("\n");
//                for(i=0;i<PQnfields(pgr);i++)
//                {
//                    for(j=0;j<PQntuples(pgr);j++)
//                        printf("%s\t");
//
//                }
//            }
                break;
            default:
                printf("%s\n",PQresultErrorMessage(p8.pgr) );
                break;

        }
        PQclear(p8.pgr);
        p8.pgr=NULL;

    }

}
int main()
{
    db_prompt();
    return 0;

}
