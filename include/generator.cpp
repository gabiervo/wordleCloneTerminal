#include "generator.h"
#include <fstream>
wordGenerator::wordGenerator(std::string directory){
  //initialize alphabet and directory from which we get the word dictionary
  this->dictDirectory = directory;
}

std::string wordGenerator::generateWord(){
  //initialize the rng seed based on the current time
  srand(time(0));
  char letter = alphabet[rand() % 26];
  int lineIndex;

  std::fstream wordDict(dictDirectory + letter + ".txt", std::ifstream::in);
  if(wordDict.good()){
    std::string target;
    int lineCount = 0;

    while(std::getline(wordDict, target)){
      lineCount++;
    }

    lineIndex = rand() % lineCount;
    int currLine = 0;
    wordDict.clear();
    wordDict.seekg(0);

    while(std::getline(wordDict, target)){
      if(currLine == lineIndex){break;}
      currLine++;
    }

    wordDict.close();
    return target;
  }
  else{
    return "errar";
  }
}

bool wordGenerator::checkWordExistence(std::string word){
  std::fstream wordFile(dictDirectory + word[0] + ".txt", std::ifstream::in);
  std::string target;
  while(std::getline(wordFile, target)){
    bool isWordMatch = true;
    for(int i = 0; i < 5; i++){
      if(target[i] != word[i]){
        isWordMatch = false;
        break;
      }
    }
    if(isWordMatch){return true;}
  }
  return false;
}
