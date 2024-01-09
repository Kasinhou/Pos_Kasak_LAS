#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct ant_t {
    //na akom policku stoji
    int actualField;
    int position;
    int direction; //0up 1right 2down 3left
} ant_t;

typedef struct world_t {
    int rows;
    int columns;
    int *array_world;
    int ants;
    // 0 = priama, 1 = inverzna
    int movement;
} world_t;

void createWorld(world_t *world) {//}, int rows, int columns, int ants, int movement) {
    world->array_world = malloc(sizeof(int) * (world->rows * world->columns));
    world->movement = (world->movement == 1) ? 1 : -1;
}

int createAnt(world_t *world, ant_t *ant, int position, int direction) {
    ant->position = position;
    ant->direction = direction;
    ant->actualField = world->array_world[ant->position];
    if (world->array_world[ant->position] == 2) {
        return 1;
    }
    return 0;
}

_Bool world_try_deserialize(world_t *world, char* buf) {
    if (sscanf(buf, "%d;%d;%d;%d;", &world->rows, &world->columns,
               &world->ants, &world->movement) == 4) {
        printf("\nuspesne deserializovane\n");
        return true;
    }
    printf("\nneuspesne bohuzial\n");
    return false;
}

void transform_to_buffer(world_t *world, char *buffer) {
    /*for (int i = 0; i < world->rows * world->columns; ++i) {
        buffer[i] = (char)world->array_world[i];
    }*/
    int index = 0;
    buffer[index++] = '\n';
    for (int i = 0; i < world->columns; ++i) {
        buffer[index++] = '-';
        buffer[index++] = '-';
    }
    buffer[index++] = '-';
    buffer[index++] = '\n';
    for (int i = 0; i < world->rows; ++i) {
        for (int j = 0; j < world->columns; ++j) {
            buffer[index++] = '|';
            switch (world->array_world[i * world->columns + j]) {
                case -1:
                    buffer[index++] = '#';
                    break;
                case 1:
                    buffer[index++] = ' ';
                    break;
                case 2:
                    buffer[index++] = '.';
                    break;
                default:
                    printf("Something went wrong with world array. Unexpected character!\n");
                    printf("\nThis is not suppose to be here -> %d\n", world->array_world[i * world->columns + j]);
                    return;
            }
        }
        buffer[index++] = '|';
        buffer[index++] = '\n';
        for (int j = 0; j < world->columns; ++j) {
            buffer[index++] = '-';
            buffer[index++] = '-';
        }
        buffer[index++] = '-';
        buffer[index++] = '\n';
    }
    buffer[index++] = '\n';
}

//postaranie sa o znicenie
void destroyWorld(world_t *world) {
    free(world->array_world);
    world->rows = 0;
    world->columns = 0;
    world->ants = 0;
    world->movement = 0;
}

void generateBlackFields(world_t *world) {
    int count = world->rows * world->columns;
    //pravdepodobnost cierneho policka
    double probability = (double)rand() / RAND_MAX;
    //printf("%.2f and %d\n", probability * 100, count);
    for (int i = 0; i < count; ++i) {
        //ak je vygenerovane cislo mensie ako probability tak je cierna (cierna 0 biela 1 mravec 2);
        if ((double)rand() / RAND_MAX < probability) {
            world->array_world[i] = -1;
        }
        else {
            world->array_world[i] = 1;
        }
    }
}

//implementacia metody do buducnosti
void defineBlackFieldsByHand(world_t *world) {
    int number;
    printf("Define the world by yourself\n0 -> black\n1 -> white\nNumbers must be separated by a space:\n");
    for (int i = 0; i < world->rows; ++i) {
        for (int j = 0; j < world->columns; ++j) {
            scanf("%d", &number);
            if (number != 0 && number != 1) {
                number = 1;
            }
            world->array_world[i * world->columns + j] = (number == 1) ? number : -1;
            //world->array_world[i * world->columns + j] = number;
        }
    }
}


void showWorldState(world_t *world) {
    printf("\n");
    for (int i = 0; i < world->columns; ++i) {
        printf("----");
    }
    printf("-\n");
    for (int i = 0; i < world->rows; ++i) {
        for (int j = 0; j < world->columns; ++j) {
            switch (world->array_world[i * world->columns + j]) {
                case -1:
                    printf("| # ");
                    break;
                case 1:
                    printf("|   ");
                    break;
                case 2:
                    printf("| . ");
                    break;
                default:
                    printf("Something went wrong with world array. Unexpected character!\n");
                    printf("\nThis is not suppose to be here -> %d\n", world->array_world[i * world->columns + j]);
                    return;
            }
            //printf("%d ", world->array_world[i * world->columns + j]);
        }
        printf("|\n");
        for (int j = 0; j < world->columns; ++j) {
            printf("----");
        }
        printf("-\n");
    }
    printf("\n");
}

