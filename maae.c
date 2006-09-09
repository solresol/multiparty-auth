#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <md5.h>
#include <sys/types.h>
#include <dirent.h>

#include "config.h"

/* curses support is not yet complete */
/* #define HAVE_CURSES */

typedef enum { OutOfMemory = 100, FileOpenError , IllegalUsername,
	       InternalUnknownDatumItem,  DirectoryOpenError,
	       DirectoryEntryError, IllegalPartyName, 
	       PartyRequirementsSyntaxError,
} MaaeError;

/* global exception storage handler */
MaaeError maaeError;

char * maaeErrorString() {
  switch (maaeError) {
  case OutOfMemory: return "Out of memory";
  case FileOpenError: return "Error opening file for some reason";
  case IllegalUsername: return "Username contains invalid characters";
  case InternalUnknownDatumItem: return "(an internal error) a function requested an item of datum which could not exist";
  case DirectoryOpenError: return "Error opening a directory for some reason";
  case DirectoryEntryError: return "Error reading a directory for some reason";
  case IllegalPartyName: return "Party name contains invalid characters";
  case PartyRequirementsSyntaxError: return "A party config file contained a syntax error";
  default: return "This error does not have an explanatory string";
  }

}

/***********************************************************************/

typedef enum { PartyOfUser = 1000, MD5ofUser} UserDatum;

static char * getUserInfo(username,item) 
     char * username;
     UserDatum item;
     /* will return the first line of the appropriate data file */
{
  char * file_name;
  int file_name_length;
  FILE * file;
  int i;
  int c;
  char * datum;
  int datum_length;
  char * file_template;

  switch (item) {
  case PartyOfUser: file_template = PARTY_FILE; break;
  case MD5ofUser: file_template = PASSWORD_FILE; break;
  default: maaeError = InternalUnknownDatumItem;  return NULL;
  }

  /* Check to see there are no nasties */
  for (i=0;i<strlen(username);i++) { 
    if (!(isalnum(username[i]))) { 
      maaeError = IllegalUsername;
      return NULL; 
    }
  }

  /* allocate memory */
  file_name_length = strlen(file_template) + strlen(username) + 1;
  file_name = malloc(file_name_length);
  if (NULL == file_name) { maaeError = OutOfMemory; return NULL; }
  snprintf(file_name,file_name_length,file_template,username);
  
  file = fopen(file_name,"r");
  if (NULL == file) { 
    maaeError = FileOpenError;
    free(file_name); 
    return NULL; 
  }

  /* get the number of characters in the first line --
    I could be smarter about this,  but I'm too lazy.
    I should also do clever things like ignore a blank
    line,  or ignore a commented (#) line.
  */

  i=0;
  c = fgetc(file);
  while ((c != EOF) && (c != '\n')) { i++;  c = fgetc(file); }
  datum_length = i;
  /* allocate memory for it */
  datum = malloc(datum_length+1);

  if (NULL == datum) { 
    maaeError = OutOfMemory;
    free(file_name); 
    return NULL; 
  }
  
  /* actually go and get it */
  rewind(file);
  (void) fread(datum,1,datum_length,file);
  datum[datum_length] = 0;

  /* tidy up */
  free(file_name);
  fclose(file);
  
  return datum;
}


/***********************************************************************/
static char * partyOfUser(username)
     char * username;
{
  return getUserInfo(username,PartyOfUser);
}

static char * md5OfUser(username)
     char * username;
{
  return getUserInfo(username,MD5ofUser);
}

/***********************************************************************/

