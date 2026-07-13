#include <cstdint>
#include<gtest/gtest.h>
#include "../include/indexer.hpp"

class TestIndexer:public ::testing::Test{
  protected:
    static Indexer indexer;
    static std::vector<Token>tokens;
    static void SetUpTestSuite(){
        tokens={
          {"quick",1,1,false},{"brown",2,1,false},{"fox",3,1,false}
        };
    }
};

Indexer TestIndexer::indexer;
std::vector<Token>TestIndexer::tokens;

TEST_F(TestIndexer, AddDocument) {
    indexer.addDocument(1, tokens);

    auto posting = indexer.getPosting("quick");

    ASSERT_NE(posting, nullptr);
    EXPECT_EQ(posting->begin()->first, 1u);
}

TEST_F(TestIndexer, GetPostingForExistingTerm) {
    indexer.addDocument(1, tokens);

    auto posting = indexer.getPosting("quick");

    ASSERT_NE(posting, nullptr);
    EXPECT_NE(posting->find(1), posting->end());
}

TEST_F(TestIndexer, GetPostingForNotExisting){
    indexer.addDocument(1,tokens);
    auto it=indexer.getPosting("notfound");
    EXPECT_EQ(it, nullptr);
}

//  -------------------Phrase Search--------------------------------------

TEST_F(TestIndexer,SearchExistingPhrase){
    indexer.addDocument(1,tokens);
    auto it=indexer.phraseSearch({"quick","brown","fox"});
    ASSERT_NE(it.size(), 0u);
    EXPECT_EQ(it[0].docId,1u);
    EXPECT_EQ(it[0].position,1u);
}

TEST_F(TestIndexer,SearchPhraseNotConsecutive){
    std::vector<Token>gapTokens={
        {"quick",1,1,false},
        {"brown",4,1,false},
        {"fox",5,1,false}
    };
    indexer.addDocument(1,gapTokens);
    auto it=indexer.phraseSearch({"quick","brown","fox"});
    EXPECT_TRUE(it.empty());
}

TEST_F(TestIndexer,SearchNonExistingPhrase){
    indexer.addDocument(1,tokens);
    auto it=indexer.phraseSearch({"quick","green"});
    ASSERT_EQ(it.size(), 0u);
}

TEST_F(TestIndexer,SearchEmptyPhrase){
    indexer.addDocument(1,tokens);
    auto it=indexer.phraseSearch({});
    EXPECT_TRUE(it.empty());
}

TEST_F(TestIndexer,PhraseSearchCarriesPageNo){
    std::vector<Token>pageToken={
        {"quick",1,2,false},
        {"brown",2,2,false},
        {"fox",3,2,false}
    };
    indexer.addDocument(1,pageToken);
    auto it=indexer.phraseSearch({"quick","brown","fox"});
    ASSERT_NE(it.size(), 0u);
    EXPECT_EQ(it[0].pageNo,2u);
}

//---------------------And Search-----------------------------------


TEST_F(TestIndexer,AndSearchPhraseEmpty){
    indexer.addDocument(1,tokens);
    auto it=indexer.andSearch({});
    EXPECT_TRUE(it.empty());
}

TEST_F(TestIndexer,AndSearchPhraseExisting){
    indexer.addDocument(1,tokens);
    auto it=indexer.andSearch({"quick","brown"});
    ASSERT_NE(it.size(), 0);
    EXPECT_EQ(it[0],1u);
}

TEST_F(TestIndexer,AndSearchPhraseNotExisting){
    indexer.addDocument(1,tokens);
    auto it=indexer.andSearch({"quick","green"});
    ASSERT_EQ(it.size(), 0u);
}

TEST_F(TestIndexer,AndSearchIntersectionAcrossDocuments){
    indexer.addDocument(1,tokens);
    std::vector<Token>doc2={
        {"quick",1,1,false},
        {"cat",2,1,false}
    };
    indexer.addDocument(2,doc2);
    auto it=indexer.andSearch({"quick","fox"});
    ASSERT_EQ(it.size(), 1u);
    EXPECT_EQ(it[0],1u);
    auto it2=indexer.andSearch({"quick"});
    ASSERT_EQ(it2.size(), 2u);
}

