#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <ncurses.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <utility>
#include <unistd.h>
#include <fstream>
#define CORRECT_GREEN 9
#define INCORRECT_RED 10

void addVectorIntoVector(std::vector<int> vectorIn, std::vector<int>* vectorOut){
  for(int i = 0; i < vectorIn.size(); i++){
    vectorOut->emplace_back(vectorIn[i]);
  }
}

class wordleWriter{
  public:
  uint8_t cursor_x = 0;
  uint8_t cursor_y;

  std::string textBox;
  //character and positions
  std::map<char, std::vector<int>> finalWord;
  std::string finalWordString;

  std::ifstream letterIn;
  char currentLetter = 'a';

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

  void addCharacterToTextBox(int character){
    if(character < 97 || character > 122){return;}
    textBox[cursor_x] = (char)character;
    if(cursor_x < 4){cursor_x++;}
  }

  void removeCharacterFromTextBox(){
    if(cursor_x > 0 && !textBox[cursor_x]){cursor_x--;}
    textBox[cursor_x] = 0;
  }

  void initializeFinalWord(std::string word){
    finalWord.clear();
    for(int i = 0; i < 5; i++){
      //add letter position to finalWord
      finalWord[(char)word[i]].emplace_back(i);
    }
  }

  void wprintTextBox(WINDOW* win){
    wmove(win, 0, 10);
    waddstr(win, textBox.c_str());
    wrefresh(win);
  }

  bool testInputValidity(){
    if(this->textBox.length() < 4){
      return 1;
    }
    return 0;
  }

  std::vector<std::vector<int>> checkWordInput(WINDOW* win = stdscr){
    if(testInputValidity()){
      //generate textBox word map
      std::map<char, std::vector<int>> inputWordMap;
      for(int i = 0; i < 5; i++){
        //add letter position to finalWord
        inputWordMap[(char)textBox[i]].emplace_back(i);
      }

      //for debugging purposes
      std::vector<int> correctLetters;
      std::vector<int> semiCorrectLetters;
      std::vector<int> incorrectLetters;

      //actually used in game code
      std::vector<int> accessibleAnswer = (std::vector<int>){0, 0, 0, 0, 0};

      for(std::map<char, std::vector<int>>::iterator it = inputWordMap.begin(); it != inputWordMap.end(); ++it){
        //if the current letter exists within the final word
        if(this->finalWord.contains(it->first)){
          std::vector<int> inputLetterVector(it->second);
          std::vector<int> finalWordLetterVector(finalWord[it->first]);

          //variables for semi correct
          int letterCount = finalWordLetterVector.size();
          int addLetters = 0;

          //check if any of the positions in the input word map match the ones in the output word map
          // checks any of the correct letters
            for(int i = 0; i < inputLetterVector.size(); i++){

              for(int j = 0; j < finalWordLetterVector.size(); j++){
                if(finalWordLetterVector[j] == inputLetterVector[i]){
                  correctLetters.emplace_back(inputLetterVector[i]);
                  accessibleAnswer[inputLetterVector[i]] = 2;
                  addLetters += 1;
                }
              }

              //checks the semi correct letters in the last iteration:
              // if there is an i instance of an x letter that have not been deemed correct
              // in the final word, and there is also a j instance x letter in the input word that have no classification
              // that same j instance of x letter is deemed semi correct
              //
              // note: this code is a mess, ill fix it when I fix it, what matters is that if it works it works
              if(i == inputLetterVector.size()-1 && (letterCount - addLetters) > 0 && inputLetterVector.size() > addLetters){
                for(int i = 0 ; i < inputLetterVector.size(); i++){
                  if(accessibleAnswer[inputLetterVector[i]] != 2){
                    addLetters += 1;
                    semiCorrectLetters.emplace_back(inputLetterVector[i]);
                    accessibleAnswer[inputLetterVector[i]] = 1;

                    if((letterCount - addLetters) <= 0){break;}
                  }
                }
              }

            }

          }
        else{
          addVectorIntoVector(it->second, &incorrectLetters);
        }
      }
      return (std::vector<std::vector<int>>){correctLetters, semiCorrectLetters, incorrectLetters, accessibleAnswer};
    }

    //error case
    return (std::vector<std::vector<int>>){};
  }

  void clearTextBox(){
    //bullshittery due to memory being ass
    this->textBox.clear();
    this->textBox.resize(0);
    this->textBox = (*new std::string(""));
    this->cursor_x = 0;
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

  std::vector<WINDOW*> resultsWindows;
  int currentWordIndex = 0;
  int row, col;
  getmaxyx(stdscr, row, col);
  WINDOW* debugWin = newwin(4, col, 0, 0);

  //0 is for play, 1 is for result scr
  int gameState = 0;
  std::string pastResults[5];

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
    bool shouldDebug = true;
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
          gameState = 1;
          ans = game.checkWordInput(debugWin);
          //gameAns = accessibleAnswer
          gameAns = ans[3];
          //gameAns = (std::vector<int>){2, 1, 0, 2, 2};
          if(currentWordIndex != 6){currentWordIndex++;}
          pastResults[currentWordIndex] = game.textBox;
          game.clearTextBox();
        }
        else if(chChecker >= 65 && chChecker <= 122){
          drawCharacterOnTextBox(resultsWindows[currentWordIndex], game.cursor_x, game.cursor_y, chChecker);
          game.addCharacterToTextBox(chChecker);
          drawWindowLetterBoxes(resultsWindows[currentWordIndex], 0);
        }

        if(shouldDebug){
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
          mvwprintw(debugWin, 3, 0, std::to_string(game.cursor_x).c_str());
          game.wprintTextBox(debugWin);
        }

      }
    }

    if(gameState == 1){
      for(int i = 0; i < 5; i++){
        int initXPos = 1 + 9*i;
        int printIndex = currentWordIndex-1;

        int colorPairIndex = (gameAns[i] != 0) ? gameAns[i] : 3;
        for(int j = 0; j < 3; j++){
          int initYPos = j+1;
          wattron(resultsWindows[printIndex], COLOR_PAIR(colorPairIndex));
          mvwprintw(resultsWindows[printIndex], initYPos, initXPos, "       ");
        }
        mvwaddch(resultsWindows[currentWordIndex-1], 2, 3+initXPos, pastResults[currentWordIndex][i]);
        wattroff(resultsWindows[printIndex], COLOR_PAIR(colorPairIndex));
        wrefresh(resultsWindows[printIndex]);

        //half of a second
        usleep(100000*5);
      }
      gameState = 0;
    }
  }
  endwin();
  return 0;
}