static char * textual_md5_of(inputstring)
     char * inputstring;
{
  char * ret_result;
  MD5_CTX my_context;
  char mydigest[16];

  ret_result = malloc(40);  /* slightly bigger than necessary, probably */
  if (NULL == ret_result) { maaeError = OutOfMemory; return NULL; }

  MD5Init(&my_context);
  MD5Update(&my_context,inputstring,strlen(inputstring));
  MD5Final(mydigest,&my_context);

  snprintf(ret_result,40,
	   "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
	   mydigest[0],	   mydigest[1],	   mydigest[2],	   mydigest[3],
	   mydigest[4],	   mydigest[5],	   mydigest[6],	   mydigest[7],
	   mydigest[8],	   mydigest[9],	   mydigest[10],   mydigest[11],
	   mydigest[12],   mydigest[13],   mydigest[14],   mydigest[15]);
  return ret_result;
}

/***********************************************************************/


typedef struct {
  char * party;
  int minimum_required;
  int number_authenticated_so_far;
} PartyAuthenticationStructure;

static PartyAuthenticationStructure * party_authents;
int num_party_authents;

int read_party_authentication_requirements () {
  DIR * reqdir;
  struct dirent * ent;
  FILE * file;
  int i;
  int j;
  int directory_entry_length;
  char * full_path_name;
  int full_path_name_length;

  reqdir = opendir(PARTY_REQS_DIR);
  if (NULL == reqdir) { maaeError = DirectoryOpenError; return 0; }
  num_party_authents = 0;
  while ((ent = readdir(reqdir)) != NULL) { 
    if ( (strcmp(ent->d_name,".") == 0) || (strcmp(ent->d_name,"..")==0)) {
      continue;
    }
    num_party_authents++; 
  }
  closedir(reqdir);

  /* now we know how much space we need */
  party_authents = malloc(sizeof(PartyAuthenticationStructure)
			  * num_party_authents);

  reqdir = opendir(PARTY_REQS_DIR);
  if (NULL == reqdir) { maaeError = DirectoryOpenError; return 0; }

  for (i=0;i<num_party_authents;i++) {

    ent = readdir(reqdir);

    if (NULL == ent) { 
      maaeError = DirectoryEntryError;
      goto breakout;
    }

    if ( (strcmp(ent->d_name,".") == 0) || (strcmp(ent->d_name,"..")==0)) {
      i--;
      continue;
    }
    
    directory_entry_length = strlen(ent->d_name);
    for (j=0;j<directory_entry_length;j++) {
      if (!isalnum(ent->d_name[j])) {
	maaeError = IllegalPartyName;
	goto breakout;
      }
    }

    full_path_name_length = directory_entry_length + strlen(PARTY_REQS_DIR) + 5;
       /* 5 just for good measure */
    full_path_name = malloc(full_path_name_length);
    if (NULL == full_path_name) { maaeError = OutOfMemory; goto breakout; }
    snprintf(full_path_name,full_path_name_length,
	     PARTY_REQS_DIR "/%s", ent->d_name);
    file = fopen(full_path_name,"r");
    free(full_path_name);

    if (NULL == file) { maaeError = FileOpenError; goto breakout; }

    if (!fscanf(file,"%d",&party_authents[i].minimum_required)) {
      maaeError = PartyRequirementsSyntaxError;
      goto breakout;
    }

    party_authents[i].number_authenticated_so_far = 0;

    party_authents[i].party = malloc(directory_entry_length+1);
    if (NULL == party_authents) { 
      maaeError = OutOfMemory;
      goto breakout;
    }
    strncpy(party_authents[i].party, ent->d_name, directory_entry_length+1);
    
  }

  return 1;

  /* post execution clean up */
  breakout:
  for (j=0;j<i;j++) { free(party_authents[j].party); }
  free(party_authents); 
  party_authents = NULL; 
  num_party_authents = 0;
  closedir(reqdir);
  return 0;

}



/************************************************************************/

#ifdef HAVE_CURSES

#include <curses.h>

int clear_screen() {
  refresh();
  return erase();
}

#else

int clear_screen() {
  return system(PATH_TO_CLEAR);
}

#endif 



/***********************************************************************/

