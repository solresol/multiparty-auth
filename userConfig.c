#include "userConfig.h"

/*
  A sample config file looks something like this:

  Username:   Alice Anderson
  Party:      Government
  Method:     Password
  Target:     125125fba124124

  Username:   Bob Balderstone
  Party:      Acme Corporation
  Method:     Swipe Card
  Target:     abfbabf12312ba
  
*/

UserConfigurationReturnResult  readUserConfig() {
  FILE* f;
  
  f = fopen(USER_CONFIG_FILE,"r");
  if (NULL == f) {
    return FILE_READING_ERROR;
  };

  

}


char * readConfigOption(keyword,fdes)
     char * keyword;
     FILE * fdes;
{
  long position = ftell(fdes);
  int c;
  int i;

  for (i=0;i<strlen(keyword);i++) {
    c = fgetc(fdes);
    if (c == EOF) { goto  backout; }
    if (c != keyword[i]) { goto backout; }
  }
  
 backout:
  ftell(fdes,position,SEEK_SET);
  return NULL;

}
