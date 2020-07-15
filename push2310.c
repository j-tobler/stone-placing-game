#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
/* The maximum permitted length of a gameboard width or height number */
#define MAX_DIMENSIONAL_DIGITS 10

/* One of the two players in the game */
typedef struct {
    char playerIcon;
    char playerType;
    int score;
} Player;

/* An (R,C) position on the grid */
typedef struct {
    int row;
    int column;
} Point;

/* Represents a playable square on the game board, e.g. '0.', '2X' */
typedef struct {
    int value;
    char playerIcon;
} Square;

/* A gameboard, composed of a grid of squares */
typedef struct {
    int height;
    int width;
    Square **grid;
} GameBoard;

/**
 * Represents the four directions one can use to navigate the gameboard:
 * Right, Down, Left, Up (respectively)
 */
const char DIRECTIONS[4] = {'r', 'd', 'l', 'u'};

void place_on_grid(Player player, Point position, GameBoard *gameBoard);
void shift_stones(Point position, GameBoard *gameBoard, char direction);
void print_grid(GameBoard gameBoard, FILE *fp);
void sum_scores(Player *player, GameBoard gameBoard);
void save_file(GameBoard gameBoard, Player playerToAct, char *filename);
void load_file(FILE *fp, int *height, int *width, char *playerToActIcon,
        Square ***grid);
int is_valid_save_file(FILE *fp);
int is_valid_position(Point position, GameBoard gameBoard);
int is_interior_full(GameBoard gameBoard);
int is_corner(Point position, GameBoard gameBoard);
int infer_input_type(char *input);
Point get_adjacent_position(Point position, char direction);
Point make_move_bot_0(Player bot, GameBoard *gameBoard);
Point make_move_bot_1(Player bot, GameBoard *gameBoard);
Square get_square(Point point, GameBoard gameBoard);

int main(int argc, char** argv) {
    // Verify args and assign pointer to the passed save file
    if (argc != 4) {
        fprintf(stderr, "Usage: push2310 typeO typeX fname\n");
        exit(1);
    }
    for (int i = 1; i <= 2; i++) {
        if (strcmp(argv[i], "0") && strcmp(argv[i], "1") && 
                strcmp(argv[i], "H")) {
            fprintf(stderr, "Invalid player type\n");
            exit(2);
        }
    }
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        fprintf(stderr, "No file to load from\n");
        exit(3);
    }
    if (!is_valid_save_file(fp)) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }
    
    // Initialise game
    int height, width;
    char playerToActIcon;
    Square **grid;


    load_file(fp, &height, &width, &playerToActIcon, &grid);


    fclose(fp);
    GameBoard gameBoard = {height, width, grid};
    if (is_interior_full(gameBoard)) {
        fprintf(stderr, "Full board in load\n");
        exit(6);
    }
    Player playerO = {'O', argv[1][0], 0};
    Player playerX = {'X', argv[2][0], 0};
    Player *playerToAct = playerToActIcon == 'O' ? &playerO : &playerX;

    print_grid(gameBoard, stdout);
    
    // Play game
    while (1) {
        Point movePosition;
        if (playerToAct->playerType == 'H') {
            // Prompt for and process human input
            char *input = malloc(sizeof(char));
            while (1) {
                printf("%c:(R C)> ", playerToAct->playerIcon);
                char c;
                int i = 0;
                // Get input
                while ((c = getchar()) != '\n') {
                    if (c == EOF) {
                        fprintf(stderr, "End of file\n");
                        exit(5);
                    }
                    input[i++] = c;
                    input = realloc(input, (i + 1) * sizeof(char));
                }
                input[i] = '\0';
                // Identify input type
                int inputType = infer_input_type(input);
                if (inputType == 1) {
                    // Coordinate input; validate coordinates
                    movePosition. row = atoi(strtok(input, " "));
                    movePosition.column = atoi(strtok(NULL, " "));
                    if (is_valid_position(movePosition, gameBoard)) {
                        break;
                    }
                }
                if (inputType == 2) {
                    // Save command; try to save the game with this filename
                    save_file(gameBoard, *playerToAct, input + 1);
                }
            }
            free(input);
            // Coordinate input verified; place stone on the gameboard
            place_on_grid(*playerToAct, movePosition, &gameBoard);
        } else if (playerToAct->playerType == '0') {
            // Place a stone according to automated player type 0's algorithm
            movePosition = make_move_bot_0(*playerToAct, &gameBoard);
            printf("Player %c placed at %d %d\n", playerToAct->playerIcon, 
                    movePosition.row, movePosition.column);
        } else {
            // Place a stone according to automated player type 1's algorithm
            movePosition = make_move_bot_1(*playerToAct, &gameBoard);
            printf("Player %c placed at %d %d\n", playerToAct->playerIcon, 
                    movePosition.row, movePosition.column);
        }
        print_grid(gameBoard, stdout);
        if (is_interior_full(gameBoard)) {
            break;
        }
        playerToAct = playerToAct == &playerO ? &playerX : &playerO;
    }
    // Game over; determine player scores and print the winner
    sum_scores(&playerO, gameBoard);
    sum_scores(&playerX, gameBoard);
    if (playerO.score > playerX.score) {
        printf("Winners: O\n");
    } else if (playerO.score < playerX.score) {
        printf("Winners: X\n");
    } else {
        printf("Winners: O X\n");
    }
    
    // Free gameboard pointers and exit
    for (int i = 0; i < height; i++) {
        free(gameBoard.grid[i]);
    }
    free(gameBoard.grid);
    return 0;
}

