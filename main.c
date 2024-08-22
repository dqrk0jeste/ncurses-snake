#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>
#include <stdbool.h>

#define TICK 100000

#define WHITE_BLOCK 1
#define GREEN_BLOCK 2
#define RED_BLOCK 3

#define TILE_WIDTH 2
#define TILE_HEIGHT 1

int screen_height, screen_width;

enum DIRECTION {
  UP = 0,
  DOWN,
  RIGHT,
  LEFT
} direction;

bool game_running = false;

int head_y, head_x;

struct tail_part {
  int y, x;
  struct tail_part *next;
  struct tail_part *prev;
};
struct tail_part *tail_start = NULL;
struct tail_part *tail_end = NULL;

int food_y, food_x;
bool has_eaten_food = false;

int random_between(int min, int max) {
    return min + rand() % (max - min + 1);
}

void draw_tile(int y, int x, chtype c) {
  for(size_t i = 0; i < TILE_HEIGHT; i++) {
    for(size_t j = 0; j < TILE_WIDTH; j++) {
      mvaddch(y * TILE_HEIGHT + i, x * TILE_WIDTH + j, c);
    }
  }
}

void draw_borders() {
  for(size_t i = 0; i < screen_width; i++) {
    draw_tile(0, i, ' ' | COLOR_PAIR(WHITE_BLOCK));
    draw_tile(screen_height - 1, i, ' ' | COLOR_PAIR(WHITE_BLOCK));
  }

  for(size_t i = 1; i < screen_height - 1; i++) {
    draw_tile(i, 0, ' ' | COLOR_PAIR(WHITE_BLOCK));
    draw_tile(i, screen_width - 1, ' ' | COLOR_PAIR(WHITE_BLOCK));
  }
}

void init_snake() {
  head_y = screen_height / 2;
  head_x = screen_width / 2;
  draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
}

void generate_food() {
  bool valid = false;

  while(!valid) {
    food_y = random_between(1, screen_height - 2);
    food_x = random_between(1, screen_width - 2);

    if(food_x != head_x || food_y != head_y) {
      valid = true;
    }

    for(struct tail_part *part = tail_start; part != NULL && part != tail_end; part = part->next) {
      if(food_x == part->x && food_y == part->y) {
        valid = false;
        break;
      }
    }
  }

  
  draw_tile(food_y, food_x, ' ' | COLOR_PAIR(RED_BLOCK));
}

bool check_if_hit_itself() {
  for(struct tail_part *part = tail_start; part != NULL && part != tail_end; part = part->next) {
    if(head_y == part->y && head_x == part->x) {
      return true;
    }
  }
  return false;
}

bool check_if_hit_wall() {
  if(head_y == 0 || head_y == screen_height - 1
    || head_x == 0 || head_x == screen_width - 1
  ) {
    return true;
  }
  return false;
}

void handle_game_input(int c) {
  switch (c) {
    case 'r':
      game_running = false;
      break;
    case KEY_UP:
    case 'k':
      if(direction != DOWN) {
        direction = UP;
      }
      break;
    case KEY_DOWN :
    case 'j':
      if(direction != UP) {
        direction = DOWN;
      }
      break;
    case KEY_RIGHT:
    case 'l':
      if(direction != LEFT) {
        direction = RIGHT;
      }
      break;
    case KEY_LEFT:
    case 'h':
      if(direction != RIGHT) {
        direction = LEFT;
      }
      break;
  }
}

