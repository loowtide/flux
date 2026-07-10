#include "../include/indexer.hpp"
#include<algorithm>
#include <cstdint>
#include <string>
void Indexer::addDocument(uint32_t docId,const std::vector<Token>&tokens){
    for(const auto& token:tokens){
       auto& postings=index[token.text][docId];
      postings.push_back({token.position,token.pageNo});
    }
    //robust check (sort)
   /*  for (const auto& token: tokens){
        auto& postings=index[token.text][docId];
        std::sort(postings.begin(),postings.end(),[](const Posting&a ,const Posting&b){
            return a.position<b.position;
        });
    }
    */
}
 const std::map<uint32_t,std::vector<Posting>>* Indexer::getPosting(const std::string &word) const {
     auto it=index.find(word);
     if(it!=index.end()){
         return &it->second;
     }
     return nullptr;
 }

/*
 * Helper function for search queries.No need to call getPosting in search functions.
 */
std::vector<const std::map<uint32_t,std::vector<Posting>>*>Indexer::getList(const std::vector<std::string>&phrase)const {
        std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists;
        lists.reserve(phrase.size());
        for(const auto &word:phrase){
            const auto *p=getPosting(word);
            if(!p) continue;
            lists.push_back(p);
        }
        return lists;
}

 std::vector<PhraseHit>Indexer::phraseSearch(const std::vector<std::string>&phrase) const{
     std::vector<PhraseHit>hits;
     if(phrase.empty())
         return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(phrase);
     if(lists.size()!=phrase.size()) return {};
     const auto *firstWord=lists[0];
     auto byPosition=[](const Posting &p,size_t pos){
       return p.position<pos;
     };
     for(const auto &[docId,positions]: *firstWord){
         for(const auto &pos: positions){
             bool match=true;
             for(size_t i=1;i<phrase.size();i++){
                 const auto *nextPostings=lists[i];
                 auto npos=nextPostings->find(docId);
                 if(npos==nextPostings->end()){
                     match=false;
                     break;
                 }
                 const auto &posList=npos->second;
                 size_t target=pos.position+i;
                 auto found=std::lower_bound(posList.begin(),posList.end(),target,byPosition);
                 if(found==posList.end() or found->position!=target){
                     match=false;
                     break;
                 }
             }
             if(match){
                 hits.push_back({docId,pos.position,pos.pageNo});
             }
         }
     }
     return hits;
 }

 std::vector<uint32_t> intersectAll(std::vector<std::vector<uint32_t>>&posts)
 {
     if(posts.empty()) return {};
     std::sort(posts.begin(),posts.end(),[](const auto&a ,const auto &b){return a.size()<b.size();});
     std::vector<uint32_t>intersect=std::move(posts[0]);
     std::vector<uint32_t>temp;
     for(size_t i=1;i<posts.size();i++)
     {
         temp.clear();
         std::set_intersection(intersect.begin(),intersect.end(),posts[i].begin(),posts[i].end(),std::back_inserter(temp));
         intersect=std::move(temp);
         if(intersect.empty()) break;
     }
     return intersect;
 }

 bool isStopWord(const std::string &word){
     static const std::unordered_set<std::string>stopWords={
         "the", "is", "at", "which", "on", "to","a",   "an", "of", "or",    "in", "for"};
     return stopWords.count(word);
 }

 std::vector<uint32_t>Indexer::andSearch(const std::vector<std::string>&phrase) const {
     if(phrase.empty()) return {};
     std::vector<std::string>filtered; //filter phrase for stop words
     filtered.reserve(phrase.size());
     for(const auto& word:phrase){
         if(!isStopWord(word)){
             filtered.push_back(word);
         }
     }
     if(filtered.empty()) return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(filtered);
     if(lists.size()!=filtered.size()) return {};

     std::vector<std::vector<uint32_t>>docIdList;
     docIdList.reserve(lists.size());
     for(const auto *p:lists){
         if(p->empty()) return {};
         std::vector<uint32_t>ids;
         ids.reserve(p->size());
         for(const auto& [docId,_]:*p)ids.push_back(docId);
         docIdList.push_back(std::move(ids));
     }
     return intersectAll(docIdList);
 }
