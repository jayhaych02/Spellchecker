#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "dictionary.h"
#include "network.h"

int main(int argc, char const *argv[])
{
	int status;
	int client_fd;
	struct sockaddr_in serv_addr;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){printf("\n Socket creation error \n");return -1;}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(DEFAULT_PORT);
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){printf("\nInvalid address/ Address not supported \n");return -1;}
	if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0){printf("\nConnection Failed \n");return -1;}

	char workthread_greeting[400];
	if(recv(client_fd, workthread_greeting, sizeof(workthread_greeting),0) < 0){perror("Couldn't receive assigned thread welcome msg!");}
	puts(workthread_greeting);

/*	
	Below = Tests multiple clients by opening multiple terminals and running client.c

	char check_word[20];
	scanf("%s", check_word);
	if(send(client_fd,check_word,sizeof(check_word),0) <0){perror("couldnt sent word of invalid size");}

	char spellcheck_result[200];
	if( recv(client_fd, spellcheck_result, sizeof(spellcheck_result),0) <0){perror("error spellchecking client sent word!");}
	puts(spellcheck_result);
	close(client_fd);	 

	When I usleep(), the client fd's are NOT in order anymore. They should be: 45678. Clients share same words, but dont share the same time accepted/completed
	If I DONT SLEEP(), then client connection never exits somehow
*/
	const char* words_to_check[] = {"garden","Rabbit","world", "Biscuit", "test", "Boots",
	"whale",
	"window",
	"yak",
	"Basketball",
	"Norway",
	"Penguin",
	"zoo",
	"Telephone",
	"Quartet",
	"Xylophone",
	};
	
	//Num 4 byte ptrs/4 byte ptr = 16 elements
    int num_words = sizeof(words_to_check) / sizeof(words_to_check[0]);
    for (int i = 0; i < num_words; i++) {
        if (send(client_fd, words_to_check[i], strlen(words_to_check[i]), 0) < 0) {
            perror("send failed");
            break;
        }
        char server_response[4096] = {0};
        ssize_t valread = recv(client_fd, server_response,sizeof(server_response), 0);
        if (valread < 0) {
            perror("recv failed");
            break;
        }
        printf("Word: %s\n", server_response);
        usleep(1000000);
    }

	close(client_fd); 
	return 0;
}