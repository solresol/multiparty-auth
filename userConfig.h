#define USER_CONFIG_FILE "/tmp/mause-usercfg"



typedef enum {
  BLANK_PASSWORD, SHARED_LIBRARY 
} AuthenticationType;

typedef struct {
  AuthenticationType authType;
} NullAuthStruct;


typedef struct {
  AuthenticationType authType;
  char digest[16];
} BlankPasswordStruct;

typedef struct {
  AuthenticationType authType;
  void * sharedLibraryPtr;
} SharedLibraryStruct;


/*          */

typedef struct {
  char * userName;
  char * partyName;
  union {
    NullAuthStruct nauth;
    BlankPasswordStruct blank;
    SharedLibraryStruct shlib;
  } methodAndTarget;
} authenticationAgainst;


enum { OK, FILE_READING_ERROR  } UserConfigurationReturnResult;
