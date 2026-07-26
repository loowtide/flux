#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include "../include/indexer.hpp"
#include "../include/tokenizer.hpp"


namespace py = pybind11;

PYBIND11_MODULE(flux, m){
    m.doc()="FLux bindings";

    py::class_<Token>(m,"Token")
        .def(py::init<>())
        .def_readwrite("position",&Token::position)
        .def_readwrite("pageNo",&Token::pageNo)
        .def_readwrite("text",&Token::text)
        .def_readwrite("is_stop_word",&Token::isStopWord);

    py::class_<Posting>(m,"Posting")
        .def(py::init<>())
        .def_readwrite("position",&Posting::position)
        .def_readwrite("pageNo",&Posting::pageNo);

    py::class_<DocScore>(m,"DocScore")
        .def(py::init<>())
        .def_readwrite("docId",&DocScore::docId)
        .def_readwrite("score",&DocScore::score);

    py::class_<PhraseHit>(m,"PhraseHit")
        .def(py::init<>())
        .def_readwrite("docId",&PhraseHit::docId)
        .def_readwrite("position",&PhraseHit::position)
        .def_readwrite("pageNo",&PhraseHit::pageNo);

    py::class_<Tokenizer>(m,"Tokenizer")
        .def(py::init<>())
        .def("tokenize",&Tokenizer::tokenize,py::arg("text"));

    py::class_<Indexer>(m,"Indexer")
        .def(py::init<>())
        .def("add_document",&Indexer::addDocument,py::arg("docId"),py::arg("tokens"))
        .def("remove_document",&Indexer::removeDocument,py::arg("docId"))
        .def("add_doc_path",&Indexer::addDocPath,py::arg("docId"),py::arg("path"))
        .def("add_posting",&Indexer::addPosting,py::arg("word"),py::arg("docId"),py::arg("posting"))
        .def("get_doc_count",&Indexer::getDocCount)
        .def("phrase_search",&Indexer::phraseSearch,py::arg("phrase"))
        .def("ranked_phrase_search",&Indexer::rankedPhraseSearch,py::arg("phrase"))
        .def("and_search",&Indexer::andSearch,py::arg("phrase"))
        .def("ranked_and_search",&Indexer::rankedAndSearch,py::arg("phrase"))
        .def("or_search",&Indexer::orSearch,py::arg("phrase"))
        .def("ranked_or_search",&Indexer::rankedOrSearch,py::arg("phrase"));
}
