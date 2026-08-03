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

struct DocScore{
    uint32_t docId;
    double score;
};

class Indexer{
    friend class Persist;
    public:
        /*
         * add tokens from document to master index
         */
        void addDocument(uint32_t docId,const std::vector<Token>&tokens);

        void removeDocument(uint32_t docId);

        /*
         * return map of {docId->{Posting}}
         */
        const std::map<uint32_t,std::vector<Posting>>* getPosting(const std::string &word) const;

        void addPosting(const std::string &word,uint32_t docId,const Posting &p){
            auto &postings=index[word][docId];
            postings.push_back(p);
        }
        void addDocPath(uint32_t id,const std::string &path){
            documentsPaths[id]=path;
        }

        uint32_t getDocCount() const{
            return seenIds.size();
        }
        // full phrase search
        std::vector<PhraseHit>phraseSearch(const std::vector<std::string>&phrase) const;

        // and search -> should contain all words(neglects stop words)
        std::vector <uint32_t>andSearch(const std::vector<std::string>&phrase) const;

        // fallback or search -> give files with at least one word
        std::vector<uint32_t>orSearch(const std::vector<std::string>&phrase)const;

        /*
         * Ranked Searches
         */

        std::vector<DocScore>rankedPhraseSearch(const std::vector<std::string>&phrase) const;

        std::vector<DocScore>rankedAndSearch(const std::vector<std::string>&phrase) const;

        std::vector<DocScore>rankedOrSearch(const std::vector<std::string>&phrase) const;

    private:
        //master index
        std::unordered_map<std::string ,std::map<uint32_t,std::vector<Posting>>>index;

        std::unordered_map<uint32_t ,std::string>documentsPaths;
        std::vector<const std::map<uint32_t,std::vector<Posting>>*>getList(const std::vector<std::string> &phrase) const;

        std::unordered_set<uint32_t>seenIds;

        //This will calculate the TF-IDF score.
        // Iterates the candidate set and look for each docId in each term's Posting
        // reducing search space (iterate only the set not the corpus)
        std::vector<DocScore>scoreAndSort(const std::vector<uint32_t>&docIds,const std::vector<const std::map<uint32_t,std::vector<Posting>>*>&lists) const;

        std::vector<std::string>filterStopWords(const std::vector<std::string>&phrase) const;

        std::unordered_map<uint32_t,std::unordered_set<std::string>>docWords;

};

#endif
