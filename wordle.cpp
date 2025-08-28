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
    //space character - debug toggle
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

  static void debugFinalWord(WINDOW* win, wordleWriter* writer){
    wmove(win, 2, 0);
    waddstr(win, writer->finalWordString.c_str());
    wmove(win, 3, 0);
    std::map<char, std::vector<int>>::iterator it;
    int iters = 0;
    for(it = writer->finalWord.begin(); it != writer->finalWord.end(); ++it){
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

class Game{
  public:
  bool SHOULD_DEBUG = false;
  wordGenerator gen;
  wordleWriter writer;

  pollfd poller;
  debug debugger;

  std::vector<WINDOW*> resultsWindows;
  int currentWordIndex = 0;
  int row, col;
  WINDOW* debugWin;

  int chChecker;

  //0 is for play, 1 is for result scr
  int writerState = 0;
  std::string pastResults[6];

  //good answer for debugging
  std::vector<std::vector<int>> ans;

  //good answer for game logic
  std::vector<int> writerAns;

  Game(std::string generatorDir, std::string writerMode) : gen(generatorDir), writer(writerMode, gen.generateWord()){}

  static void gameSetup(){
    //sets up everything that only needs to be setup once
    //ncurses initialization
    initscr();
    noecho();
    curs_set(0);

    //necessary so that esc quits at once instead of delaying
    ESCDELAY = 0;
    keypad(stdscr, TRUE);

    start_color();
    //here so that ncurses doesn't change terminal background color
    init_color(CORRECT_GREEN, 278, 717, 423);
    init_color(COLOR_BLACK, 0, 0, 0);
    init_pair(1, COLOR_WHITE, COLOR_YELLOW);
    init_pair(2, COLOR_WHITE, CORRECT_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_RED);
    use_default_colors();
  }

  void drawAllWindowLetterBoxes(){
    for(int i = 0; i < 6; i++){
      drawWindowLetterBoxes(resultsWindows[i], 0);
    }
  }

  void gameInit(){

    getmaxyx(stdscr, row, col);
    debugWin = newwin(4, col, 0, 0);

    //initializes the rest of the class
    memset(&poller, 0, sizeof(poller));
    poller.fd = 0;
    poller.events = POLLIN;

    for(int i = 0; i < 6; i++){
      //8 pixel square for each letter with 1 pixel intervals in between
      resultsWindows.emplace_back(newwin(5, 48, 5 + (5*i), (col/2-24) + 1));
    }
    drawAllWindowLetterBoxes();
  }

  bool gameLoop(){
    bool shouldContinue = true;
    while(true){
      if(writerState == 0){
        if(poll(&poller, 1, 100) == 1){
          chChecker = wgetch(stdscr);
          debugger.debugIn(debugWin, chChecker);
          //esc
          if(chChecker == 27){
            shouldContinue = false;
            break;
          }

          //delete
          if(chChecker == 263){
            writer.removeCharacterFromTextBox();
            drawCharacterOnTextBox(resultsWindows[currentWordIndex], writer.cursor_x, writer.cursor_y, ' ');
          }

          //enter
          else if(chChecker == 10){
            //checks if we have 5 letters in the word and if it is an actual word that exists
            bool wordExists = gen.checkWordExistence(writer.textBox);
            if(writer.setLetters == 5 && wordExists){
              writerState = 1;
              ans = writer.checkWordInput(debugWin);
              //writerAns = accessibleAnswer
              writerAns = ans[3];
              pastResults[currentWordIndex] = writer.textBox;
              if(currentWordIndex != 6){currentWordIndex++;}
              writer.clearTextBox();
            }
            else if(!wordExists){
              for(int i = 0; i < 5; i++){
                int initXPos = 1 + 9*i;

                uint8_t printIndent = 3;

                int colorPairIndex = 3;
                //printing background
                for(int j = 0; j < 3; j++){
                  int initYPos = j+1;
                  wattron(resultsWindows[currentWordIndex], COLOR_PAIR(colorPairIndex));
                  mvwprintw(resultsWindows[currentWordIndex], initYPos, initXPos, "       ");
                }
                mvwaddch(resultsWindows[currentWordIndex], 2, printIndent+initXPos, writer.textBox[i]);
                wattroff(resultsWindows[currentWordIndex], COLOR_PAIR(colorPairIndex));
                wrefresh(resultsWindows[currentWordIndex]);

                //one fifth of a second
                usleep(100000/5);
              }
              for(int i = 0; i < 5; i++){
                int initXPos = 1 + 9*i;

                uint8_t printIndent = 3;

                int colorPairIndex = 3;
                //printing background
                for(int j = 0; j < 3; j++){
                  int initYPos = j+1;
                  mvwprintw(resultsWindows[currentWordIndex], initYPos, initXPos, "       ");
                }
                mvwaddch(resultsWindows[currentWordIndex], 2, printIndent+initXPos, writer.textBox[i]);
                wrefresh(resultsWindows[currentWordIndex]);

                //one fifth of a second
                usleep(100000/5);
              }
            }

          }
          else if(chChecker >= 65 && chChecker <= 122){
            drawCharacterOnTextBox(resultsWindows[currentWordIndex], writer.cursor_x, writer.cursor_y, chChecker);
            writer.addCharacterToTextBox(chChecker);
            drawAllWindowLetterBoxes();
          }

          if(debugger.shouldDebug){
            wclear(debugWin);
            debugger.debugOut(debugWin);
            debug::debugFinalWord(debugWin, &writer);

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
            mvwprintw(debugWin, 3, 0, std::to_string(writer.setLetters).c_str());
            writer.wprintTextBox(debugWin);
          }
          else{wclear(debugWin); wrefresh(debugWin);}
        }
      }

      if(writerState == 1){
        bool hasFinished = true;
        for(int i = 0; i < 5; i++){
          if(writerAns[i] != 2){hasFinished = false;}
          int initXPos = 1 + 9*i;
          int printIndex = currentWordIndex-1;

          uint8_t printIndent = 3;

          int colorPairIndex = writerAns[i];
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
        writerState = 0;
        if(hasFinished){break;}
      }
    }
    if(shouldContinue){
      usleep(100000*3);
      wclear(debugWin);
      wclear(stdscr);

      wrefresh(debugWin);
      wrefresh(stdscr);
    }
    return shouldContinue;
  }
};


int main(){
  Game *game;
  bool shouldContinueGame = true;
  Game::gameSetup();

  while(shouldContinueGame){
    game = new Game("./ValidWordsdictionary/dicts/", "offline");
    game->gameInit();
    shouldContinueGame = game->gameLoop();

    delete game;
  }

  endwin();
  return 0;
}
