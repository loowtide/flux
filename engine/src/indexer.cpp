#include "../include/indexer.hpp"
#include <cstdint>
#include<unordered_set>
#include<set>
#include<algorithm>
#include<cmath>

void Indexer::addDocument(uint32_t docId,const std::vector<Token>&tokens){
    seenIds.insert(docId);
    std::unordered_set<std::string>touchedWords;
    touchedWords.reserve(tokens.size());

    for(const auto& token:tokens){
       auto& postings=index[token.text][docId];
      postings.push_back({token.position,token.pageNo});
      touchedWords.insert(token.text);
    }
    //robust check (sort)
   for (const auto& word: touchedWords){
        auto& postings=index[word][docId];
        std::sort(postings.begin(),postings.end(),[](const Posting&a ,const Posting&b){
            return a.position<b.position;
        });
    }
    auto &tracked=docWords[docId];
    tracked.insert(touchedWords.begin(),touchedWords.end());
}

void Indexer::removeDocument(uint32_t docId){
    auto wordsIt=docWords.find(docId);
    if(wordsIt==docWords.end()) return;
    for(const auto& word:docWords[docId]){
        auto indexIt=index.find(word);
        if(indexIt==index.end()) continue;
        auto &docMap=indexIt->second;
        docMap.erase(docId);
        if(docMap.empty()) index.erase(indexIt);
    }
    docWords.erase(wordsIt);
    documentsPaths.erase(docId);
    seenIds.erase(docId);
}


const std::map<uint32_t,std::vector<Posting>>* Indexer::getPosting(const std::string &word) const {
     auto it=index.find(word);
     if(it!=index.end()){
         return &it->second;
     }
     return nullptr;
 }

 //----------------------Helper Functions----------------------------------------------