void move_snake() {
  int old_head_y = head_y;
  int old_head_x = head_x;

  if(has_eaten_food) {
    has_eaten_food = false;
    switch(direction) {
      case UP:
        head_y--;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      case DOWN:
        head_y++;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      case RIGHT:
        head_x++;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      case LEFT:
        head_x--;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
    }

    struct tail_part *new = (struct tail_part*)malloc(sizeof(struct tail_part));

    if(tail_start == NULL) {
      new->y = old_head_y;
      new->x = old_head_x;
      new->next = NULL;
      new->prev = NULL;

      tail_start = new;
      tail_end = NULL;

      draw_tile(tail_start->y, tail_start->x, ' ' | COLOR_PAIR(GREEN_BLOCK));
    } else if(tail_end == NULL) {
      new->y = old_head_y;
      new->x = old_head_x;
      new->next = tail_start;
      new->prev = NULL;

      tail_start->prev = new;
      tail_start->next = NULL;

      tail_end = tail_start;
      tail_start = new;

      draw_tile(tail_start->y, tail_start->x, ' ' | COLOR_PAIR(GREEN_BLOCK));
    } else {
      new->y = old_head_y;
      new->x = old_head_x;
      new->next = tail_start;
      new->prev = NULL;

      tail_start->prev = new;
      tail_start = new;

      draw_tile(tail_start->y, tail_start->x, ' ' | COLOR_PAIR(GREEN_BLOCK));
    }

    return;
  }

  switch(direction) {
    case UP:
      draw_tile(head_y, head_x, ' ');
      head_y--;
      draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
      break;
    case DOWN:
      draw_tile(head_y, head_x, ' ');
      head_y++;
      draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
      break;
    case RIGHT:
      draw_tile(head_y, head_x, ' ');
      head_x++;
      draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
      break;
    case LEFT:
      draw_tile(head_y, head_x, ' ');
      head_x--;
      draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
      break;
  }

  if(tail_start != NULL && tail_end == NULL) {
    draw_tile(tail_start->y, tail_start->x, ' ');

    tail_start->y = old_head_y;
    tail_start->x = old_head_x;
    tail_start->next = NULL;
    tail_start->prev = NULL;

    draw_tile(tail_start->y, tail_start->x, ' ' | COLOR_PAIR(GREEN_BLOCK));
  } else if(tail_end != NULL) {
    draw_tile(tail_end->y, tail_end->x, ' ');

    tail_end = tail_end->prev;
    struct tail_part *old_tail_end = tail_end->next;
    tail_end->next = NULL;

    old_tail_end->y = old_head_y;
    old_tail_end->x = old_head_x;
    old_tail_end->prev = NULL;
    old_tail_end->next = tail_start;

    tail_start->prev = old_tail_end;
    tail_start = old_tail_end;

    draw_tile(tail_start->y, tail_start->x, ' ' | COLOR_PAIR(GREEN_BLOCK));
  }
}

bool check_if_eaten_food() {
  return head_x == food_x && head_y == food_y;
}

int main() {
  srand(time(NULL));

  initscr();
  raw();
  keypad(stdscr, true);
  noecho();
  curs_set(false);

  start_color();
  use_default_colors();

  if (!has_colors()) {
    endwin();
    return 1;
  }

  init_pair(WHITE_BLOCK, COLOR_WHITE, COLOR_WHITE);
  init_pair(GREEN_BLOCK, COLOR_GREEN, COLOR_GREEN);
  init_pair(RED_BLOCK, COLOR_RED, COLOR_RED);

  screen_height = getmaxy(stdscr) / TILE_HEIGHT;
  screen_width = getmaxx(stdscr) / TILE_WIDTH;

  while(true) {
    clear();
    nodelay(stdscr, false);

    head_y = 0;
    head_x = 0;
    tail_start = NULL;
    tail_end = NULL;
    food_y = 0;
    food_x = 0;
    has_eaten_food = false;

    draw_borders();
    init_snake();
    generate_food();

    refresh();

    int c;
    bool valid_input = false;
    while(!valid_input) {
      c = getch();
      switch (c) {
        case 'q': 
          endwin();
          return 0;
        case KEY_UP:
        case 'k':
          direction = UP;
          valid_input = true;
          break;
        case KEY_DOWN :
        case 'j':
          direction = DOWN;
          valid_input = true;
          break;
        case KEY_RIGHT:
        case 'l':
          direction = RIGHT;
          valid_input = true;
          break;
        case KEY_LEFT:
        case 'h':
          direction = LEFT;
          valid_input = true;
          break;
      }
    }

    game_running = true;
    nodelay(stdscr, true);

    while(game_running) {
      c = getch();
      if (c != -1) {
        handle_game_input(c);
      }
      move_snake();
      if(check_if_hit_itself() || check_if_hit_wall()) {
        game_running = false;
      }
      if(check_if_eaten_food()) {
        has_eaten_food = true;
        generate_food();
      }
      refresh();
      usleep(TICK);
    }

    struct tail_part *part = tail_start;
    struct tail_part *tmp;
    while (part != NULL) {
       tmp = part;
       part = part->next;
       free(tmp);
    }
  }
}

