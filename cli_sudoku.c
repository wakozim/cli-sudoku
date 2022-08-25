#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>


int max(int first, int second) 
{
  return (first > second) ? first : second;
}

struct Screen {
  int length; 
  int max_width;
};


void get_screen_max_size(struct Screen *screen) 
{
  struct winsize console;  
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &console);
  screen->max_width = console.ws_col - 1;
}


void print(struct Screen *screen, char line[], ...) {
  get_screen_max_size(screen);
  va_list vargs;
  va_start(vargs, line);
  char written_line[screen->max_width];
  int line_len = vsprintf(written_line, line, vargs);
  if (line_len > screen->max_width) {
    printf("Can't write more than max width symbols!"); 
    exit(1);
  }

  printf(written_line);
  
  for (int i = 0; i < line_len; i++)
    if (written_line[i] == '\n')
      screen->length += 1;
}


void clear_screen(struct Screen *screen)
{
  int screen_length = screen->length;
  for (int i = 0; i < screen_length; i++) {
    printf("\033[A");
    for (int j = 0; j < screen->max_width; j++) { 
      printf(" ");
    }
    printf("\r");
  }
  
  screen->length = 0;
}


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


void color_print_field(struct Screen *screen, int **field, int **open_cells, int c_row, int c_col, int lives, int tips, int is_correct_choice)
{
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (c_row == i && c_col == j) print(screen, "\033[1;30m\e[104m");
      else if ((3 * (c_row/3) == 3 * (i/3)) &&(3 * (c_col/3) == 3 * (j/3))) 
        print(screen, "\e[30m\e[104m");
      else if (c_row == i) print(screen, "\e[0;30m\e[104m"); 
      else if (c_col == j) print(screen, "\e[0;30m\e[104m");

      else if (field[i][j] == field[c_row][c_col] && open_cells[c_row][c_col] == 1) print(screen, "\033[1;30m\e[44m");
      else print(screen, "\e[0;30m\e[44m");
      
      if (open_cells[i][j] == 1) 
        print(screen, " %d \033[0m", field[i][j]);
      else if (i == c_row && j == c_col)
        if (is_correct_choice == 1) {
          print(screen, "[ ]\033[0m");
        } else {
          print(screen, "\e[1;91m[ ]\033[0m");
        } 
      else 
        print(screen, "   \033[0m");

      if ((j + 1) % 3 == 0 && j < 8) {
        if (i == c_row) print(screen, "\e[104m│\033[0m");
        else print(screen, "\e[44m│\033[0m"); 
      }
      
    }

    if (i == 0) {
      print(screen, "   \e[44m┏\e[0m");
      for (int i = 0; i < 11; i++) {
        print(screen, "\e[44m━");
      }
      print(screen, "\e[44m┓\e[0m");
    } else if (i == 1)
      print(screen, "   \e[44m┃ Lives:  %d ┃\e[0m", lives);
    else if (i == 2)
      print(screen, "   \e[44m┃ Tips:   %d ┃\e[0m", tips);

    print(screen, "\n");
    if ((i + 1) % 3 == 0 && i < 8) {
      for (int j = 0; j < 9*3 + 2; j++) 
        if (j > 0 && (j+1) % 10 == 0) print(screen, "\e[44m┼\033[0m");
        else if (j < 10 && c_col == (j) / 3) print(screen, "\e[104m─\033[0m");
        else if (j > 9 && j < 20 && c_col == (j-1) / 3) print(screen, "\e[104m─\033[0m");
        else if (j > 19 && c_col == (j-2) / 3) print(screen, "\e[104m─\033[0m");
        else print(screen, "\e[44m─\e[0m");
      if (i + 1 == 3) {
        print(screen, "   \e[44m┗");
        for (int i = 0; i < 11; i++)
          print(screen, "━");
        print(screen, "┚\e[0m");
      }
      print(screen, "\n");
    }
  }
}

