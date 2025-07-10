#ifndef GENERATOR_H
#define GENERATOR_H
#include <string>

class wordGenerator{
  public:
  const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
  std::string dictDirectory;

  wordGenerator(std::string directory);
  std::string generateWord();
  bool checkWordExistence(std::string word);
};

#endif
