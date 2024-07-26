#ifndef DICTIONARY_H
#define DICTIONARY_H

#define DEFAULT_DICTIONARY "default_dictionary.txt"
#define DICTAM "dictAM.txt"
#define DICTNZ "dictNZ.txt"
#define DEFAULT_PORT 8888
#define LOOPBACK_IP "127.0.0.1"
#include "network.h"

int word_in_dict(char* client_testword,const char* chosen_dict);

#endif