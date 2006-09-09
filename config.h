#ifndef _CONFIG_H_
#define _CONFIG_H_

#define LONGEST_USERNAME 255
#define LONGEST_PASSWORD 10000
#define LONGEST_PARTYNAME NAME_MAX
#define MAAE_CONFIG_DIRECTORY "/tmp/maae/"
#define PARTY_FILE   MAAE_CONFIG_DIRECTORY "usernames/%s/party"
#define PASSWORD_FILE  MAAE_CONFIG_DIRECTORY "usernames/%s/password"
#define PARTY_REQS_DIR MAAE_CONFIG_DIRECTORY "party-requirements"
#define MAX_INCORRECT_PASSWORDS 3
#define PATH_TO_SHELL "/bin/sh"
#define PATH_TO_CLEAR "/usr/bin/clear"


#endif
