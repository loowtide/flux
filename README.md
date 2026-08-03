# 🔍 Flux

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-3.23%2B-064F8C?logo=cmake&logoColor=white) ![Python](https://img.shields.io/badge/Python-3.13%2B-3776AB?logo=python&logoColor=white) ![Django](https://img.shields.io/badge/Django-6.0%2B-092E20?logo=django&logoColor=white) ![Tests](https://img.shields.io/badge/tests-GoogleTest-brightgreen) ![License](https://img.shields.io/badge/license-MIT-lightgrey)

> A fast inverted-index search engine with native PDF tokenization — phrase, boolean, and ranked queries out of the box.

## Features

- Positional inverted index (term → docId → posting list)
- Phrase search, boolean AND/OR, TF-IDF ranked search
- PDF tokenization (MuPDF) with stop-word filtering
- pybind11 Python bindings (`flux` module)
- Django backend with PDF.js viewer

## Types

```cpp
struct Token    { std::string text; uint32_t position, pageNo; bool isStopWord; };
struct Posting  { uint32_t position, pageNo; };
struct PhraseHit{ uint32_t docId, position, pageNo; };
struct DocScore { uint32_t docId; double score; };
```

## API

| Method                       | Description                      |
| ---------------------------- | -------------------------------- |
| `addDocument(docId, tokens)` | Index a document's tokens        |
| `removeDocument(docId)`      | Remove a document from the index |
| `getPosting(word)`           | Get postings for a term          |
| `addDocPath(docId, path)`    | Store file path for a document   |
| `getDocCount()`              | Number of indexed documents      |
| `phraseSearch(terms)`        | Match consecutive terms          |
| `andSearch(terms)`           | Docs containing all terms        |
| `orSearch(terms)`            | Docs containing any term         |
| `rankedPhraseSearch(terms)`  | Phrase match with TF-IDF scoring |
| `rankedAndSearch(terms)`     | Boolean AND with TF-IDF scoring  |
| `rankedOrSearch(terms)`      | Boolean OR with TF-IDF scoring   |

## Build & test

```bash
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

## Requirements

**C++17** · **CMake ≥ 3.23** · **Python ≥ 3.13**

| Library          | Purpose             |
| ---------------- | ------------------- |
| **MuPDF**        | PDF parsing         |
| **pybind11**     | C++/Python bindings |
| **GoogleTest**   | C++ unit testing    |
| **Django ≥ 6.0** | Web backend         |

Python: `django >= 6.0.5`, `django-environ >= 0.14.0`, `pybind11 >= 3.0.4`

## Project layout

```
CMakeLists.txt
engine/
├── include/    indexer.hpp, tokenizer.hpp, persist.hpp
├── src/        indexer.cpp, tokenizer.cpp, bindings.cpp, persist.cpp
└── tests/      indexer_test.cpp, tokenizer_test.cpp, test.pdf
config/         Django settings
apps/search/    engine.py, models.py, views.py, urls.py
templates/      HTML templates
static/         CSS, PDF.js viewer
media/          Uploaded documents
.github/        CI workflow
```
