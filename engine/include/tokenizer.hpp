#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include<string>
#include<unordered_set>
#include<vector>

extern "C" {
#include <mupdf/fitz.h>
}

/*
 * Token Structure
 * defined as the text , postion, page no of the document and is it a stop word
 */

struct Token
{
    std::string text;
    uint32_t position;
   uint32_t pageNo;
    bool isStopWord;
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
    /*
     * make tokens from word string
     */
    void makeToken(std::vector<Token>&tokens,std::string &word,uint32_t &currentPos,const uint32_t &pageNo);

    /*
     * tokenize line extracted from mupdf line
     */
    void tokenizeLine(fz_stext_line* line,std::vector<Token>&tokens,uint32_t &currentPos,const uint32_t &pageNo);
    /*
    * tokenize stextpage from mupdf page
    */
    void tokenizeStextPage(fz_stext_page *stext_page,std::vector<Token>&tokens,uint32_t &currentPos,const uint32_t &pageNo);
    /*
     * tokenize page
     */
    void tokenizePage(fz_context *ctx,fz_document* doc,int pageIndex,std::vector<Token>&tokens,uint32_t &currentPos);
};

#endif
