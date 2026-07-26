#ifndef PERSIST_HPP
#define PERSIST_HPP

#include<string>
#include "indexer.hpp"


class Persist{
    public:
    static bool loadIndex(Indexer &indexer,const std::string &path);
    static bool saveIndex(const Indexer &indexer,const std::string &path);
};

#endif
