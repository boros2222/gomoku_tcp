#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

#define SIZE 1024
#define PORT 9002

int inv (int n) {
	if (n == 1)
		return 0;
	else if (n == 0)
		return 1;
}

int check_for_win (char* table) {
	int HEIGHT = 10;
	int WIDTH = 10;

	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			char player = table[x + 10*y];

			if (player == '0')
				continue;

			if (y + 3 < WIDTH &&
			player == table[x + 10*(y+1)] &&
			player == table[x + 10*(y+2)] &&
			player == table[x + 10*(y+3)] &&
			player == table[x + 10*(y+4)])
				if (player == 'X') return 0; else return 1;

			if (x + 3 < HEIGHT) {
				if (player == table[x+1 + 10*y] &&
				player == table[x+2 + 10*y] &&
				player == table[x+3 + 10*y] &&
				player == table[x+4 + 10*y])
					if (player == 'X') return 0; else return 1;

				if (y + 3 < WIDTH &&
				player == table[x+1 + 10*(y+1)] &&
				player == table[x+2 + 10*(y+2)] &&
				player == table[x+3 + 10*(y+3)] &&
				player == table[x+4 + 10*(y+4)])
					if (player == 'X') return 0; else return 1;

				if (y - 3 >= 0 &&
				player == table[x+1 + 10*(y-1)] &&
				player == table[x+2 + 10*(y-2)] &&
				player == table[x+3 + 10*(y-3)] &&
				player == table[x+4 + 10*(y-4)])
					if (player == 'X') return 0; else return 1;
			}
		}
	}

	return -1;
}

void getxy (char* message, int* x, int* y) {
	char temp[10];
	int i,j;

	for (i = 0; message[i] != ','; ++i);

	for (j = 1; j < i; ++j)
		temp[j-1] = message[j];

	temp[j-1] = '\0';
	*x = atoi(temp);
		
	for (j = i+1; message[j] != ')'; ++j)
		temp[j-i-1] = message[j];

	temp[j-i-1] = '\0';
	*y = atoi(temp);
}

int main (int argc, char *argv[]) {

	int clients[2] = {0, 0};
	int num_of_clients = 0;

	char server_message[SIZE];
	char on = 1;

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in clients_address[2];
	unsigned int clients_address_size[2];

	//setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	//setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 2);
	printf("Waiting for 2 players!\n");

	for (int i = 0; i < 2; ++i) {
		clients_address_size[i] = sizeof(clients_address[i]);
		clients[i] = accept(server_socket, (struct sockaddr *) &(clients_address[i]), &(clients_address_size[i]));

		if (i == 0) {
			sprintf(server_message, "wait");
			send(clients[i], server_message, sizeof(server_message), 0);
			printf("Player 1 joined!\n");
		}
		else if (i == 1) {
			sprintf(server_message, "start");
			send(clients[i], server_message, sizeof(server_message), 0);
			send(clients[i-1], server_message, sizeof(server_message), 0);
			printf("Player 2 joined!\n");
		}
	}

	char table[100];
	bool exit = false;
	bool replay = true;

	while(!exit && replay) {
		printf("Starting game...\n");
		for (int i = 0; i < 100; ++i)
			table[i] = '0';

		for (int i = 0; i < 2; ++i) {
			sprintf(server_message, "table");
			send(clients[i], server_message, sizeof(server_message), 0);

			send(clients[i], table, sizeof(server_message), 0);
		}

		int turn = 0;
		int x = -1, y = -1;
		bool legal = false;
		bool give_up = false;
		int winner = -1;

		while (winner == -1 || !give_up) {
			sprintf(server_message,"no_turn");
			send(clients[inv(turn)], server_message, sizeof(server_message), 0);

			sprintf(server_message,"turn");
			send(clients[turn], server_message, sizeof(server_message), 0);

			recv(clients[turn], &server_message, sizeof(server_message), 0);
		
			if (strcmp(server_message,"give_up") == 0) {
				give_up = true;
				sprintf(server_message,"you_give_up");
				send(clients[turn], server_message, sizeof(server_message), 0);
				sprintf(server_message,"other_give_up");
				send(clients[inv(turn)], server_message, sizeof(server_message), 0);

				for (int i = 0; i < 2; ++i) {
					sprintf(server_message,"end");
					send(clients[i], server_message, sizeof(server_message), 0);
				}
				printf("Someone gave up! Stopping game!\n");

				break;
			}
			else {
				while(!legal) {
					getxy(server_message, &x, &y);

					if (0 <= x && x <= 9 && 0 <= y && y <= 9) {
						if (table[y*10 + x] == '0') {
							sprintf(server_message,"ok");
							send(clients[turn], server_message, sizeof(server_message), 0);
							printf("(%d,%d): %c\n", x, y, turn == 0 ? 'X' : 'O');
							legal = true;
					
							if (turn == 0)
								table[y*10 + x] = 'X';
							else if (turn == 1)
								table[y*10 + x] = 'O';
						}
						else {
							legal = false;
							sprintf(server_message,"illegal");
							send(clients[turn], server_message, sizeof(server_message), 0);
						}
					}
					else {
						legal = false;
						sprintf(server_message,"illegal");
						send(clients[turn], server_message, sizeof(server_message), 0);
					}

					if (!legal)
						recv(clients[turn], &server_message, sizeof(server_message), 0);
				}
			}

			for (int i = 0; i < 2; ++i) {
				sprintf(server_message, "table");
				send(clients[i], server_message, sizeof(server_message), 0);

				send(clients[i], table, sizeof(server_message), 0);
			}

			legal = false;
			turn = inv(turn);

			winner = check_for_win(table);
			if (winner != -1) {
				sprintf(server_message,"winner");
				send(clients[winner], server_message, sizeof(server_message), 0);
				sprintf(server_message,"loser");
				send(clients[inv(winner)], server_message, sizeof(server_message), 0);
			
				for (int i = 0; i < 2; ++i) {
					sprintf(server_message,"end");
					send(clients[i], server_message, sizeof(server_message), 0);
				}
				
				break;
			}
		}

		replay = false;
		for (int i = 0; i < 2; ++i) {
			recv(clients[i], &server_message, sizeof(server_message), 0);

			if (strcmp(server_message,"replay") == 0) {
				replay = true;
			}
			else if (strcmp(server_message,"exit") == 0) {
				exit = true;
			}
		}

		if (!exit && replay) {
			for (int i = 0; i < 2; ++i) {
				sprintf(server_message,"start");
				send(clients[i], server_message, sizeof(server_message), 0);
			}
		}
		else if (exit && !replay) {
			for (int i = 0; i < 2; ++i) {
				sprintf(server_message,"close");
				send(clients[i], server_message, sizeof(server_message), 0);
			}
			printf("Server closed!\n");
		}
		else if (exit && replay) {
			for (int i = 0; i < 2; ++i) {
				sprintf(server_message,"no_replay");
				send(clients[i], server_message, sizeof(server_message), 0);
			}
			printf("Server closed!\n");
		}
	}


	close(server_socket);
	return 0;
}
