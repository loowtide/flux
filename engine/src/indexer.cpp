#include "../include/indexer.hpp"
#include <algorithm>
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

 std::vector<PhraseHit>Indexer::phraseSearch(const std::vector<std::string>&phrase) const{
     std::vector<PhraseHit>hits;
     if(phrase.empty())
         return {};
     const auto *firstWord=getPosting(phrase[0]);
     auto byPosition=[](const Posting &p,size_t pos){
       return p.position<pos;
     };
     if(!firstWord) return hits;
     for(const auto &[docId,positions]: *firstWord){
         for(const auto &pos: positions){
             bool match=true;
             for(size_t i=1;i<phrase.size();i++){
                 const auto *nextPostings=getPosting(phrase[i]);
                 if(!nextPostings){
                     match=false;
                     break;
                 }
                 auto npos=nextPostings->find(docId);
                 if(npos==nextPostings->end()){
                     match=false;
                     break;
                 }
                 const auto &posList=npos->second;
                 size_t target=pos.position+1;
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

 std::vector<int> intersectAll(std::vector<std::vector<int>>&posts)
 {
     if(posts.empty()) return {};
     sort(posts.begin(),posts.end(),[](const auto&a ,const auto &b){return a.size()<b.size();});
     std::vector<int>intersect=std::move(posts[0]);
     std::vector<int>temp;
     for(size_t i=1;i<posts.size();i++)
     {
         temp.clear();
         std::set_intersection(intersect.begin(),intersect.end(),posts[i].begin(),posts[i].end(),std::back_inserter(temp));
         intersect=std::move(temp);
         if(intersect.empty()) break;
     }
     return intersect;
 }

 std::vector<uint32_t>Indexer::andSearch(const std::vector<std::string>&phrase) const {
     if(phrase.empty()) return {};
     const auto *first=getPosting(phrase[0]);
     if(!first || first->empty()) return {};
     std::vector<uint32_t>candidates;
     candidates.reserve(first->size());
     for (const auto& [docId, _] : *first) {
         candidates.push_back(docId);
     }

     for (size_t i = 1; i < phrase.size() && !candidates.empty(); ++i) {
         const auto* postings = getPosting(phrase[i]);
         if (!postings) return {};

         std::vector<uint32_t> next;
         for (int docId : candidates) {
             if (postings->count(docId)) {
                 next.push_back(docId);
             }
         }
         candidates = std::move(next);
     }

     return candidates;
 }