TEST_F(TestIndexer,AndSearchAllStopWords){
    std::vector<Token>stopOnly={
        {"the",1,1,true},
        {"a",1,2,true}
    };
    indexer.addDocument(1,stopOnly);
    auto it=indexer.andSearch({"the","a"});
    EXPECT_TRUE(it.empty());
}

TEST_F(TestIndexer,AndSearchIgnoresStopWords){
    std::vector<Token>stopMix={
        {"the",1,1,true},
        {"quick",1,2,true}
    };
    indexer.addDocument(1,stopMix);
    auto it=indexer.andSearch({"the","quick"});
    ASSERT_EQ(it.size(),1u);
    EXPECT_EQ(it[0],1u);
}

//-------------------orSearch-------------------------

TEST_F(TestIndexer,orSearchExistingPhrase){
    std::vector<Token>token={
        {"the",1,1,true},
        {"quick",1,2,true}
    };
    indexer.addDocument(1,token);
    auto it=indexer.orSearch({"quick","the"});
    ASSERT_EQ(it.size(),1u);
}

TEST_F(TestIndexer,orSearchExistingPhraseMultipleDocuments){
    std::vector<Token>token={
        {"the",1,1,true},
        {"quick",1,2,true}
    };
    indexer.addDocument(1,token);
    std::vector<Token>token2={
        {
            "fox",3,1,false
        }
    };
    indexer.addDocument(5,token2);
    auto it=indexer.orSearch({"fox","quick"});
    ASSERT_EQ(it.size(),2u);
    EXPECT_EQ(it[1],5u);
}

