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
    textBox[cursor_x] = 0;
    if(cursor_x > 0){cursor_x--;}
  }

  void initializeFinalWord(std::string word){
    finalWord.clear();
    for(int i = 0; i < 5; i++){
      //add letter position to finalWord
      finalWord[(char)word[i]].emplace_back(i);
    }
  }

  void wprintTextBox(WINDOW* win){
    wclear(win);
    waddstr(win, textBox.c_str());
    wrefresh(win);
  }

  bool testInputValidity(){
    if(this->textBox.length() < 4){
      return 1;
    }
    return 0;
  }

  std::vector<std::vector<int>> checkWordInput(){
    if(testInputValidity()){
      //generate textBox word map
      std::map<char, std::vector<int>> inputWordMap;
      for(int i = 0; i < 5; i++){
        //add letter position to finalWord
        inputWordMap[(char)textBox[i]].emplace_back(i);
      }

      int i = 0;
      std::vector<int> correctLetters;
      std::vector<int> semiCorrectLetters;
      std::vector<int> incorrectLetters;
      for(std::map<char, std::vector<int>>::iterator it = inputWordMap.begin(); it != inputWordMap.end(); ++it){
        //if letter exists in map
        if(this->finalWord.contains(it->first)){
        //check if it has the same position or an incorrect position

        //check if any of the positions in the input word map match the ones in the output word map
          for(int i = 0; i < it->second.size(); i++){
            bool isCorrect = false;
            for(int j = 0; j < finalWord[it->first].size(); j++){
              if(finalWord[it->first][j] == it->second[i]){
                correctLetters.emplace_back(it->second[i]);
                isCorrect = true;
              }
            }
            //if(!isCorrect){semiCorrectLetters.emplace_back(it->second[i]);}
          }
        }
        else{addVectorIntoVector(it->second, &incorrectLetters);}
        i++;
      }
      return (std::vector<std::vector<int>>){correctLetters, semiCorrectLetters, incorrectLetters};
    }

    //error case
    return (std::vector<std::vector<int>>){};
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
  int gameWinHeight = 5;
  int gameWinWidth = 10;

  int gameWinYPos = 5;
  int gameWinXPos = 10;
  WINDOW* gameWin = newwin(gameWinHeight, gameWinWidth, gameWinYPos, gameWinXPos);

  std::vector<std::vector<int>> ans;

  while(true){
    if(poll(&poller, 1, 100) == 1){
      chChecker = wgetch(stdscr);
      debugger.debugIn(stdscr, chChecker);
      //esc
      if(chChecker == 27){break;}

      //delete
      if(chChecker == 263){game.removeCharacterFromTextBox();}

      //enter
      else if(chChecker == 10){
        ans = game.checkWordInput();
      }
      else{game.addCharacterToTextBox(chChecker);}
    }
    wclear(stdscr);
    debugger.debugOut(stdscr);
    debug::debugFinalWord(stdscr, &game);

    for(int i = 0; i < ans.size(); i++){
      for(int j = 0; j < ans[i].size(); j++){
        mvwprintw(stdscr, 20+i, j, std::to_string(ans[i][j]).c_str());
      }
    }
    mvwprintw(stdscr, 10, 0, std::to_string(game.cursor_x).c_str());
    wrefresh(stdscr);
    game.wprintTextBox(gameWin);
    usleep(1600);
  }
  endwin();
  return 0;
}
