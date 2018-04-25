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

			if (y + 4 < WIDTH &&
			player == table[x + 10*(y+1)] &&
			player == table[x + 10*(y+2)] &&
			player == table[x + 10*(y+3)] &&
			player == table[x + 10*(y+4)])
				return (player == 'X' ? 0 : 1);

			if (x + 4 < HEIGHT) {
				if (player == table[x+1 + 10*y] &&
				player == table[x+2 + 10*y] &&
				player == table[x+3 + 10*y] &&
				player == table[x+4 + 10*y])
					return (player == 'X' ? 0 : 1);

				if (y + 4 < WIDTH &&
				player == table[x+1 + 10*(y+1)] &&
				player == table[x+2 + 10*(y+2)] &&
				player == table[x+3 + 10*(y+3)] &&
				player == table[x+4 + 10*(y+4)])
					return (player == 'X' ? 0 : 1);

				if (y - 4 >= 0 &&
				player == table[x+1 + 10*(y-1)] &&
				player == table[x+2 + 10*(y-2)] &&
				player == table[x+3 + 10*(y-3)] &&
				player == table[x+4 + 10*(y-4)])
					return (player == 'X' ? 0 : 1);
			}
		}
	}

	for (int i = 0; i < HEIGHT*WIDTH; i++)
		if (table[i] == '0')
			return -1;

	return 2;
}

void getxy (char* str, int* x, int* y) {
	char temp[10];
	int i, j;

	for (i = 0; str[i] != ','; ++i);

	for (j = 1; j < i; ++j)
		temp[j-1] = str[j];

	temp[j-1] = '\0';
	*x = atoi(temp);
		
	for (j = i+1; str[j] != ')'; ++j)
		temp[j-i-1] = str[j];

	temp[j-i-1] = '\0';
	*y = atoi(temp);
}

int main (int argc, char *argv[]) {

	int clients[2] = {0, 0};
	int num_of_clients = 0;

	char message[SIZE];
	char on = 1;

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in clients_address[2];
	unsigned int clients_address_size[2];

	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 2);
	printf("Waiting for 2 players!\n");

	for (int i = 0; i < 2; ++i) {
		clients_address_size[i] = sizeof(clients_address[i]);
		clients[i] = accept(server_socket, (struct sockaddr *) &(clients_address[i]), &(clients_address_size[i]));

		if (i == 0) {
			sprintf(message, "wait");
			send(clients[i], message, sizeof(message), 0);
			printf("Player 1 joined!\n");
		}
		else if (i == 1) {
			sprintf(message, "start");
			send(clients[i], message, sizeof(message), 0);
			send(clients[i-1], message, sizeof(message), 0);
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
			sprintf(message, "table");
			send(clients[i], message, sizeof(message), 0);

			send(clients[i], table, sizeof(message), 0);
		}

		int turn = 0;
		int x = -1, y = -1;
		bool legal = false;
		bool give_up = false;
		int winner = -1;

		while (winner == -1 && !give_up) {
			sprintf(message,"no_turn");
			send(clients[inv(turn)], message, sizeof(message), 0);

			sprintf(message,"turn");
			send(clients[turn], message, sizeof(message), 0);
			
			do {
				recv(clients[turn], &message, sizeof(message), 0);
				
				if (strcmp(message, "give_up") == 0) {
					give_up = true;
					sprintf(message,"you_give_up");
					send(clients[turn], message, sizeof(message), 0);
					sprintf(message,"other_give_up");
					send(clients[inv(turn)], message, sizeof(message), 0);

					for (int i = 0; i < 2; ++i) {
						sprintf(message,"end");
						send(clients[i], message, sizeof(message), 0);
					}
					printf("Someone gave up. Stopping game...\n");
				}
				else if (strcmp(message, "give_up") != 0) {
					getxy(message, &x, &y);

					if (0 <= x && x <= 9 && 0 <= y && y <= 9) {
						if (table[y*10 + x] == '0') {					
							if (turn == 0)
								table[y*10 + x] = 'X';
							else if (turn == 1)
								table[y*10 + x] = 'O';

							legal = true;
							sprintf(message,"ok");
							send(clients[turn], message, sizeof(message), 0);
							printf("(%d,%d): %c\n", x, y, turn == 0 ? 'X' : 'O');
						}
						else {
							legal = false;
							sprintf(message,"illegal");
							send(clients[turn], message, sizeof(message), 0);
						}
					}
					else {
						legal = false;
						sprintf(message,"illegal");
						send(clients[turn], message, sizeof(message), 0);
					}
				}
			} while (!legal && !give_up);

			if (!give_up) {
				for (int i = 0; i < 2; ++i) {
					sprintf(message, "table");
					send(clients[i], message, sizeof(message), 0);

					send(clients[i], table, sizeof(message), 0);
				}

				legal = false;
				turn = inv(turn);

				winner = check_for_win(table);
				if (winner != -1) {
					if (winner != 2){
						sprintf(message, "winner");
						send(clients[winner], message, sizeof(message), 0);
						sprintf(message, "loser");
						send(clients[inv(winner)], message, sizeof(message), 0);

						printf("Player %d won\n", winner+1);
						printf("Player %d lost\n", inv(winner)+1);
					}
			
					for (int i = 0; i < 2; ++i) {
						if (winner == 2) {
							sprintf(message, "tie");
							send(clients[i], message, sizeof(message), 0);
						}

						sprintf(message, "end");
						send(clients[i], message, sizeof(message), 0);
					}
				
					if (winner == 2)
						printf("Game tie\n");
					
					printf("Game ended\n");
				}
			}
		}

		replay = false;
		for (int i = 0; i < 2; ++i) {
			recv(clients[i], &message, sizeof(message), 0);

			if (strcmp(message,"replay") == 0)
				replay = true;
			else if (strcmp(message,"exit") == 0)
				exit = true;
		}

		if (!exit && replay) {
			bool swap = true;

			for (int i = 0; i < 2; ++i) {
				sprintf(message,"swap");
				send(clients[i], message, sizeof(message), 0);
			}

			for (int i = 0; i < 2; ++i) {
				recv(clients[i], &message, sizeof(message), 0);

				if (strcmp(message,"swap_no") == 0)
					swap = false;
			}

			if (swap) {
				int temp = clients[0];
				clients[0] = clients[1];
				clients[1] = temp;

				for (int i = 0; i < 2; ++i) {
					sprintf(message,"swapped");
					send(clients[i], message, sizeof(message), 0);
				}
			}
			else {
				for (int i = 0; i < 2; ++i) {
					sprintf(message,"not_swapped");
					send(clients[i], message, sizeof(message), 0);
				}
			}

			for (int i = 0; i < 2; ++i) {
				sprintf(message,"start");
				send(clients[i], message, sizeof(message), 0);
			}
		}
		else if (exit && !replay) {
			for (int i = 0; i < 2; ++i) {
				sprintf(message,"close");
				send(clients[i], message, sizeof(message), 0);
			}
			printf("Server closed\n");
		}
		else if (exit && replay) {
			for (int i = 0; i < 2; ++i) {
				sprintf(message,"no_replay");
				send(clients[i], message, sizeof(message), 0);
			}
			printf("Server closed\n");
		}
	}


	close(server_socket);
	return 0;
}
