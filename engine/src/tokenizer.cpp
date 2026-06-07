#include "../include/tokenizer.hpp"
#include<vector>
#include<string>


Tokenizer::Tokenizer()
{
    loadStopWords();
}

void Tokenizer::loadStopWords()
{
    stopWords={"the", "is", "at", "which", "on", "to",
                 "a",   "an", "of", "or",    "in", "for"};
}

void Tokenizer::makeToken(std::vector<Token>&tokens, std::string &word,uint32_t &currentPos,const uint32_t &pageNo)
{
    if(word.empty()) return;
    Token token;
    token.text=word;
    token.isStopWord=(stopWords.count(word))>0;
    token.pageNo=pageNo;
    token.position=currentPos++;
    tokens.push_back(token);
    word.clear();
}