/**
 * Reads a game save file and checks whether it is formatted correctly and
 * does not contain any contradictions related to board dimensions.
 * @param fp pointer to the file to be validated.
 * @return 1 if this file is a valid game save file, else 0.
 */
int is_valid_save_file(FILE *fp) {
    rewind(fp);
    // Read width and height
    char c;
    // Allocate enough memory for two dimensions (width + height), one space
    // character and one terminating character
    int buffer = MAX_DIMENSIONAL_DIGITS * 2 + 2;
    char dimensions[buffer * sizeof(char)];
    int i = 0;
    // Verify there is a height dimension
    if (!isdigit(dimensions[i++] = getc(fp))) {
        return 0;
    }
    // Verify any remaining height digits
    while ((c = getc(fp)) != ' ') {
        if (i == buffer - 1 || !isdigit(c)) {
            return 0;
        }
        dimensions[i++] = c;
    }
    dimensions[i++] = c;
    // Verify there is a width dimension
    if (!isdigit(dimensions[i++] = getc(fp))) {
        return 0;
    }
    // Verify any remaining width digits
    while ((c = getc(fp)) != '\n') {
        if (i == buffer - 1 || !isdigit(c)) {
            return 0;
        }
        dimensions[i++] = c;
    }
    // Store width and height as ints and verify these dimensions
    dimensions[i] = '\0';
    int height = atoi(strtok(dimensions, " "));
    int width = atoi(strtok(NULL, " "));
    if (height < 3 || width < 3) {
        return 0;
    }

    // Verify player to act
    if ((c = getc(fp)) != 'O' && c != 'X') {
        return 0;
    }
    if ((c = getc(fp)) != '\n') {
        return 0;
    }

    // Verify grid
    for (int row = 0; row < height; row++) {
        for (int column = 0; column < width * 2 + 1; column++) {
            c = getc(fp);
            // Check that corners are empty
            if ((row == 0 || row == height - 1) && 
                    (column == 0 || column == 1 ||
                    column == width * 2 - 2 || column == width * 2 - 1)) {
                if (c == ' ') {
                    continue;
                } else {
                    return 0;
                }
            }
            // Check that a new line exists at the end of each row
            if (column == width * 2) {
                if (c == '\n') {
                    continue;
                } else {
                    return 0;
                }
            }
            // Check that every odd column (1st, 3rd, 5th...) contains a digit
            // and every even column (2nd, 4th, 6th...) contains a player icon
            // or period
            if (column % 2 == 0) {
                if (!isdigit(c)) {
                    return 0;
                }
            } else {
                if (c != '.' && c != 'O' && c != 'X') {
                    return 0;
                }
            }
        }
    }
    return getc(fp) == EOF ? 1 : 0;
}

