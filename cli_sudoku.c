#include <stdio.h>
#include <termios.h>
#include <unistd.h>     
#include <time.h>
#include <stdlib.h>
#include <field_sudoku.c>
#include <ctype.h>

int game()
{ 
  int **field = generate_field();    
  int cursor_row = 0, cursor_col = 0;
  
  int *rows = calloc(9 * 9, sizeof(int));
  int **open_cells = malloc(9 * sizeof(int*));
  for (int i = 0; i < 9; i++) {
    open_cells[i] = rows + (i * 9); 
  }
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      open_cells[i][j] = 0;
    }
  }

  int open_cells_perstange = 45; 
  open_random_cells(open_cells, open_cells_perstange); 
  
  open_cells[1][5] = 1;
  int lives = 5;
  int tips = 1;
  int is_correct_choice = 1;
  print_field(field, open_cells, cursor_row, cursor_col, lives, tips, is_correct_choice);
  while (lives > 0) {
    char a = getchar();
    switch (a) {
      case 'w':
      case 'W':
        if (cursor_row > 0) cursor_row -= 1;
        break;
      case 'a':
      case 'A':
        if (cursor_col > 0) cursor_col -= 1;
        break;
      case 's':
      case 'S':
        if (cursor_row < 8) cursor_row += 1;
        break;
      case 'd':
      case 'D':
        if (cursor_col < 8) cursor_col += 1;
        break;
      case 'h':
      case 'H':
        if (tips > 0) {
          tips -= 1;
          open_cells[cursor_row][cursor_col] = 1;
          is_correct_choice = 1;
        }
        break;
      default :
        if (open_cells[cursor_row][cursor_col] == 1) 
           break;
        if (isdigit(a)) {
          for (int i = 0; i < 21; i++) printf(" ");
          if (field[cursor_row][cursor_col] == a - '0') { 
            open_cells[cursor_row][cursor_col] = 1;
            is_correct_choice = 1;
          } else {
            lives -= 1;
            is_correct_choice = 0; 
          }
         break;
      }
    }
    printf("\033[11A\r");  
    print_field(field, open_cells, cursor_row, cursor_col, lives, tips, is_correct_choice);
    is_correct_choice = 1;
    if (is_all_cells_open(open_cells) == 1)
      break;  
  }
  
  printf("\n");
  if (lives == 0) {
    printf("┏━━━━━━━━━━━━━┓\n\e[0m");
    printf("┃  \e[31mYou lose!  \e[0m┃\n"); 
    printf("┗━━━━━━━━━━━━━┛\n\e[0m");
  } else {
    printf("┏━━━━━━━━━━━━┓\n\e[0m");
    printf("┃  \e[32mYou win!  \e[0m┃\n"); 
    printf("┗━━━━━━━━━━━━┛\n\e[0m");  
  }
  
  return 0;
}


int print_help()
{
 printf("WASD - control\n");
}


int start()
{
  printf("Hello in Sudoku Game by Lookins!\n");
  printf("\n");
  printf("Controls:\n");
  printf("   \e[32mWASD\e[0m - to move cursor,\n");
  printf("   \e[32m1-9\e[0m  - open cell with it number.\n");
  printf("\n");
  printf("Press \e[1;32mS\e[0m to start game.\n");
  printf("Press \e[1:32mX\e[0m to exit programm.\n");
  
  int start_game = 0;
  while (!start_game) {
    char cmd = getchar();
    switch (cmd) {
      case 's':
      case 'S':
        start_game = 1;
        break;
      case 'x':
      case 'X':
        return 1;
        break;
    }
  }
  for (int i = 0; i < 8; i++) {
    printf("\e[A\r");
    for (int j = 0; j < 35; j++) 
      printf(" ");
    printf("\r");
  }
  return 0;  
}


int main()
{
  // seet seed for rand
  srand(time(NULL));
  // set terminal settings to ICANON and ECHO
  static struct termios oldt, newt;

  tcgetattr( STDIN_FILENO, &oldt);
  newt = oldt;

  newt.c_lflag &= ~(ICANON | ECHO);          

  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  // greeting and explain how play
  int result = start();
  
  // game 
  if (result == 0)
      game();

  // Set back terminal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}
