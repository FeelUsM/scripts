#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char * helpstring = "find-non-ascii выводит имя файла, если в нем содержаться не ascii символы \n";
int main(int argc, char * argv[])
{
  if(argc == 1)
    exit(1);
  if(strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0)
	  {  fprintf(stderr,helpstring);  exit(0);  }
  FILE * f = fopen(argv[1],"r");
  if(!f) {  fprintf(stderr,"can not open %s\n",argv[1]);  exit(1);  }
  int c;
  while((c=fgetc(f))!=EOF){
    if(c<0 || c>127){
      printf("%s\n",argv[1]);
      exit(0);
    }
  }
    
}