/**
 * Reads from a valid save file and stores various data in the variables
 * pointed to by the appropriate passed pointers.
 * @param fp pointer to the valid save file to read from.
 * @param height pointer to the variable in which to store board height.
 * @param wdith pointer to the variable in which to store board width.
 * @param playerToActIcon pointer to the variable in which to store the icon
 * of the player to act next.
 * @param grid pointer to the 2d array in which to store gameboard contents.
 */
void load_file(FILE *fp, int *height, int *width, char *playerToActIcon,
        Square ***grid) {
    rewind(fp);
    char c;
    // Load dimensions
    int i = 0;
    char dimensions[MAX_DIMENSIONAL_DIGITS];
    while ((c = getc(fp)) != '\n') {
        dimensions[i++] = c;
    }
    dimensions[i] = '\0';
    *height = atoi(strtok(dimensions, " "));
    *width = atoi(strtok(NULL, " "));
    // Load player to act
    *playerToActIcon = getc(fp);
    getc(fp);
    // Resize grid to match height/width dimensions
    *grid = malloc(*height * sizeof(Square*));
    for (int row = 0; row < *height; row++) {
        (*grid)[row] = malloc(*width * sizeof(Square));
    }
    // Load grid contents
    for (int row = 0; row < *height; row++) {
        for (int column = 0; column < *width; column++) {
            int value = getc(fp) - '0';
            char playerIcon = getc(fp);
            Square square = {value, playerIcon};
            (*grid)[row][column] = square;
        }
        getc(fp);
    }
}

/**
 * Places a given player's stone on this gameboard in the specified position,
 * shifting stones if appropriate.
 * Has the effect of changing the player icon of the square at this position
 * to that of the given player's player icon.
 * Does not protect against invalid positions.
 * @param player the player who's stone is to be placed on the gameboard.
 * @param position to place a stone at on the gameboard.
 * @param gameBoard on which to place the stone.
 */
void place_on_grid(Player player, Point position, GameBoard *gameBoard) {
    // Place the stone
    gameBoard->grid[position.row][position.column].playerIcon =
            player.playerIcon;
    // If position is on an edge, shift the other stones appropriately
    if (position.row == 0) {
        shift_stones(position, gameBoard, 'd');
    } else if (position.row == gameBoard->height - 1) {
        shift_stones(position, gameBoard, 'u');
    } else if (position.column == 0) {
        shift_stones(position, gameBoard, 'r');
    } else if (position.column == gameBoard->width - 1) {
        shift_stones(position, gameBoard, 'l');
    }
}

/**
 * Starting from the stone at the specified position, shifts all consecutive
 * stones in the specified direction.
 * Does not protect against invalid stone shifts.
 * @param position of the first stone to shift.
 * @param gameBoard on which to shift the stones.
 * @param direction in which to shift the stones. See @DIRECTIONS.
 */
void shift_stones(Point position, GameBoard *gameBoard, char direction) {
    // Check for an empty adjacent square in the passed direction, and if a
    // stone was found instead, repeat this step for this second stone, and
    // so on until we find an empty square.
    Point adjacentPosition = get_adjacent_position(position, direction);
    if (get_square(adjacentPosition, *gameBoard).playerIcon != '.') {
        shift_stones(adjacentPosition, gameBoard, direction);
    }
    // When an empty adjacent square is found, move the last stone into it,
    // then move the second last stone into the position of the last stone,
    // and so on until we have shifted all stones in the specified direction
    gameBoard->grid[adjacentPosition.row][adjacentPosition.column].playerIcon
            = get_square(position, *gameBoard).playerIcon;
    gameBoard->grid[position.row][position.column].playerIcon = '.';
}

/**
 * Gets the position which is adjacent to the passed position in the direction
 * of the passed direction.
 * Does not protect against invalid positions.
 * @param position from which to find the adjacent square.
 * @param direction in which to look for an adjacent square. See @DIRECTIONS.
 */
