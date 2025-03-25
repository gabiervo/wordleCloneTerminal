#include <cstdint>
#include <iostream>
#include <ncurses.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>

class wordleWriter{
  public:
  uint8_t cursor_x = 0;
  uint8_t cursor_y;

  uint8_t textBox[5];

  void addCharacterToTextBox(uint8_t character){
    textBox[cursor_x] = character;
    if(cursor_x < 4){cursor_x++;}
  }

  void removeCharacterFromTextBox(){
    cursor_x--;
    textBox[cursor_x] = 0;
  }
};

class debug{
  public:
  int chCheck = 0;
  const char* printer;

  void debugIn(WINDOW* win, int ch){
    chCheck = ch;
  }
  void debugOut(WINDOW* win){
    //we need to do this conversion here, idk why
    printer = std::to_string(chCheck).c_str();

    waddch(win, chCheck);
    move(1, 0);
    waddstr(win, printer);
    move(0, 0);
  }
};

int main(){
  pollfd poller;
  memset(&poller, 0, sizeof(poller));
  poller.fd = 0;
  poller.events = POLLIN;

  int chChecker;

  //ncurses initialization
  initscr();
  noecho();
  curs_set(0);

  //necessary so that esc quits at once instead of delaying
  ESCDELAY = 0;
  keypad(stdscr, TRUE);

  debug debugger;
  while(true){
    if(poll(&poller, 1, 100) == 1){
      chChecker = wgetch(stdscr);
      debugger.debugIn(stdscr, chChecker);
      if(chChecker == 27){break;}
    }
    wclear(stdscr);
    debugger.debugOut(stdscr);
    wrefresh(stdscr);
  }
  endwin();
  return 0;
}
