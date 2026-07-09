#include<gtest/gtest.h>
#include "../include/indexer.hpp"

class TestIndexer:public ::testing::Test{
  protected:
    static Indexer indexer;
    static Tokenizer tokenizer;
    static std::vector<Token>tokens;
    static void SetUpTestSuite(){
        tokens=tokenizer.tokenize(std::string(TEST_DATA_DIR)+"/test.pdf");
    }
};

Indexer TestIndexer::indexer;
Tokenizer TestIndexer::tokenizer;
std::vector<Token>TestIndexer::tokens;

TEST_F(TestIndexer, AddDocument) {
    indexer.addDocument(1, tokens);

    auto posting = indexer.getPosting("quick");

    ASSERT_NE(posting, nullptr);
    EXPECT_EQ(posting->begin()->first, 1);
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

TEST_F(TestIndexer,SearchExistingPhrase){
    indexer.addDocument(1,tokens);
    auto it=indexer.phraseSearch({"quick","brown"});
    ASSERT_NE(it.size(), 0);
    EXPECT_EQ(it[0].docId,1);
    EXPECT_EQ(it[0].position,1);
}

TEST_F(TestIndexer,SearchNonExistingPhrase){
    indexer.addDocument(1,tokens);
    auto it=indexer.phraseSearch({"quick","green"});
    ASSERT_EQ(it.size(), 0);
}

TEST_F(TestIndexer,AndSearchPhraseExisting){
    indexer.addDocument(1,tokens);
    auto it=indexer.andSearch({"quick","lazy"});
    ASSERT_NE(it.size(), 0);
    EXPECT_EQ(it[0],1);
}

TEST_F(TestIndexer,AndSearchPhraseNotExisting){
    indexer.addDocument(1,tokens);
    auto it=indexer.andSearch({"quick","green"});
    ASSERT_EQ(it.size(), 0);
}
