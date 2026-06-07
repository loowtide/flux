#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include<string>
#include<unordered_set>
#include<vector>

#include<mupdf/fitz.h>

/*
 * Token Structure
 * defined as the text , postion, page no of the document and is it a stop word
 */

struct Token
{
    uint32_t position;
    bool isStopWord;
    std::string text;
   uint32_t pageNo;
};

class Tokenizer {
    public:
        /*
         * Function to implement tokenization logic
         */
        Tokenizer();
        std::vector<Token>tokenize(const std::string &file_path);

    private:
    std::unordered_set<std::string>stopWords;
    void loadStopWords();
    void makeToken(std::vector<Token>&tokens,std::string &word,uint32_t &currentPos,const uint32_t &pageNo);
};

#endif
