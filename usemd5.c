#include <stdio.h>
#include <stdlib.h>
#include <md5.h>

int main(argc, argv) 
     int argc;
     char * argv[];
{
  int i;
  MD5_CTX my_context;
  char my_string[] = "my_stringy thing";
  char mydigest[16];
  MD5Init(&my_context);
  MD5Update(&my_context,my_string,strlen(my_string));
  MD5Final(mydigest,&my_context);
  for (i=0;i<16;i++) { 
    printf("%x",mydigest[i]);
  }
  printf("\n");
}