TEST_F(TestIndexer,orSearchEmptyPhrase){
    std::vector<Token>token={
        {"the",1,1,true},
        {"quick",1,2,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.orSearch({""});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,scoreOfExistingTerm){
    std::vector<Token>token={
        {"fox",1,1,false},
        {"quick",1,2,false}
    };
    indexer.addDocument(1,token);
    std::vector<Token>token2={
        {"fox",1,1,false},
        {"quick",1,2,false},
        {"quick",2,1,false},
        {"fox",3,1,false}
    };
    indexer.addDocument(2,token2);
    auto it=indexer.rankedOrSearch({"quick"});
    ASSERT_FALSE(it.empty());
    ASSERT_EQ(it.size(),2u);

    std::unordered_set<uint32_t>resultIds;
    for(const auto &ds:it){
        resultIds.insert(ds.docId);
    }
    ASSERT_TRUE(resultIds.count(1));
    ASSERT_TRUE(resultIds.count(2));

    auto score=[&](uint32_t id){
        for(const auto& ds:it){
            if(ds.docId==id) return ds.score;
        }
        return -1.0;
    };
    ASSERT_GE(score(2),score(1));
    ASSERT_NEAR(score(1), 0.693147, 1e-4);
    ASSERT_NEAR(score(2), 1.386294, 1e-4);
}

TEST_F(TestIndexer,scoreOfNonExistingTerm){
    std::vector<Token>token={
        {"fox",1,1,false},
        {"quick",1,2,false}
    };
    indexer.addDocument(1,token);
    std::vector<Token>token2={
        {"fox",1,1,false},
        {"quick",1,2,false},
        {"quick",2,1,false},
        {"fox",3,1,false}
    };
    indexer.addDocument(2,token2);
    auto it=indexer.rankedOrSearch({"quic"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedOrSearchStopWords){
    std::vector<Token>token={
        {"fox",1,1,false},
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedOrSearch({"fox","the"});
    auto it2=indexer.rankedOrSearch({"fox"});
    ASSERT_EQ(it.size(),it2.size());
    ASSERT_EQ(it.size(),1u);
    ASSERT_NEAR(it[0].score,it2[0].score,1e-9);
}

TEST_F(TestIndexer,rankedAndSearchBasic){
    std::vector<Token>token1={
        {"fox",1,1,false},
        {"quick",2,1,false},
    };
    indexer.addDocument(1,token1);
    std::vector<Token>token2={
        {"quick",1,1,false},
        {"quick",2,1,false},
        {"fox",3,1,false},
    };
    std::vector<Token>token3={
      {"quick",1,1,false},
    };
    indexer.addDocument(2,token2);
    indexer.addDocument(3,token3);
    auto it=indexer.rankedAndSearch({"quick","fox"});;
    ASSERT_EQ(it.size(),2u);

    std::unordered_set<uint32_t>resultIds;
    for(const auto& ds:it){
        resultIds.insert(ds.docId);
    }
    ASSERT_TRUE(resultIds.count(1));
    ASSERT_TRUE(resultIds.count(2));
    ASSERT_FALSE(resultIds.count(3));

    auto score=[&](uint32_t id){
        for(const auto &ds:it){
            if(ds.docId==id) return ds.score;
        }
        return -1.0;
    };
    ASSERT_NEAR(score(1),1.540445,1e-4);
    ASSERT_NEAR(score(2),2.233592,1e-4);
}


TEST_F(TestIndexer,rankedAndSearchMissingTermReturnsEmpty){
    std::vector<Token>token={
        {"quick",1,1,false},
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedAndSearch({"quick","fox"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedAndSearchEmptyQuery){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedAndSearch({});
    ASSERT_TRUE(it.empty());
}
TEST_F(TestIndexer,rankedAndSearchOnlyStopWords){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.andSearch({"the","a","is"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedAndSearchNoIntersection){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    std::vector<Token>token2={
      {"killa",1,2,false}
    };
    auto it=indexer.rankedAndSearch({"quick","killa"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedPhraseSearchExact){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"brown",2,1,false},
        {"fox",3,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedPhraseSearch({"quick","brown","fox"});
    ASSERT_EQ(it.size(), 1u);
    ASSERT_EQ(it[0].docId,1u);
    ASSERT_NEAR(it[0].score,2.079442,1e-4);
}

TEST_F(TestIndexer,rankedPhraseSearchNonAdjacent){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"brown",2,2,false},
        {"fox",3,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedPhraseSearch({"quick","brown","fox"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedAndSearchMultipleDocs){
    std::vector<Token>token1={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token1);
    std::vector<Token>token2={
        {"quick",1,1,false},
        {"fox",2,1,false},
        {"quick",3,1,false},
        {"fox",4,1,false},
    };
    indexer.addDocument(2,token2);
    auto it=indexer.rankedPhraseSearch({"quick","fox"});;
    ASSERT_EQ(it.size(),2u);

    auto score=[&](uint32_t id){
        for(const auto &ds:it){
            if(ds.docId==id) return ds.score;
        }
        return -1.0;
    };
    ASSERT_NEAR(score(1),1.386294,1e-4);
    ASSERT_NEAR(score(2),2.772589,1e-4);
    ASSERT_GE(score(2),score(1));
}

TEST_F(TestIndexer,rankedPhraseSearchEmptyQuery){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedPhraseSearch({});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedPhraseSearchMissingTermReturnsEmpty){
    std::vector<Token>token={
        {"quick",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    auto it=indexer.rankedPhraseSearch({"quick","brown","fox"});
    ASSERT_TRUE(it.empty());
}

TEST_F(TestIndexer,rankedPhraseSearchSingleWords){
    std::vector<Token>token={
        {"fox",2,1,false}
    };
    indexer.addDocument(1,token);
    std::vector<Token>token2={
        {"fox",1,1,false},
        {"fox",2,1,false}
    };
    indexer.addDocument(2,token2);
    auto it=indexer.rankedPhraseSearch({"fox"});
    ASSERT_EQ(it.size(),2u);

    auto score=[&](uint32_t id){
        for(const auto &ds:it){
            if(ds.docId==id) return ds.score;
        }
        return -1.0;
    };

    ASSERT_NEAR(score(1),0.693147 , 1e-5);
    ASSERT_NEAR(score(2),1.386294 , 1e-5);
}
