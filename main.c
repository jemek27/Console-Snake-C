#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define bool char
#define true 1
#define false 0

void gotoXY (COORD* coord,  SHORT x, SHORT y);

typedef struct Position {
    SHORT x;
    SHORT y;
} Position;

typedef struct Snake {
    SHORT size;
    char direction;
    Position * fragments;
    Position tile;

    void (*init)(struct Snake*, SHORT, SHORT, char);
    void (*move)(struct Snake*, COORD* coord, char** map, Position nextPosition);
    void (*tryChangeDir)(struct Snake*, char);
} Snake;

void init(struct Snake* s, SHORT x, SHORT y , char dir) {
    s->size = 2;
    s->direction = dir;
    s->fragments[0] = (Position){x, y};

    switch (tolower(dir)) {
        case 's':
            s->fragments[1] = (Position){x, y - 1};
            break;
        case 'a':
            s->fragments[1] = (Position){x + 1, y};
            break;
        case 'd':
            s->fragments[1] = (Position){x - 1, y};
            break;
        default:
            s->fragments[1] = (Position){x, y + 1};
            break;
    }

    s->tile = s->fragments[1];
}

void free2DMemory(char** p, int n) {
    for (int i = 0; i < n; ++i) {
        free(p[i]);
    }
    free(p);
}

bool checkGameOver(Position nextPosition, char** map) {
    return (map[nextPosition.y][nextPosition.x] == '#' ||
            map[nextPosition.y][nextPosition.x] == '*'    ) ? true : false;
}

Position tryMove(struct Snake* s) {
    Position nextPosition = s->fragments[0];
    switch (s->direction) {
        case 's':
            nextPosition.y += 1;
            break;
        case 'a':
            nextPosition.x -= 1;
            break;
        case 'd':
            nextPosition.x += 1;
            break;
        default:
            nextPosition.y -= 1;
            break;
    }
     return nextPosition;
}

void move(struct Snake* s, COORD* coord, char** map, Position nextPosition) {

    s->tile = s->fragments[s->size - 1];
    gotoXY(coord, s->tile.x, s->tile.y);
    map[s->tile.y][s->tile.x] = ' ';
    printf(" ");
    gotoXY(coord, s->fragments[0].x, s->fragments[0].y);
    map[s->fragments[0].y][s->fragments[0].x] = '*';
    printf("*");

    for (int i = s->size - 1; i > 0; --i) {
        s->fragments[i] = s->fragments[i - 1];
    }

    s->fragments[0] = nextPosition;

    gotoXY(coord, s->fragments[0].x, s->fragments[0].y);
    map[s->fragments[0].y][s->fragments[0].x] = '0';
    printf("0");
}

void tryChangeDir(struct Snake* s, char dir) {
    if (dir == 'w' || dir == 's' || dir == 'a' || dir == 'd') {
        switch (s->direction) {
            case 'w':
                if (dir != 's') { s->direction = dir; }
                break;
            case 's':
                if (dir != 'w') { s->direction = dir; }
                break;
            case 'a':
                if (dir != 'd') { s->direction = dir; }
                break;
            case 'd':
                if (dir != 'a') { s->direction = dir; }
                break;
        }
    }
}

Snake * createSnake(SHORT mapHeight, SHORT mapWidth) {
    Snake * s = (Snake*) calloc(1, sizeof(Snake));
    s->fragments = (Position*) calloc((mapHeight - 1) * (mapWidth - 1), sizeof(Position));
    s->init = init;
    s->move = move;
    s->tryChangeDir = tryChangeDir;
    return s;
}

char** createMap(Snake* snake, SHORT mapHeight, SHORT mapWidth) {
    char** map = (char**)calloc(mapHeight, sizeof(char*));

    for (int i = 0; i < mapHeight; i++) {
        char* line = (char*)calloc((mapWidth + 1), sizeof(char));
        for (int j = 0; j < mapWidth; j++) {
            if (i == 0 || i == mapWidth - 1 || j == 0
                || j == mapHeight - 1) {
                line[j] = '#';
            }
            else {
                line[j] = ' ';
            }
        }
        line[mapWidth] = '\n';
        map[i] = line;
    }

    map[snake->fragments[0].y][snake->fragments[0].x] = '0';
    for (int i = 1; i < snake->size; ++i) {
        map[snake->fragments[i].y][snake->fragments[i].x] = '*';
    }

    return map;
}

