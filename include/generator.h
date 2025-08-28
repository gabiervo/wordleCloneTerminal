#ifndef GENERATOR_H
#define GENERATOR_H
#include <string>

class wordGenerator{
  public:
  const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
  std::string validWordsDirectory;
  std::string answerWordsDirectory;

  wordGenerator(std::string validDirectory, std::string answerDirectory);
  std::string generateWord();
  bool checkWordExistence(std::string word);
};

#endif