Point get_adjacent_position(Point position, char direction) {
    Point newPosition;
    switch(direction) {
        case 'u':
            newPosition.row = position.row - 1;
            newPosition.column = position.column;
            break;
        case 'd':
            newPosition.row = position.row + 1;
            newPosition.column = position.column;
            break;
        case 'l':
            newPosition.row = position.row;
            newPosition.column = position.column - 1;
            break;
        case 'r':
            newPosition.row = position.row;
            newPosition.column = position.column + 1;
            break;
    }
    return newPosition;
}

/**
 * Prints the grid of the this gameboard to the file pointed to by the given
 * pointer.
 * @param gameBoard of which to print the grid.
 * @param fp pointer to the file to print the grid to.
 */
void print_grid(GameBoard gameBoard, FILE *fp) {
    for (int row = 0; row < gameBoard.height; row++) {
        for (int column = 0; column < gameBoard.width; column++) {
            Point position = {row, column};
            if (is_corner(position, gameBoard)) {
                fprintf(fp, "  ");
            } else {
                Square square = get_square(position, gameBoard);
                fprintf(fp, "%d%c", square.value, square.playerIcon);
            }
        }
        fprintf(fp, "\n");
    }
}

/**
 * Takes a null-terminated string of stdin user input and infers what
 * type of input it is, based on the syntax.
 * Save command syntax is "s%s".
 * Coordinate syntax is "%d %d".
 * @param input stdin user input to be evaluated.
 * @return 2 if input is a save command, or 1 if input is a grid coordinate,
 * or 0 if the syntax of the input is incorrect.
 */
int infer_input_type(char *input) {
    int i = 0;
    // Assume input type from first char
    if (isdigit(input[i])) {
        // Verify input as coordinate
        while (input[++i] != ' ') {
            if (!isdigit(input[i])) {
                return 0;
            }
        }
        while (input[++i] != '\0') {
            if (!isdigit(input[i])) {
                return 0;
            }
        }
        // Input satisfies grid coordinate syntax
        return 1;
    } else if (input[i] == 's' && strlen(input) > 1) {
        // Input satisfies save command syntax
        return 2;
    } else {
        // Input contains invalid syntax
        return 0;
    }
}

/**
 * Checks if the square at the passed position is a valid place to put a stone
 * on this gameboard. 
 * @param position at which to place the stone.
 * @param gameBoard on which to place to stone.
 * @return 1 if the position is valid, else 0.
 */
int is_valid_position(Point position, GameBoard gameBoard) {
    int row = position.row;
    int column = position.column;
    // Check if the position is within the gameboard
    if (row < 0 || row >= gameBoard.height) {
        return 0;
    }
    if (column < 0 || column >= gameBoard.width) {
        return 0;
    }
    if (is_corner(position, gameBoard)) {
        return 0;
    }
    // Check if it is on a dot
    if (get_square(position, gameBoard).playerIcon != '.') {
        return 0;
    }
    // Check if it is on an inside square
    if (row > 0 && row < gameBoard.height - 1 && column > 0 && 
            column < gameBoard.width - 1) {
        return 1;
    }
    // The position is on an edge; get the inward-facing direction and the
    // length of the gameboard in this direction
    char adjacentDirection;
    int boardLen;
    if (row == 0) {
        adjacentDirection = 'd';
        boardLen = gameBoard.height;
    } else if (row == gameBoard.height - 1) {
        adjacentDirection = 'u';
        boardLen = gameBoard.height;
    } else if (column == 0) {
        adjacentDirection = 'r';
        boardLen = gameBoard.width;
    } else if (column == gameBoard.width - 1) {
        adjacentDirection = 'l';
        boardLen = gameBoard.width;
    } else {
        // Uknown error: no such coordinates exist
        return 0;
    }
    // Check that there is an adjacent stone
    Point adjacentPosition = get_adjacent_position(
            position, adjacentDirection);
    if (get_square(adjacentPosition, gameBoard).playerIcon == '.') {
        return 0;
    }
    // Check that there is a blank space for stones to be pushed into
    for (int i = 0; i < boardLen - 2; i++) {
        adjacentPosition = get_adjacent_position(
                adjacentPosition, adjacentDirection);
        if (get_square(adjacentPosition, gameBoard).playerIcon == '.') {
            return 1;
        }
    }
    return 0;
}