std::vector<const std::map<uint32_t,std::vector<Posting>>*>Indexer::getList(const std::vector<std::string>&phrase)const {
        std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists;
        lists.reserve(phrase.size());
        for(const auto &word:phrase){
            lists.push_back(getPosting(word));
        }
        return lists;
}

 //utility for andSearch to find the doc that intersects with all the word postings

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


 std::vector<std::string>Indexer::filterStopWords(const std::vector<std::string>&phrase)const{
     static const std::unordered_set<std::string>stopWords={
         "the", "is", "at", "which", "on", "to","a",   "an", "of", "or",    "in", "for"};
     std::vector<std::string>filtered;
     filtered.reserve(phrase.size());
     for(const auto &word:phrase){
         if(!stopWords.count(word)){
             filtered.push_back(word);
         }
     }
     return filtered;
 }

 //--------------------------Search Functions----------------------------------

 std::vector<PhraseHit>Indexer::phraseSearch(const std::vector<std::string>&phrase) const{
     std::vector<PhraseHit>hits;
     if(phrase.empty())
         return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(phrase);

     for(const auto* l:lists){
         if(!l) return {}; //word missing
     }

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
                 if(found==posList.end() or found->position!=target or found->pageNo!=pos.pageNo){
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



 std::vector<uint32_t>Indexer::andSearch(const std::vector<std::string>&phrase) const {
     if(phrase.empty()) return {};
     std::vector<std::string>filtered=filterStopWords(phrase); //filter phrase for stop words
     if(filtered.empty()) return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(filtered);

     for(const auto* l:lists){
         if(!l) return {}; //word missing
     }

     std::vector<std::vector<uint32_t>>docIdList;
     docIdList.reserve(lists.size());
     for(const auto *p:lists){
         if(!p or p->empty()) return {};
         std::vector<uint32_t>ids;
         ids.reserve(p->size());
         for(const auto& [docId,_]:*p)ids.push_back(docId);
         docIdList.push_back(std::move(ids));
     }
     return intersectAll(docIdList);
 }

 std::vector<uint32_t>Indexer::orSearch(const std::vector<std::string>&phrase)const{
     if(phrase.empty()) return {};
     std::vector<std::string>filtered=filterStopWords(phrase); //filter phrase for stop words
     if(filtered.empty()) return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(filtered);
     std::set<uint32_t>docIdList;
     for(const auto* p:lists){
         if(!p) continue;
         for(const auto& [docId,_]:*p){
             docIdList.insert(docId);
         }
     }
     return std::vector<uint32_t>(docIdList.begin(),docIdList.end());
 }

//-------------------------- Ranker utility functions -----------------------

 std::vector<DocScore>Indexer::scoreAndSort(const std::vector<uint32_t>&docIds,const std::vector<const std::map<uint32_t,std::vector<Posting>>*>&lists) const {
     if(docIds.empty() or lists.empty()) return {};
     const double N=static_cast<double>(getDocCount());
     // Compute IDF
     std::vector<double>idfs;
     idfs.reserve(lists.size());
     for(const auto* postings:lists){
         if(!postings or postings->empty()) {
             idfs.push_back(0.0);
             continue;
         }
         const double df=static_cast<double>(postings->size());
         idfs.push_back(std::log((N+1)/(df+1)+1.0));
     }
     std::vector<DocScore>results;
     results.reserve(docIds.size());
     for(uint32_t docId:docIds){
         double score=0.0;
         for(size_t i=0;i<lists.size();i++){
             const auto* postings=lists[i];
             if(!postings) continue;
             if(!postings->count(docId)) continue;
             const double tf=static_cast<double>(postings->at(docId).size());
             score+=tf*idfs[i];
         }
         if(score>0.0) results.push_back({docId,score});
     }
     std::sort(results.begin(),results.end(),[](const auto &a,const auto &b){
         return a.docId<b.docId;
     });
     return results;
 }

 //-------------------------- Ranked Search ----------------------------------

 std::vector<DocScore>Indexer::rankedOrSearch(const std::vector<std::string>&phrase) const{
     if(phrase.empty()) return {};
     std::vector<std::string>filtered=filterStopWords(phrase);
     if(filtered.empty()) return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(filtered);
     for(const auto* l:lists){
         if(!l) return {}; //word missing
     }

     std::unordered_set<uint32_t>docIdSet;
     for(const auto *p:lists){
         if(!p) continue;
         for(const auto &[docId,_]:*p ){
             docIdSet.insert(docId);
         }
     }
     std::vector<uint32_t>docIds(docIdSet.begin(),docIdSet.end());
     return scoreAndSort(docIds,lists);
 }


 std::vector<DocScore>Indexer::rankedAndSearch(const std::vector<std::string>&phrase) const {
     if(phrase.empty()) return {};
     std::vector<std::string>filtered=filterStopWords(phrase);
     if(filtered.empty()) return {};
     std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(filtered);
     for(const auto* l:lists){
         if(!l) return {}; //word missing
     }

     std::vector<std::vector<uint32_t>>docIdList;
     docIdList.reserve(lists.size());
     for(const auto *p:lists){
         if(!p or p->empty()) return {};
         std::vector<uint32_t>ids;
         ids.reserve(p->size());
         for(const auto &[docId,_]:*p){
             ids.push_back(docId);
         }
         docIdList.push_back(std::move(ids));
     }
         std::vector<uint32_t>candidates=intersectAll(docIdList);
         return scoreAndSort(candidates,lists);
}

std::vector<DocScore>Indexer::rankedPhraseSearch(const std::vector<std::string>&phrase) const {
    if(phrase.empty()) return {};
    std::vector<PhraseHit>hits=phraseSearch(phrase);
    std::unordered_set<uint32_t>docIdSet;
    for(const auto &hit:hits){
        docIdSet.insert(hit.docId);
    }
    std::vector<const std::map<uint32_t,std::vector<Posting>>*>lists=getList(phrase);
    std::vector<uint32_t>docIds(docIdSet.begin(),docIdSet.end());
    return scoreAndSort(docIds,lists);
}
