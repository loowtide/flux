#ifndef INDEXER_HPP
#define INDEXER_HPP

#include "tokenizer.hpp"
#include <cstdint>
#include<map>
#include <unordered_map>

struct Posting{
    uint32_t position;
    uint32_t  pageNo;
};

struct PhraseHit{
    uint32_t docId;
    uint32_t position;
    uint32_t pageNo;
};

class Indexer{
    public:
        /*
         * add tokens from document to master index
         */
        void addDocument(uint32_t docId,const std::vector<Token>&tokens);

        /*
         * return map of {docId->{Posting}}
         */
        const std::map<uint32_t,std::vector<Posting>>* getPosting(const std::string &word) const;

        void addPosting(const std::string &word,uint32_t docId,const Posting &p){
            auto &postings=index[word][docId];
            postings.push_back(p);
        }

        // full phrase search
        std::vector<PhraseHit>phraseSearch(const std::vector<std::string>&phrase) const;

        // and search -> should contain all words(neglects stop words)
        std::vector <uint32_t>andSearch(const std::vector<std::string>&phrase) const;

        // fallback or search -> give files with at least one word
        std::vector<uint32_t>orSearch(const std::vector<std::string>&phrase);

        uint32_t getDocCount() const{
            return documentsPaths.size();
        }

        void addDocPath(uint32_t id,std::string &path){
            documentsPaths[id]=path;
        }

    private:
        //master index
        std::unordered_map<std::string ,std::map<uint32_t,std::vector<Posting>>>index;

        std::unordered_map<uint32_t ,std::string>documentsPaths;
        std::vector<const std::map<uint32_t,std::vector<Posting>>*>getList(const std::vector<std::string> &phrase) const;
};

#endif