/**
 * Gets the square located at the given position on the given gameboard.
 * Does not protect against invalid positions.
 * @param position at which to get the square.
 * @param gameBoard on which to get the square at the given position.
 */
Square get_square(Point position, GameBoard gameBoard) {
    return gameBoard.grid[position.row][position.column];
}

/**
 * Checks whether this gameboard is full.
 * Gameboards are full when there are no empty interior squares.
 * @param gameBoard of which to evaluate the fullness.
 * @return 1 if the gameboard is full, else 0.
 */
int is_interior_full(GameBoard gameBoard) {
    for (int row = 1; row < gameBoard.height - 1; row++) {
        for (int column = 1; column < gameBoard.width - 1; column++) {
            Point position = {row, column};
            if (get_square(position, gameBoard).playerIcon == '.') {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * Assigns this player's score according to the total value of their captured
 * squares on the given game board.
 * @param player pointer to the player of whom to sum and store the scores.
 * @param gameBoard to scan for squares occupied by the player.
 */
void sum_scores(Player *player, GameBoard gameBoard) {
    for (int row = 0; row < gameBoard.height; row++) {
        for (int column = 0; column < gameBoard.width; column++) {
            Point position = {row, column};
            if (!is_corner(position, gameBoard)) {
                // If we find a square with the player's icon on it, add the
                // value of that square to the player's score attribute
                Square square = get_square(position, gameBoard);
                if (square.playerIcon == player->playerIcon) {
                    player->score += square.value;
                }
            }
            
        }
    }
}

/**
 * Logic for automated player type 0 to make a move on the given gameboard.
 * If the player's icon is 'O', the player searches the interior of the 
 * gameboard from left-to-right, top-to-bottom until a valid move position
 * is found.
 * If the player's icon is 'X', the player searches the interior of the 
 * gameboard from right-to-left, bottom-to-top until a valid move position
 * is found.
 * @param bot the automated player to make the move.
 * @param gameBoard pointer to the gameboard for the bot to make a move on.
 * @return the position of the placed stone, or a (0,0) position if no valid
 * position was found.
 */
Point make_move_bot_0(Player bot, GameBoard *gameBoard) {
    if (bot.playerIcon == 'O') {
        // Search left-to-right, top-to-bottom for an empty square
        for (int row = 1; row < gameBoard->height - 1; row++) {
            for (int column = 1; column < gameBoard->width - 1; column++) {
                Point position = {row, column};
                if (get_square(position, *gameBoard).playerIcon == '.') {
                    place_on_grid(bot, position, gameBoard);
                    return position;
                }
            }
        }
    } else if (bot.playerIcon == 'X') {
        // Search right-to-left, bottom-to-top for an empty square
        for (int row = gameBoard->height - 2; row > 0; row--) {
            for (int column = gameBoard->width - 2; column > 0; column--) {
                Point position = {row, column};
                if (get_square(position, *gameBoard).playerIcon == '.') {
                    place_on_grid(bot, position, gameBoard);
                    return position;
                }
            }
        }
    }
    Point point = {0, 0};
    return point;
}

/**
 * Logic for automated player type 1 to make a move on the given gameboard.
 *
 * Scan, in order, the top row (left to right), right column (top to bottom),
 * bottom row (right to left), and left column (bottom to top), for valid
 * positions to place a stone. If a valid position is found, scan the
 * perpendicular row/column up until an unoccupied square is found. In the
 * area we scanned, sum both the value of each square occupied by the
 * opponent, and each square in front of such squares. The difference between
 * these two numbers is the change in the opponent's score that will occur if
 * a stone is placed at the valid position found. If the opponent's score
 * would decrease, place a stone here. 
 * 
 * If we did not find a valid position that reduces the opponent's score, scan
 * the interior of the board left-to-right top-to-bottom for the highest
 * valued square, and place a stone here.
 * If a tie is found, place a stone at the first square found with this value.
 * 
 * @param bot the automated player to make the move.
 * @param gameBoard pointer to the gameboard for the bot to make a move on.
 * @return the position of the placed stone, or a (0,0) position if no valid
 * position was found.
 */
Point make_move_bot_1(Player bot, GameBoard *gameBoard) {
    char opponentsIcon = bot.playerIcon == 'O' ? 'X' : 'O';
    Point position = {0, 0};
    // The length of the edge we are searching across to find a valid position
    int edgeLen = gameBoard->width;
    for (int i = 0; i < 4; i++) {
        // The direction in which to look for a valid external position
        char externalDirection = DIRECTIONS[i];
        // The perpendicular direction in which to scan for opponent's squares
        char internalDirection = DIRECTIONS[(i + 1) % 4];
        for (int i = 1; i < edgeLen; i++) {
            // Increment across this edge to find a valid position
            position = get_adjacent_position(position, externalDirection);
            if (!is_valid_position(position, *gameBoard)) {
                continue;
            }
            // Check if playing here will decrease the opponent's score;
            // (futureScore - currentScore) represents the change in the
            // opponent's score that will occur if we place a stone here
            int currentScore = 0;
            int futureScore = 0;
            // Until we find an empty square, search perpendicular to the
            // direction in which we were incrementing along the edge, for any
            // internal squares occupied by the opponent
            Point internalPosition = get_adjacent_position(
                    position, internalDirection);
            Square square;
            while ((square = get_square(
                    internalPosition, *gameBoard)).playerIcon != '.') {
                if (square.playerIcon == opponentsIcon) {
                    // We found an opponent's stone; add the value of this
                    // square to the currentScore and add the value of the
                    // next square to the futureScore
                    currentScore += square.value;
                    Square futureSquare = get_square(get_adjacent_position(
                            internalPosition, internalDirection), *gameBoard);
                    futureScore += futureSquare.value;
                }
                internalPosition = get_adjacent_position(
                        internalPosition, internalDirection);
            }
            // Check if the opponent's score would decrease if we play here
            if (futureScore < currentScore) {
                place_on_grid(bot, position, gameBoard);
                return position;
            }
        }
        // When we reach a corner, alternate the edge length between the
        // gameboard's height and width so we search across the correct number
        // of squares
        edgeLen = edgeLen == gameBoard->height ? 
                gameBoard->width : gameBoard->height;
    }

    // There is no valid edge move that reduces the opponent's score;
    // Find the highest-valued internal square
    int maxValue = -1;
    Point maxPosition = {0, 0};
    for (int row = 1; row < gameBoard->height - 1; row++) {
        for (int column = 1; column < gameBoard->width - 1; column++) {
            Point position = {row, column};
            Square square = get_square(position, *gameBoard);
            if(is_valid_position(position, *gameBoard) && 
                    square.value > maxValue) {
                maxValue = square.value;
                maxPosition = position;
            }
        }
    }
    place_on_grid(bot, maxPosition, gameBoard);
    return maxPosition;
}

/**
 * Checks whether this position represents a corner on this gameboard.
 * @param position to check.
 * @param gameBoard on which to check this position.
 * @return 1 if this position represents a corner on this gameboard, else 0.
 */
int is_corner(Point position, GameBoard gameBoard) {
    if ((position.row == 0 || position.row == gameBoard.height - 1) &&
            (position.column == 0 || 
            position.column == gameBoard.width - 1)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Tries to write this game to a file with the given filename.
 * If filename is invalid, simply does nothing.
 * Valid filenames must not begin with '/'.
 * @param gameBoard of the game to save.
 * @param playerToAct the player to act next.
 * @param filename name of the file to save the game to.
 */
void save_file(GameBoard gameBoard, Player playerToAct, char *filename) {
    if (filename[0] == '/') {
        fprintf(stderr, "Save failed\n");
        return;
    }
    FILE *fp;
    fp = fopen(filename, "w");
    // Write dimensions and player to act
    fprintf(fp, "%d %d\n%c\n", gameBoard.height, gameBoard.width,
            playerToAct.playerIcon);
    // Write grid
    print_grid(gameBoard, fp);
    fclose(fp);
}