//krok mravca podla typu pohybu
//direct -> biele 1 otocka vpravo, zmena na cierne 0, type direct/inverse
int antsStep(world_t *world, ant_t *ant, int type) {
    _Bool step = false;
    while (!step) {
        if (ant->actualField == type) {
            ant->direction = (ant->direction + 3) % 4;
        } else {
            ant->direction = (ant->direction + 1) % 4;
        }

        switch (ant->direction) {
            case 0:
                if (ant->position / world->columns != 0) {
                    step = true;
                    //zmenit field na actual field opacnej farby, position mravca, actual field na nove, nove na 2
                    world->array_world[ant->position] = (ant->actualField * -1);
                    //zalezi od smeru vypocet
                    ant->position -= world->columns;
                    ant->actualField = world->array_world[ant->position];
                    //kolizie, ak sa stretnu ten co je tam skor ma prednost a prezil
                    if (world->array_world[ant->position] != 2) {
                        world->array_world[ant->position] = 2;
                    } else {
                        return 1;
                    }
                }
                break;
            case 1:
                if (ant->position % world->columns != world->columns - 1) {
                    step = true;
                    world->array_world[ant->position] = (ant->actualField * -1);
                    ant->position++;
                    ant->actualField = world->array_world[ant->position];
                    if (world->array_world[ant->position] != 2) {
                        world->array_world[ant->position] = 2;
                    } else {
                        return 1;
                    }
                }
                break;
            case 2:
                if (ant->position / world->columns != world->rows - 1) {
                    step = true;
                    world->array_world[ant->position] = (ant->actualField * -1);
                    ant->position += world->columns;
                    ant->actualField = world->array_world[ant->position];
                    if (world->array_world[ant->position] != 2) {
                        world->array_world[ant->position] = 2;
                    } else {
                        return 1;
                    }
                }
                break;
            case 3:
                if (ant->position % world->columns != 0) {
                    step = true;
                    world->array_world[ant->position] = (ant->actualField * -1);
                    ant->position--;
                    ant->actualField = world->array_world[ant->position];
                    if (world->array_world[ant->position] != 2) {
                        world->array_world[ant->position] = 2;
                    } else {
                        return 1;
                    }
                }
                break;
            default:
                printf("Something went wrong with direction. Unexpected error.");
                return -1;
        }
    }
    return 0;
}

//simulacia s posielanim odpovede na klienta
void simulation(world_t *world, ant_t *ants, int type, int socket) {
    while (true) {
        char buf[2048];
        for (int i = 0; i < world->ants; i++) {
            if (antsStep(world, &ants[i], type) == 1) {
                for (int j = i; j < world->ants - 1; ++j) {
                    ants[j] = ants[j + 1];
                }
                world->ants--;
                i--;
            }
        }
        transform_to_buffer(world, buf);
        int n = write(socket, buf, strlen(buf));
        if (n < 0)
        {
            perror("Error writing to socket");
            return;
        }
        //showWorldState(world);
        usleep(1000000);
    }
}


int main(int argc, char *argv[])
{
    //spracovanie socketu
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[1024];

    if (argc < 2)
    {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }

    bzero(buffer,1024);
    n = read(newsockfd, buffer, 1023);
    if (n < 0)
    {
        perror("Error reading from socket");
        return 4;
    }
    printf("Here is the message: %s\n", buffer);

    //vytvorenie sveta a inicializovanie
    world_t world;
    world_try_deserialize(&world, buffer);
    //printf("R: %d\nC: %d\nN: %d\nM: %d", world.rows, world.columns, world.ants, world.movement);
    createWorld(&world);
    generateBlackFields(&world);

    ant_t antsArray[world.ants];
    //bud nahodne alebo zo vstupu, chyba vstup
    for (int i = 0; i < world.ants; ++i) {
        //double position = (double)rand() / RAND_MAX;
        if (createAnt(&world, &antsArray[i],
                      (int)((double)rand() / RAND_MAX * (world.rows * world.columns)),
                      (int)((double)rand() / RAND_MAX * 4)) == 1) {
            for (int j = 0; j < world.ants; ++j) {
                antsArray[j] = antsArray[j + 1];
            }
            world.ants--;
            i--;
        }
    }

    simulation(&world, antsArray, world.movement, newsockfd);



    const char *msg = "I got your message";
    n = write(newsockfd, msg, strlen(msg));
    if (n < 0) {
        perror("Error writing to socket");
        return 5;
    }


    close(newsockfd);
    close(sockfd);

    destroyWorld(&world);

    return 0;
}