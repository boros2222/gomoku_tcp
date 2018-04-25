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
	char on = 1;
	network_socket = socket(AF_INET, SOCK_STREAM, 0);

	char server_addr[16];
	sprintf(server_addr, "%s", argv[1]);
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr(server_addr);

	setsockopt(network_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	setsockopt(network_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	if (connection_status < 0) {
		printf("There was an error making a connection to the server!\n");
		return 1;
	}

	char message[SIZE];
	char command[100];

	recv(network_socket, &message, sizeof(message), 0);
	if (strcmp(message, "wait") == 0) {
		printf("Waiting for a player!\n");
		recv(network_socket, &message, sizeof(message), 0);
	}

	while (strcmp(message,"start") == 0)
	{
		do {
			recv(network_socket, &message, sizeof(message), 0);

			if (strcmp(message, "table") == 0) {
				recv(network_socket, &message, sizeof(message), 0);
				system("clear");
				draw_table(message);
			}
			else if (strcmp(message, "turn") == 0) {
				printf("Your turn!\n");

				do {
					do {
						printf("Command: ");
						scanf("%s", command);

						if (strcmp(command,"feladom") != 0 && !check_format(command))
							printf("Wrong format! Usage: \"(number,number)\"\n");

					} while (strcmp(command,"feladom") != 0 && !check_format(command));

					if (strcmp(command, "feladom") == 0) {
						sprintf(message, "give_up");
						send(network_socket, message, sizeof(message), 0);
					}
					else if (strcmp(command, "feladom") != 0) {
						sprintf(message, "%s", command);
						send(network_socket, message, sizeof(message), 0);

						recv(network_socket, &message, sizeof(message), 0);

						if (strcmp(message, "illegal") == 0)
							printf("Illegal move!\n");
					}
				} while (strcmp(message, "ok") != 0 && strcmp(command, "feladom") != 0);
			}
			else if (strcmp(message, "no_turn") == 0) {
				printf("Other player's turn!\n");
			}
			else if (strcmp(message, "winner") == 0) {
				printf("You won!\n");
			}
			else if (strcmp(message, "loser") == 0) {
				printf("You lost!\n");
			}
			else if (strcmp(message, "tie") == 0) {
				printf("Tie! No winner!\n");
			}
			else if (strcmp(message, "you_give_up") == 0) {
				printf("You gave up!\n");
			}
			else if (strcmp(message, "other_give_up") == 0) {
				printf("Other player gave up!\n");
			}
			else if (strcmp(message, "end") == 0) {
				printf("Ending game...\n");
			}
		} while (strcmp(message, "end") != 0);
		
		printf("Replay? (ujra)\nCommand: ");
		scanf("%s", command);

		if (strcmp(command,"ujra") == 0) {
			sprintf(message, "replay");
			send(network_socket, message, sizeof(message), 0);
		}
		else {
			sprintf(message, "exit");
			send(network_socket, message, sizeof(message), 0);
		}

		recv(network_socket, &message, sizeof(message), 0);

		if (strcmp(message, "swap") == 0) {
			printf("Swap symbols? (igen)\nCommand: ");
			scanf("%s", command);

			if (strcmp(command, "igen") == 0) {
				sprintf(message, "swap_yes");
				send(network_socket, message, sizeof(message), 0);
			}
			else {
				sprintf(message, "swap_no");
				send(network_socket, message, sizeof(message), 0);
			}

			recv(network_socket, &message, sizeof(message), 0);
			if (strcmp(message, "swapped") == 0)
				printf("Symbols swapped!\n");
			else if (strcmp(message, "not_swapped") == 0)
				printf("No agreement! Symbols are not swapped!\n");

			recv(network_socket, &message, sizeof(message), 0);

			sleep(2);
		}
		else if (strcmp(message,"close") == 0)
			printf("Game closed!\n");
		else if (strcmp(message,"no_replay") == 0)
			printf("No agreement! Game closed!\n");
	}
	

	close(network_socket);

	return 0;

}
