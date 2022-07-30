#include <stdio.h>
#include <termios.h>
#include <unistd.h>     
#include <time.h>
#include <stdlib.h>
#include <ctype.h>


int *get_neighbors(int **field, int *neighbors, int x, int y)
{
  // init neighbors
  for (int i = 0; i < 9; i++) 
    neighbors[i] = 0;
  
  int cell_value;
  // check line
  for (int i = 0; i < 9; i++) {
    cell_value = field[i][x] - 1;
    if (cell_value >= 0)
      neighbors[cell_value] = 1;
  }

  // check column
  for (int i = 0; i < 9; i++) {
    cell_value = field[y][i] - 1;
    if (cell_value >= 0)
      neighbors[cell_value] = 1;
  }

  // check square
  for (int i = 0; i < 3; i++) {
    int cell_y = 3 * (y / 3) + i;
    for (int j = 0; j < 3; j++) {
      int cell_x = 3 * (x / 3) + j;
      if (cell_x == x && cell_y == y)
        continue;
      cell_value = field[cell_y][cell_x] - 1;
      if (cell_value >= 0)
        neighbors[cell_value] = 1;
    }
  } 
  
  return neighbors;
}


int is_square_checked(int *checked_cells)
{
  for (int i = 0; i < 9; i++) {
    if (checked_cells[i] != 1)
       return 0; 
  }
  return 1;
}


void clear_field_from_number(int **field, int number)
{
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (field[i][j] == number)
        field[i][j] = 0;
    }
  }
}


void clear_field(int **field)
{
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      field[i][j] = 0;
    }
  }
}


int is_all_cells_open(int **open_cells) {
  int sum = 0;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      sum += open_cells[i][j];
    }
  }
  
  return sum == 81;;
}


void open_random_cells(int **open_cells, int open_perstange) {
  int openned_cells = (9 * 9 * open_perstange + 99) % 100;
  for (int i = 0; i < openned_cells; i++) {
    int rand_y = rand() % 9;
    int rand_x = rand() % 9;
    open_cells[rand_y][rand_x] = 1; 
  }
} 


void print_field(int **field, int **open_cells, int c_row, int c_col, int lives, int tips, int is_correct_choice)
{
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (c_row == i && c_col == j) printf("\033[1;30m\e[104m");
      else if ((3 * (c_row/3) == 3 * (i/3)) &&(3 * (c_col/3) == 3 * (j/3))) 
        printf("\e[30m\e[104m");
      else if (c_row == i) printf("\e[0;30m\e[104m"); 
      else if (c_col == j) printf("\e[0;30m\e[104m");

      else if (field[i][j] == field[c_row][c_col] && open_cells[c_row][c_col] == 1) printf("\033[1;30m\e[44m");
      else printf("\e[0;30m\e[44m");
      
      if (open_cells[i][j] == 1) 
        printf(" %d \033[0m", field[i][j]);
      else if (i == c_row && j == c_col)
        if (is_correct_choice == 1) {
          printf("[ ]\033[0m");
        } else {
          printf("\e[1;91m[ ]\033[0m");
        } 
      else 
        printf("   \033[0m");

      if ((j + 1) % 3 == 0 && j < 8) {
        if (i == c_row) printf("\e[104m│\033[0m");
        else printf("\e[44m│\033[0m"); 
      }
      
    }

    if (i == 0) {
      printf("   \e[44m┏\e[0m");
      for (int i = 0; i < 11; i++) {
        printf("\e[44m━");
      }
      printf("\e[44m┓\e[0m");
    } else if (i == 1)
      printf("   \e[44m┃ Lives:  %d ┃\e[0m", lives);
    else if (i == 2)
      printf("   \e[44m┃ Tips:   %d ┃\e[0m", tips);

    printf("\n");
    if ((i + 1) % 3 == 0 && i < 8) {
      for (int j = 0; j < 9*3 + 2; j++) 
        if (j > 0 && (j+1) % 10 == 0) printf("\e[44m┼\033[0m");
        else if (j < 10 && c_col == (j) / 3) printf("\e[104m─\033[0m");
        else if (j > 9 && j < 20 && c_col == (j-1) / 3) printf("\e[104m─\033[0m");
        else if (j > 19 && c_col == (j-2) / 3) printf("\e[104m─\033[0m");
        else printf("\e[44m─\e[0m");
      if (i + 1 == 3) {
        printf("   \e[44m┗");
        for (int i = 0; i < 11; i++)
          printf("━");
        printf("┚\e[0m");
      }
      printf("\n");
    }
  }
}


int **generate_field() 
{
  // Init field 9x9 by 0
  int *rows = calloc(9 * 9, sizeof(int));
  int **table = malloc(9 * sizeof(int*));
  for (int i = 0; i < 9; i++) {
    table[i] = rows + (i * 9); 
  }
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      table[i][j] = 0;
    }
  } 

  // Set numbers at field 
  int *neighbors = malloc(9 * sizeof(int));
  int *checked_cells = malloc(9 * sizeof(int));
  int back = 0;
  int back_count = 0;
  // Numbers from 1 to 9
  for (int i = 1; i <= 9; i++) {
      if (back_count > 3) { 
        clear_field(table);
        i = 1;
        back_count = 0;
      }
      back = 0;
      for (int y = 0; y < 9 && !back; y+=3) {
        for (int x = 0; x < 9 && !back; x+=3) {
          for (int j = 0; j < 9; j++) checked_cells[j] = 0;
          while (1) {
            int cell_cord = rand() % 9;
            if (is_square_checked(checked_cells) == 1) {      
              clear_field_from_number(table, i);
              back_count += 1;
              i -= 1;
              back = 1;
              break;
            }
             
            if (checked_cells[cell_cord] != 0) continue;
            if (table[y+cell_cord/3][x+cell_cord%3] != 0) {
              checked_cells[cell_cord] = 1;
            } else {
              get_neighbors(table, neighbors, x+cell_cord%3, y+cell_cord/3);
              if (neighbors[i-1] == 0) {
                table[y+cell_cord/3][x+cell_cord%3] = i;
                break;
              } else {
                checked_cells[cell_cord] = 1;
              }
            }
            
          }
        }
      }
  }
  return table;
}


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
  // TODO: Ask to play again if user lose
  if (result == 0)
      game();

  // Set back terminal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}
