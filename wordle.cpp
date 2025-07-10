#include <cstdint>
#include <ncurses.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <utility>
#include <unistd.h>

#include "./include/writer.h"
#include "./include/generator.h"

//rng related
#include <cstdlib>
#include <ctime>
#define CORRECT_GREEN 9
#define INCORRECT_RED 10

class debug{
  public:
  int chCheck = 0;
  const char* printer;
  bool shouldDebug;

  void debugIn(WINDOW* win, int ch){
    chCheck = ch;
    if(ch == 32){shouldDebug = (shouldDebug == true) ? false : true;}
  }
  void debugOut(WINDOW* win){
    //we need to do this conversion in this specific spot, idk why
    printer = std::to_string(chCheck).c_str();

    wmove(win, 0, 0);
    waddch(win, chCheck);
    wmove(win, 1, 0);
    waddstr(win, printer);
  }

  static void debugFinalWord(WINDOW* win, wordleWriter* game){
    wmove(win, 2, 0);
    waddstr(win, game->finalWordString.c_str());
    wmove(win, 3, 0);
    std::map<char, std::vector<int>>::iterator it;
    int iters = 0;
    for(it = game->finalWord.begin(); it != game->finalWord.end(); ++it){
      mvwaddch(win, 3, 10+iters, it->first);
      iters++;
    }
  }
};

void rectangle(WINDOW* win, int y1, int x1, int y2, int x2){
    mvwhline(win, y1, x1, 0, x2-x1);
    mvwhline(win, y2, x1, 0, x2-x1);
    mvwvline(win, y1, x1, 0, y2-y1);
    mvwvline(win, y1, x2, 0, y2-y1);
    mvwaddch(win, y1, x1, ACS_ULCORNER);
    mvwaddch(win, y2, x1, ACS_LLCORNER);
    mvwaddch(win, y1, x2, ACS_URCORNER);
    mvwaddch(win, y2, x2, ACS_LRCORNER);
}

void drawWindowLetterBoxes(WINDOW* win, int windowInitX, int windowInitY=0, int windowHeight=4, int windowWidth=8){
  for(int j = 0; j < 5; j++){
    int initXPos = windowInitX + 9*j;
    rectangle(win, windowInitY, initXPos, windowHeight, initXPos + windowWidth);
  }
  wrefresh(win);
}

void drawCharacterOnTextBox(WINDOW* win, int curs_x, int curs_y, char newLetter){
  int row, col;
  getmaxyx(stdscr, row, col);
  int xPos = 4 + (curs_x) * 9;
  mvwaddch(win, 2, xPos, newLetter);
  wrefresh(win);
}

int main(){
  bool SHOULD_DEBUG = false;

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
  wordGenerator gen("/Users/gabrielalvesiervolino/Desktop/Coding/games/wordleClone/dictionary/dicts/");
  wordleWriter game("offline", gen.generateWord());
  debug debugger;

  std::vector<WINDOW*> resultsWindows;
  int currentWordIndex = 0;
  int row, col;
  getmaxyx(stdscr, row, col);
  WINDOW* debugWin = newwin(4, col, 0, 0);

  //0 is for play, 1 is for result scr
  int gameState = 0;
  std::string pastResults[6];

  start_color();
  //here so that ncurses doesn't change terminal background color
  init_color(CORRECT_GREEN, 278, 717, 423);
  init_color(COLOR_BLACK, 0, 0, 0);
  init_pair(1, COLOR_WHITE, COLOR_YELLOW);
  init_pair(2, COLOR_WHITE, CORRECT_GREEN);
  init_pair(3, COLOR_WHITE, COLOR_RED);
  use_default_colors();

  for(int i = 0; i < 6; i++){
    //8 pixel square for each letter with 1 pixel intervals in between
    resultsWindows.emplace_back(newwin(5, 48, 5 + (5*i), (col/2-24) + 1));
  }

  //better for debugging
  std::vector<std::vector<int>> ans;

  //an answer variable thats easier to use in game logic
  std::vector<int> gameAns;

  drawWindowLetterBoxes(resultsWindows[currentWordIndex], 0);

  while(true){
    if(gameState == 0){
      if(poll(&poller, 1, 100) == 1){
        chChecker = wgetch(stdscr);
        debugger.debugIn(debugWin, chChecker);
        //esc
        if(chChecker == 27){break;}

        //delete
        if(chChecker == 263){
          game.removeCharacterFromTextBox();
          drawCharacterOnTextBox(resultsWindows[currentWordIndex], game.cursor_x, game.cursor_y, ' ');
        }

        //enter
        else if(chChecker == 10){
          //checks if we have 5 letters in the word and if it is an actual word that exists
          if(game.setLetters == 5 && gen.checkWordExistence(game.textBox)){
            gameState = 1;
            ans = game.checkWordInput(debugWin);
            //gameAns = accessibleAnswer
            gameAns = ans[3];
            //gameAns = (std::vector<int>){2, 1, 0, 2, 2};
            pastResults[currentWordIndex] = game.textBox;
            if(currentWordIndex != 6){currentWordIndex++;}
            game.clearTextBox();
          }
        }
        else if(chChecker >= 65 && chChecker <= 122){
          drawCharacterOnTextBox(resultsWindows[currentWordIndex], game.cursor_x, game.cursor_y, chChecker);
          game.addCharacterToTextBox(chChecker);
          drawWindowLetterBoxes(resultsWindows[currentWordIndex], 0);
        }

        if(debugger.shouldDebug){
          wclear(debugWin);
          debugger.debugOut(debugWin);
          debug::debugFinalWord(debugWin, &game);

          for(int i = 0; i < ans.size(); i++){
            std::string category;
            if(i == 0){category = "correct";}
            if(i == 1){category = "semi";}
            if(i == 2){category = "incorrect";}

            mvwprintw(debugWin, 0+i, 20, category.c_str());
            for(int j = 0; j < ans[i].size(); j++){
              mvwprintw(debugWin, 0+i, 30+j, std::to_string(ans[i][j]).c_str());
            }
          }
          mvwprintw(debugWin, 3, 0, std::to_string(game.setLetters).c_str());
          game.wprintTextBox(debugWin);
        }
        else{wclear(debugWin); wrefresh(debugWin);}

      }
    }

    if(gameState == 1){
      for(int i = 0; i < 5; i++){
        int initXPos = 1 + 9*i;
        int printIndex = currentWordIndex-1;

        uint8_t printIndent = 3;

        int colorPairIndex = gameAns[i];
        //printing background
        for(int j = 0; j < 3; j++){
          int initYPos = j+1;
          wattron(resultsWindows[printIndex], COLOR_PAIR(colorPairIndex));
          mvwprintw(resultsWindows[printIndex], initYPos, initXPos, "       ");
        }
        mvwaddch(resultsWindows[printIndex], 2, printIndent+initXPos, pastResults[printIndex][i]);
        wattroff(resultsWindows[printIndex], COLOR_PAIR(colorPairIndex));
        wrefresh(resultsWindows[printIndex]);

        //half of a second
        usleep(100000*1);
      }
      gameState = 0;
    }
  }
  endwin();
  return 0;
}
