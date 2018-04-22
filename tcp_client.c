#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 1024
#define PORT 9002

void draw_table (char* table) {
	int count = 0;
	int row = 0;
	
	printf("__|0  1  2  3  4  5  6  7  8  9\n");
	for (int i = 0; i < 100; ++i) {
		if (count == 0) {
			printf("%d ", row);
			row++;
		}

		printf("[%c]", table[i] == '0' ? ' ' : table[i]);
		count++;

		if (count == 10) {
			printf("\n");
			count = 0;
		}
	}
}

bool check_format (char* command) {
	int start = 0, comma = 0, end = 0, i;

	if (command[start] != '(')
		return false;
	
	for (i = 0; command[i] != '\0' && i < 11; i++) {
		if (command[i] == ',')
			comma = i;
	}
	end = i-1;

	if (command[end] != ')')
		return false;

	if (comma <= start || end <= comma)
		return false;

	return true;
}
int main (int argc, char *argv[]) {
	
	if (argc < 2) {
		printf("No IP address!\n");
		return 1;
	}

	int network_socket;
	network_socket = socket(AF_INET, SOCK_STREAM, 0);

	char server_addr[16];
	sprintf(server_addr, "%s", argv[1]);
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr(server_addr);

	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	if (connection_status < 0) {
		printf("There was an error making a connection to the server! (error %d)\n", connection_status);
		return 1;
	}

	char server_response[SIZE];
	char command[100];

	recv(network_socket, &server_response, sizeof(server_response), 0);
	if (strcmp(server_response, "wait") == 0) {
		printf("Waiting for a player!\n");
		recv(network_socket, &server_response, sizeof(server_response), 0);
	}

	while (strcmp(server_response,"start") == 0)
	{
		do {
			recv(network_socket, &server_response, sizeof(server_response), 0);

			if (strcmp(server_response, "table") == 0) {
				recv(network_socket, &server_response, sizeof(server_response), 0);
				system("clear");
				draw_table(server_response);
			}
			else if (strcmp(server_response, "turn") == 0) {
				printf("Your turn!\n");
				printf("Command: ");
				scanf("%s", command);

				if (strcmp(command, "feladom") == 0) {
					sprintf(server_response, "give_up");
					send(network_socket, server_response, sizeof(server_response), 0);
				}
				else {
					do {
						while(!check_format(command)) {
							printf("Wrong format! Usage: \"(number,number)\"\n");
							printf("Command: ");
							scanf("%s", command);
						}

						sprintf(server_response, "%s", command);
						send(network_socket, server_response, sizeof(server_response), 0);

						recv(network_socket, &server_response, sizeof(server_response), 0);

						if (strcmp(server_response, "illegal") == 0) {
							printf("Illegal move!\n");
							printf("Command: ");
							scanf("%s", command);
						}
					} while (strcmp(server_response, "ok") != 0);
				}
			}
			else if (strcmp(server_response, "no_turn") == 0) {
				printf("Other player's turn!\n");
			}
			else if (strcmp(server_response, "winner") == 0) {
				printf("You are the winner!\n");
			}
			else if (strcmp(server_response, "loser") == 0) {
				printf("You are the loser!\n");
			}
			else if (strcmp(server_response, "you_give_up") == 0) {
				printf("You gave up!\n");
			}
			else if (strcmp(server_response, "other_give_up") == 0) {
				printf("Other player gave up!\n");
			}
			else if (strcmp(server_response, "end") == 0) {
				printf("Ending game...\n");
			}
		} while (strcmp(server_response, "end") != 0);
		
		printf("Replay?\nCommand: ");
		scanf("%s", command);

		if (strcmp(command,"ujra") == 0) {
			sprintf(server_response, "replay");
			send(network_socket, server_response, sizeof(server_response), 0);
		}
		else {
			sprintf(server_response, "exit");
			send(network_socket, server_response, sizeof(server_response), 0);
		}

		recv(network_socket, &server_response, sizeof(server_response), 0);

		if (strcmp(server_response,"close") == 0)
			printf("Game closed!\n");
		else if (strcmp(server_response,"no_replay") == 0)
			printf("No replay! Game closed!\n");
	}
	

	close(network_socket);

	return 0;

}
