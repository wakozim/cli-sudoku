#include <stdio.h>
#include <termios.h>
#include <unistd.h>     
#include <time.h>
#include <stdlib.h>



unsigned long int randint() {
   
  FILE *urandom = fopen("/dev/urandom", "r");
  if (urandom == NULL) return 0;
  else {
    char random_data[25] = {'0'};
    fscanf(urandom, "%25s", random_data);
   
    unsigned long int random_number = 0;
    for (int i = 0; i < 25; i++) {
      random_number += random_data[i] % 10;
    }
    fclose(urandom);
    return random_number;
  }
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

  if (sum == 81)
    return 1;
  else
    return 0;
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
            int cell_cord = randint() % 9;
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
