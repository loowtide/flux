#include<gtest/gtest.h>
#include "../include/tokenizer.hpp"
#include<algorithm>

class TestTokenizer:public ::testing::Test
{
    protected:
        static Tokenizer tokenizer;
        static std::vector<Token>tokens;
        static void SetUpTestSuite(){
            tokens=tokenizer.tokenize(std::string(TEST_DATA_DIR)+"/test.pdf");
        }
};
Tokenizer TestTokenizer::tokenizer;
std::vector<Token>TestTokenizer::tokens;

TEST_F(TestTokenizer,ProducesToken){
    EXPECT_FALSE(tokens.empty());
}

TEST_F(TestTokenizer, ContainsQuick){
    auto it=std::find_if(tokens.begin(),tokens.end(),[](const Token &t){
        return t.text=="quick";
    });
    EXPECT_NE(it, tokens.end());
}
TEST_F(TestTokenizer,IsTheStopWord){
    auto it=std::find_if(tokens.begin(),tokens.end(),[](const Token &t){
        return t.text=="the";
    });
    ASSERT_NE(it, tokens.end());
    EXPECT_TRUE(it->isStopWord);
}
