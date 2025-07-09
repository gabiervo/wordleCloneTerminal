#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <ncurses.h>
class wordleWriter{
  public:
  uint8_t cursor_x = 0;
  uint8_t cursor_y;
  uint8_t setLetters = 0;

  std::string textBox;

  std::map<char, std::vector<int>> finalWord;
  std::string finalWordString;

  wordleWriter(std::string mode, std::string fWord="");
  void addCharacterToTextBox(int character);
  static bool checkWordIntegrity(std::string word);
  void removeCharacterFromTextBox();
  void initializeFinalWord(std::string word);
  void wprintTextBox(WINDOW* win);
  bool testInputValidity();
  std::vector<std::vector<int>> checkWordInput(WINDOW* win = stdscr);
  void clearTextBox();
};