void print_field(struct Screen *screen, int **field, int **open_cells, int c_row, int c_col, int lives, int tips)
{
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      
      if (open_cells[i][j] == 1 && i == c_row && j == c_col) 
          print(screen, "[%d]", field[i][j]);
      else if (open_cells[i][j] == 1)
          print(screen, " %d ", field[i][j]);
      else if (i == c_row && j == c_col)
          print(screen, "[ ]");
      else 
          print(screen, "   ");

      if ((j + 1) % 3 == 0 && j < 8) {
          print(screen, "│");
      } 
    }

    if (i == 0) {
      print(screen, "   ┏");
      for (int i = 0; i < 11; i++) {
        print(screen, "━");
      }
      print(screen, "┓");
    } else if (i == 1)
      print(screen, "   ┃ Lives:  %d ┃", lives);
    else if (i == 2)
      print(screen, "   ┃ Tips:   %d ┃", tips);

    print(screen, "\n");
    if ((i + 1) % 3 == 0 && i < 8) {
      for (int j = 0; j < 9*3 + 2; j++) 
        if (j > 0 && (j+1) % 10 == 0) print(screen, "┼");
        else print(screen, "─");
      if (i + 1 == 3) {
        print(screen, "   ┗");
        for (int i = 0; i < 11; i++)
          print(screen, "━");
        print(screen, "┚");
      }
      print(screen, "\n");
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


int game(struct Screen *screen, bool colorless)
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
  if (colorless == false)
      color_print_field(screen, field, open_cells, cursor_row, cursor_col, lives, tips, is_correct_choice);
  else
      print_field(screen, field, open_cells, cursor_row, cursor_col, lives, tips);
  
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
    clear_screen(screen); 
    
    
    if (colorless == false)
        color_print_field(screen, field, open_cells, cursor_row, cursor_col, lives, tips, is_correct_choice);
    else
        print_field(screen, field, open_cells, cursor_row, cursor_col, lives, tips);
    
    is_correct_choice = 1;
    if (is_all_cells_open(open_cells) == 1)
      break;  
  }
  
  clear_screen(screen);
  if (colorless == false) {
      if (lives == 0) {
          print(screen, "┏━━━━━━━━━━━━━┓\n\e[0m");
          print(screen, "┃  \e[31mYou lose!  \e[0m┃\n"); 
          print(screen, "┗━━━━━━━━━━━━━┛\n\e[0m");
      } else {
          print(screen, "┏━━━━━━━━━━━━┓\n\e[0m");
          print(screen, "┃  \e[32mYou win!  \e[0m┃\n"); 
          print(screen, "┗━━━━━━━━━━━━┛\n\e[0m");  
      }
  } else {
      if (lives == 0) {
          print(screen, "┏━━━━━━━━━━━━━┓\n");
          print(screen, "┃  You lose!  ┃\n"); 
          print(screen, "┗━━━━━━━━━━━━━┛\n");
      } else {
          print(screen, "┏━━━━━━━━━━━━┓\n");
          print(screen, "┃  You win!  ┃\n"); 
          print(screen, "┗━━━━━━━━━━━━┛\n");  
      }
  }
  sleep(2);
  clear_screen(screen);
  return 0;
}


int start(struct Screen *screen, bool colorless)
{
  if (colorless == false)
  {
      print(screen, "Hello in Sudoku Game by Lookins!\n");
      print(screen, "\n");
      print(screen, "Controls:\n");
      print(screen, "   \e[32mWASD\e[0m - to move cursor,\n");
      print(screen, "   \e[32m1-9\e[0m  - open cell with it number.\n");
      print(screen, "\n");
      print(screen, "Press \e[1;32mS\e[0m to start game.\n");
      print(screen, "Press \e[1:32mX\e[0m to exit programm.\n"); 
  } 
  else 
  {
      print(screen, "Hello in Sudoku Game by Lookins!\n");
      print(screen, "\n");
      print(screen, "Controls:\n");
      print(screen, "   WASD - to move cursor,\n");
      print(screen, "   1-9  - open cell with it number.\n");
      print(screen, "\n");
      print(screen, "Press S to start game.\n");
      print(screen, "Press X to exit programm.\n");
  
  }
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
        return 0;
        break;
    }
  }
  clear_screen(screen);
  return start_game;  
}


int main(int argc, char **argv)
{
  bool colorless = false;

  if (argc > 1) 
  {
      for (int i = 1; i < argc; i++) 
      {
          colorless = !strcmp(argv[i], "--colorless") || !strcmp(argv[i], "-c");
      }
  }
  
  // init screen struct to easy print manipulation
  struct Screen screen;
  get_screen_max_size(&screen);
  screen.max_width = 0;
  screen.length = 0;

  // seet seed for rand
  srand(time(NULL));
  
  // set terminal settings to ICANON and ECHO
  static struct termios oldt, newt;
  tcgetattr( STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);          
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  // game 
  while (start(&screen, colorless))
    game(&screen, colorless);
  // Set back terminal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}
