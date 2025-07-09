#include "./writer.h"

void addVectorIntoVector(std::vector<int> vectorIn, std::vector<int>* vectorOut){
  for(int i = 0; i < vectorIn.size(); i++){
    vectorOut->emplace_back(vectorIn[i]);
  }
}

bool wordleWriter::checkWordIntegrity(std::string word){
  if(word.length()==5){return true;}
  return false;
}

wordleWriter::wordleWriter(std::string mode, std::string fWord){
  if(mode == "online"){
    //make fetching words work online
  }

  if(mode == "offline"){
    if(fWord != "" && checkWordIntegrity(fWord)){
      finalWordString = fWord;
      initializeFinalWord(fWord);
    }
    else{
      //fetch word from dictionary, but since i havent made it just throw err
      finalWordString = "error";
      initializeFinalWord(finalWordString);
    }
  }
}

void wordleWriter::addCharacterToTextBox(int character){
  if(character < 97 || character > 122){return;}
  textBox[cursor_x] = (char)character;
  if(cursor_x < 4){cursor_x++;}
  if(setLetters < 5){setLetters++;};
}

void wordleWriter::removeCharacterFromTextBox(){
  if(cursor_x > 0 && !textBox[cursor_x]){cursor_x--;}
  textBox[cursor_x] = 0;
  if(setLetters > 0){setLetters--;}
}

void wordleWriter::initializeFinalWord(std::string word){
  finalWord.clear();
  for(int i = 0; i < 5; i++){
    //add letter position to finalWord
    finalWord[(char)word[i]].emplace_back(i);
  }
}

void wordleWriter::wprintTextBox(WINDOW* win){
  wmove(win, 0, 10);
  waddstr(win, textBox.c_str());
  wrefresh(win);
}

bool wordleWriter::testInputValidity(){
  if(this->textBox.length() < 4){
    return 1;
  }
  return 0;
}

std::vector<std::vector<int>> wordleWriter::checkWordInput(WINDOW* win){
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
    int incorrectColorPairIndex = 3;
    int correctColorPairIndex = 2;
    int semiCorrectColorPairIndex = 1;
    std::vector<int> accessibleAnswer = (std::vector<int>){3, 3, 3, 3, 3};

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
                accessibleAnswer[inputLetterVector[i]] = correctColorPairIndex;
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
                  accessibleAnswer[inputLetterVector[i]] = semiCorrectColorPairIndex;

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

void wordleWriter::clearTextBox(){
  //bullshittery due to memory being ass
  this->textBox.clear();
  this->textBox.resize(0);
  this->textBox = (*new std::string(""));
  this->cursor_x = 0;
  this->setLetters = 0;
}
