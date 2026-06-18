#include "../include/indexer.hpp"
#include<map>

void Indexer::addDocument(uint32_t docId,const std::vector<Token>&tokens){
    for(const auto& token:tokens){
       auto& postings=index[token.text][docId];
      postings.push_back({token.position,token.pageNo});
    }
}
 const std::map<uint32_t,std::vector<Posting>>* Indexer::getPosting(const std::string &word) const {
     auto it=index.find(word);
     if(it!=index.end()){
         return &it->second;
     }
     return nullptr;
 }
