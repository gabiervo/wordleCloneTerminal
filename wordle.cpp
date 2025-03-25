#include <cstdint>
#include <iostream>
#include <ncurses.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <utility>

class wordleWriter{
  public:
  uint8_t cursor_x = 0;
  uint8_t cursor_y;

  char textBox[5];
  //character and positions
  std::map<char, std::vector<int>> finalWord;
  std::string finalWordString;

  wordleWriter(std::string mode, std::string finalWord=""){
    if(mode == "online"){
      //make fetching words work online
    }

    if(mode == "offline"){
      if(finalWord != "" && checkWordIntegrity(finalWord)){
        finalWordString = finalWord;
        initializeFinalWord(finalWord);
      }
      else{
        //fetch word from dictionary, but since i havent made it just throw err
        finalWord = "error";
        initializeFinalWord(finalWord);
      }
    }
  }

  int checkWordIntegrity(std::string word){
    if(word.length()==5){return 1;}
    return 0;
  }

  void addCharacterToTextBox(char character){
    textBox[cursor_x] = character;
    if(cursor_x < 4){cursor_x++;}
  }

  void removeCharacterFromTextBox(){
    if(cursor_x > 0){cursor_x--;}
    textBox[cursor_x] = 0;
  }

  void initializeFinalWord(std::string word){
    finalWord.clear();
    for(int i = 0; i < 5; i++){
      //add letter position to finalWord
      finalWord[(char)word[i]].emplace_back(i);
    }
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
    //we need to do this conversion in this specific spot, idk why
    printer = std::to_string(chCheck).c_str();

    waddch(win, chCheck);
    move(1, 0);
    waddstr(win, printer);
    move(0, 0);
  }

  static void debugFinalWord(WINDOW* win, wordleWriter* game){
    move(5, 0);
    waddstr(win, game->finalWordString.c_str());
    move(6, 0);
    std::map<char, std::vector<int>>::iterator it;
    int iters = 0;
    for(it = game->finalWord.begin(); it != game->finalWord.end(); ++it){
      mvwaddch(win, 6, iters, it->first);
      iters++;
    }
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

  //init game
  wordleWriter game("offline", "tests");
  debug debugger;
  while(true){
    if(poll(&poller, 1, 100) == 1){
      chChecker = wgetch(stdscr);
      debugger.debugIn(stdscr, chChecker);
      if(chChecker == 27){break;}
    }
    wclear(stdscr);
    debugger.debugOut(stdscr);
    debug::debugFinalWord(stdscr, &game);
    wrefresh(stdscr);
  }
  endwin();
  return 0;
}
