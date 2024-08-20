#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>
#include <stdbool.h>

#define TICK 10000

#define MAX_TAIL_LENGTH 256

#define WHITE_BLOCK 1
#define GREEN_BLOCK 2
#define RED_BLOCK 3

#define TILE_WIDTH 2
#define TILE_HEIGHT 1

enum DIRECTION {
  UP = 0,
  DOWN,
  RIGHT,
  LEFT
};

bool game_running = false;

int head_y, head_x;
struct tail_part {
  int y, x;
  struct tail_part *next;
};
struct tail_part tail_start = { 0, 0, NULL };
enum DIRECTION direction;
int food_y, food_x;

inline int random_between(int min, int max) {
    return min + rand() % (max - min + 1);
}

void draw_tile(int y, int x, chtype c) {
  for(size_t i = 0; i < TILE_HEIGHT; i++) {
    for(size_t j = 0; j < TILE_WIDTH; j++) {
      mvaddch(y * TILE_HEIGHT + i, x * TILE_WIDTH + j, c);
    }
  }
}

void draw_borders(int screen_height, int screen_width) {
  for(size_t i = 0; i < screen_width; i++) {
    draw_tile(0, i, ' ' | COLOR_PAIR(WHITE_BLOCK));
    draw_tile(screen_height - 1, i, ' ' | COLOR_PAIR(WHITE_BLOCK));
  }

  for(size_t i = 1; i < screen_height - 1; i++) {
    draw_tile(i, 0, ' ' | COLOR_PAIR(WHITE_BLOCK));
    draw_tile(i, screen_width - 1, ' ' | COLOR_PAIR(WHITE_BLOCK));
  }

  refresh();
}

void handle_resize(int sig) {
  endwin();
  refresh();
  clear();

  int screen_height = getmaxy(stdscr) / TILE_HEIGHT;
  int screen_width = getmaxx(stdscr) / TILE_WIDTH;

  draw_borders(screen_height, screen_width);
}

void init_game(int screen_height, int screen_width) {
  head_y = screen_height / 2;
  head_x = screen_width / 2;
  draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));

  refresh();
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

  int screen_height = getmaxy(stdscr) / TILE_HEIGHT;
  int screen_width = getmaxx(stdscr) / TILE_WIDTH;

  draw_borders(screen_height, screen_width);

  signal(SIGWINCH, handle_resize);

  init_game(screen_height, screen_width);
  
  int c;
  while(true) {
    c = getch();
    if(c == KEY_UP || c == 'k') {
      direction = UP;
      break;
    } else if(c == DOWN || c == 'j') {
      direction = KEY_DOWN;
      break;
    } else if(c == RIGHT || c == 'l') {
      direction = RIGHT;
      break;
    } else if(c == KEY_LEFT || c == 'h') {
      direction = LEFT;
      break;
    }
  }
  game_running = true;

  while(game_running) {
    switch(direction) {
      case UP: {
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(-1));
        head_y--;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      }
      case DOWN: {
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(-1));
        head_y++;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      }
      case RIGHT: {
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(-1));
        head_x++;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      }
      case LEFT: {
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(-1));
        head_x--;
        draw_tile(head_y, head_x, ' ' | COLOR_PAIR(GREEN_BLOCK));
        break;
      }
    }
    usleep(TICK);
    refresh();
  }
  return 0;
}