int main (argc,argv) 
     int argc;
     char *argv[];
{
  char username[LONGEST_USERNAME];
  char password[LONGEST_PASSWORD];
  char  * partyname;
  char * md5target;
  char * md5result;
  int i;
  int have_found_appropriate_party;
  int got_sufficient_authentication;
  struct termios term_info;
  int number_of_incorrect_passwords = 0;

#ifdef HAVE_CURSES
  initscr();
  cbreak();
  noecho();
#endif

  if (!read_party_authentication_requirements()) {
    fprintf(stderr,"The following problem:\n %s\nwas encountered while reading the party requirements in %s\n",maaeErrorString(),PARTY_REQS_DIR);
    exit(1);
  }

  got_sufficient_authentication = 0;


  while (!got_sufficient_authentication) {
    clear_screen();
    printf(" ----------+ Maae Authentication +---------\n");
    for (i=0;i<num_party_authents;i++) {
      printf("    [%s: %d/%d]\n",
	     party_authents[i].party,
	     party_authents[i].number_authenticated_so_far,
	     party_authents[i].minimum_required);
    }
    printf("\n");
    /* **************************************** */

    printf("Name: ");
    fgets(username,LONGEST_USERNAME,stdin);
  
    /* strip off the newline character */
    i = strlen(username);
    if (username[i-1] == '\n') { username[i-1] = 0; }
    
    /* **************************************** */

    partyname = partyOfUser(username);
    if (NULL == partyname) {
      fprintf(stderr,"Username %s,  getting partyname: %s\n",username,maaeErrorString());
      exit(1);
    }
    
    printf("%s is a member of %s\n",username,partyname);
    
    
    /* **************************************** */
    printf("Password / Swipe card: ");
    tcgetattr(STDIN_FILENO,&term_info);
    if (term_info.c_lflag & ECHO) {
      term_info.c_lflag -= ECHO;
    }
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&term_info); /* stty -noecho */
    
    fgets(password,LONGEST_PASSWORD,stdin);
    
    term_info.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&term_info);
    
    /* strip off the newline character */
    i = strlen(password);
    if (password[i-1] == '\n') { password[i-1] = 0; }
    
    md5result = textual_md5_of(password);
  

    /* **************************************** */
  
    md5target = md5OfUser(username);
    if (NULL == md5target) {
      fprintf(stderr,"Username %s, getting md5: %s\n",username,maaeErrorString());
      exit(1);
    }
    


    if (strcmp(md5result,md5target) == 0) {
      /* OK,  they got it right */
      fprintf(stderr,"Password OK!\n");

      have_found_appropriate_party = 0;
      for(i=0;i<num_party_authents;i++) {
	if (strcmp(party_authents[i].party,partyname)==0) {
	  have_found_appropriate_party = 1;
	  party_authents[i].number_authenticated_so_far++;
	  break;
	}
      }
      
      if (!have_found_appropriate_party) {
	fprintf(stderr,"%s is a member of %s,  but there is no %s in %s",
		username,partyname,partyname,PARTY_REQS_DIR);
	exit(1);
      }
      
      
    } else {
      /* password is wrong */
      fprintf(stderr,"Incorrect password.\n");
      number_of_incorrect_passwords++;
      if (number_of_incorrect_passwords < MAX_INCORRECT_PASSWORDS) {
	goto loop_ending;
      } else {
	exit(1);
      }
    }
    
    got_sufficient_authentication = 1;
    for (i=0;i<num_party_authents;i++) {
      if (party_authents[i].number_authenticated_so_far <
	  party_authents[i].minimum_required) {
	got_sufficient_authentication = 0;
	break;
      }
    }

  loop_ending:
  free(partyname);
  free(md5target);
  free(md5result);
  } /* while (!got_sufficient_authentication) */

  clear_screen();
  printf("\nStarting %s\n",PATH_TO_SHELL);
  /*  execl(PATH_TO_SHELL, PATH_TO_SHELL, NULL ); */

  return 0;

}
