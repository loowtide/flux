#include "../include/tokenizer.hpp"
#include<vector>
#include<string>
#include<iostream>


Tokenizer::Tokenizer()
{
    loadStopWords();
}

void Tokenizer::loadStopWords()
{
    stopWords={"the", "is", "at", "which", "on", "to",
                 "a",   "an", "of", "or",    "in", "for"};
}

void Tokenizer::makeToken(std::vector<Token>&tokens, std::string &word,uint32_t &currentPos, const uint32_t &pageNo)
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

void Tokenizer::tokenizeLine(fz_stext_line* line,std::vector<Token>&tokens,uint32_t &currentPos,const uint32_t& pageNo){
    std::string currWord;
    for(fz_stext_char *ch=line->first_char;ch;ch=ch->next)
    {
        char c=ch->c;
        if(c>=0 && c<=127 && std::isalnum(c)){
            currWord+=std::tolower(c);
        }
        else {
            makeToken(tokens, currWord, currentPos,pageNo);
        }
    }
    makeToken(tokens,currWord, currentPos, pageNo);
}

void Tokenizer::tokenizeStextPage(fz_stext_page* stext_page,std::vector<Token>&tokens,uint32_t &currentPos,const uint32_t &pageNo){
    for(fz_stext_block* block=stext_page->first_block;block;block=block->next)
    {
        if(block->type!=FZ_STEXT_BLOCK_TEXT)continue;
        for(fz_stext_line* line=block->u.t.first_line;line;line=line->next)
            tokenizeLine(line,tokens,currentPos,pageNo);
    }
}

void Tokenizer::tokenizePage(fz_context* ctx,fz_document *doc,int pageIndex,std::vector<Token>&tokens,uint32_t &currentPos){
    fz_page* page=nullptr;
    fz_stext_page* stext_page=nullptr;
    fz_try(ctx)
    {
        page=fz_load_page(ctx, doc, pageIndex);
        stext_page=fz_new_stext_page_from_page(ctx, page,nullptr);
        tokenizeStextPage(stext_page, tokens,currentPos,pageIndex+1);
    }
    fz_always(ctx){
        fz_drop_stext_page(ctx,stext_page);
        fz_drop_page(ctx, page);
    }
    fz_catch(ctx){
        fz_rethrow(ctx);
    }
}

std::vector<Token> Tokenizer::tokenize(const std::string &file_path){
    std::vector<Token>tokens;
    fz_context *ctx=fz_new_context(nullptr,nullptr,FZ_STORE_DEFAULT);
    if(!ctx)
    {
        fprintf(stderr,"Failed to create a new file context");
        return tokens;
    }
    fz_document* doc=nullptr;
    uint32_t currentPos=0;
    fz_try(ctx)
    {
        fz_register_document_handlers(ctx);
        doc=fz_open_document(ctx, file_path.c_str());
        int pageCount=fz_count_pages(ctx,doc);
        for(int page=0;page<pageCount;page++){
            tokenizePage(ctx, doc, page, tokens, currentPos);
        }
    }
    fz_always(ctx)
    {
        fz_drop_document(ctx,doc);
    }
    fz_catch(ctx){
        std::cerr<<"PDF extraction failed"<<std::endl;
    }
    fz_drop_context(ctx);
    return tokens;
}
