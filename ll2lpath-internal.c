// todo переписать все нахрен с использованием <dirent.h> и <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>

void readuntilspace(char * * ps)
{
	while(**ps && !isspace(**ps))
		(*ps)++;
}
void readspaces(char * * ps)
{
	while(**ps && isspace(**ps))
		(*ps)++;
}

#define MYBUFSIZ 10000
#define ifnot(arg) if(!(arg))

int main(int argc, char * argv[])
{
	setlocale(LC_ALL,"");

	char curpath[MYBUFSIZ];
	char str[MYBUFSIZ];

	FILE * out, * sapid, * in;
	ifnot(out=fopen(argv[1],"w"))
	{ fprintf(stderr,"can not open file %s",argv[1]); exit(1); }
	ifnot(sapid=fopen(argv[2],"w"))
	{ fprintf(stderr,"can not open file %s",argv[2]); exit(1); }
	in=stdin;

	int dir_count=0;
#define RID_PRINT 100
	while(!feof(in))
	{
		fgets(curpath,MYBUFSIZ,in);
		curpath[strlen(curpath)-1]='\0';
		curpath[strlen(curpath)-1]='\0';
		
		dir_count++;
		if(dir_count%RID_PRINT==0){
			if(dir_count/RID_PRINT==1)
				fprintf(stderr,"progress begin\n");
			fprintf(stderr,"%s\n",curpath);
		}
		//		printf("curpath = \"%s\"\n",curpath);
		
		
		bool dot_files = false;
		int i;
		for(i=3; i<argc; i++)
			if(strlen(curpath)>2 && strstr(curpath+2,argv[i])==curpath+2){
				dot_files=true;
				break;
			}
		fgets(str,MYBUFSIZ,in);//итого кол-во файлов - игнорируем
		while(fgets(str,MYBUFSIZ,in)){

			if(isspace(*str))break;
			char * p=str;

			if(*p=='d' || *p=='l') continue;

			if(*p!='-')	{
				char type=*p;
				readuntilspace(&p); readspaces(&p); //printf("after 1 readuntilspace p = \"%s\" \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 2 readuntilspace p = %s \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 3 readuntilspace p = %s \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 4 readuntilspace p = %s \n",p);
					int err=0;
					errno=0;
				size_t s = strtol(p,&p,10);  readspaces(&p); //printf("size=%d\n",s);
					if(errno){	fprintf(stderr,"size of file is overflow\n");	err=1;	}
					errno=0;
				int date = strtol(p,&p,10);  readspaces(&p);
					if(errno){	fprintf(stderr,"date of file is overflow\n");	err=1;	}
				if(p[strlen(p)-1]=='\n') p[strlen(p)-1]='\0';
					if(err)	fprintf(stderr,"%s/%s\n",curpath,p);
				//printf("add %10.10d %s/%s\n",s,curpath,p);                                  //!!!
				//sdp.insert(make_pair(s,make_pair(date,string(curpath)+'/'+string(p))));
				fprintf(sapid,"%c\t%d\t%d\t%s/%s\n",type,s,date,curpath,p);
			}
			else{
				readuntilspace(&p); readspaces(&p); //printf("after 1 readuntilspace p = \"%s\" \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 2 readuntilspace p = %s \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 3 readuntilspace p = %s \n",p);
				readuntilspace(&p); readspaces(&p); //printf("after 4 readuntilspace p = %s \n",p);
					int err=0;
					errno=0;
				size_t s = strtol(p,&p,10);  readspaces(&p); //printf("size=%d\n",s);
					if(errno){	fprintf(stderr,"size of file is overflow\n");	err=1;	}
					errno=0;
				int date = strtol(p,&p,10);  readspaces(&p);
					if(errno){	fprintf(stderr,"date of file is overflow\n");	err=1;	}
				if(p[strlen(p)-1]=='\n') p[strlen(p)-1]='\0';
					if(err)	fprintf(stderr,"%s/%s\n",curpath,p);
				//printf("add %10.10d %s/%s\n",s,curpath,p);                                  //!!!
				//sdp.insert(make_pair(s,make_pair(date,string(curpath)+'/'+string(p))));
				if(!dot_files)
					fprintf(out,"%d\t%d\t%s/%s\n",s,date,curpath,p);
			}	  
		}
		//           getchar();
	}
	if(dir_count>=RID_PRINT)
		fprintf(stderr,"progress end\n");

	fclose(in);
	fclose(out);
	fclose(sapid);
}
