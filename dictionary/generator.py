with open("./words_alpha.txt", 'r') as txt:
    writeFile = open('./dicts/a.txt', "w")
    currentLetter = 'a'
    for line in txt:
        word = line.strip()
        if word.__len__() != 5:
            continue
        if word[0] != currentLetter:
            print(word[0])
            currentLetter = word[0]
            writeFile = open("./dicts/" + currentLetter + ".txt", "a")
        writeFile.write(word + "\n")
