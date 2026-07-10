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
    EXPECT_EQ(it.size(),1u);
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
        {"quick",1,2,true}
    };
    indexer.addDocument(1,token);
    auto it=indexer.orSearch({""});
    ASSERT_TRUE(it.empty());
}