void draw(char** map, SHORT mapHeight, SHORT mapWidth) {
    system("cls");
    char* mapString = (char*)calloc((mapHeight * (mapWidth + 1)), sizeof(char));
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j <= mapWidth; j++) {
            mapString[(i * (mapWidth + 1)) + j] = map[i][j];
        }
    }
    printf("%s", mapString);
    free(mapString);
}

bool checkFruitCollision(Position nextPosition, char** map) {
    return (map[nextPosition.y][nextPosition.x] == '$') ? true : false;
}

void fruitAppearance(COORD* coord, char** baseMap, SHORT mapHeight, SHORT mapWidth){

    Position possible[(mapHeight - 2) * (mapWidth - 2) - 2]; // -2 walls and -2 for base snake size
    int sizeOfPossible = 0;
    int skippedCounter = 0;

    for (int i = 1; i < mapHeight - 1; i++) {
        for (int j = 1; j < mapWidth - 1; j++) {
            if (baseMap[i][j] == ' '){
                ++sizeOfPossible;
                possible[((i - 1) * (mapWidth - 2)) + (j - 1) - skippedCounter] = (Position){j, i};
            } else {
                ++skippedCounter;
            }
        }
    }

    int randomIndex = rand() % sizeOfPossible;
    baseMap[possible[randomIndex].y][possible[randomIndex].x] = '$';
    gotoXY(coord, possible[randomIndex].x, possible[randomIndex].y);
    printf("$");
}

void fruitEaten(Snake* snake) {
    snake->fragments[(snake->size)++] = snake->tile;
}

void hide_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void clearInputBuffer() {
    while (_kbhit()) {
        _getch();
    }
}

void gotoXY (COORD* coord,  SHORT x, SHORT y) {
    coord->X = x; coord->Y = y; // X and Y are the coordinates
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), *coord);
}

//void show_cursor() {
//    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//    CONSOLE_CURSOR_INFO cursorInfo;
//    GetConsoleCursorInfo(hConsole, &cursorInfo);
//    cursorInfo.bVisible = TRUE;
//    SetConsoleCursorInfo(hConsole, &cursorInfo);
//}

int main() {
    const SHORT mapHeight = 20;
    const SHORT mapWidth = 20;
    const SHORT snakeStartX = 14;
    const SHORT snakeStartY = 14;
    const char snakeStartDirection = 'w'; //wsad

    bool playing = true;
    bool gameRunning = true;
    bool win = false;
    const int winCounterConst = (mapHeight - 2) * (mapWidth - 2) - 2;

    srand(time(NULL));
    hide_cursor();
    COORD coord = {0, 0};

    while (gameRunning) {
        int winCounter = winCounterConst;
        Snake * snake = createSnake(mapHeight, mapWidth);
        char** baseMap = createMap(snake, mapHeight, mapWidth);
        snake->init(snake, snakeStartX, snakeStartY, snakeStartDirection);
        Position nextPosition = {0, 0};


        fruitAppearance(&coord, baseMap, mapHeight, mapWidth);

        draw(baseMap, mapHeight, mapWidth);

        while (playing) {

            if (_kbhit()) { snake->tryChangeDir(snake, _getch()); }
            nextPosition = tryMove(snake);

            if(checkGameOver(nextPosition, baseMap)){ playing = false; }

            if(checkFruitCollision(nextPosition, baseMap)) {
                fruitEaten(snake);
                snake->move(snake, &coord, baseMap, nextPosition);
                fruitAppearance(&coord, baseMap, mapHeight, mapWidth);
                if(--winCounter == 0) { win = true; }
            } else {
                snake->move(snake, &coord, baseMap, nextPosition);
            }

            Sleep(200);
        }

        char* message = win ? "     YOU WON     " : "    GAME OVER    ";
        gotoXY(&coord, 0, mapHeight);
        printf(  "*******************"
                 "\n*                 *"
                 "\n*%s*"
                 "\n*                 *\n"
                 "*******************\n\n"
                 "press q tu quit or eny other button to play again",
                 message);

        Sleep(100);
        clearInputBuffer();
        Sleep(700);

        if (_getch() == 'q') {
            gameRunning = false;
        } else {
            playing = true;
        }

        free(snake->fragments);
        free(snake);
        free2DMemory(baseMap, mapHeight);
    }

    return 0;
}