# 🔍 Flux

> A fast inverted-index search engine with native PDF tokenization — phrase and boolean queries out of the box.

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)

![CMake](https://img.shields.io/badge/CMake-3.15%2B-064F8C?logo=cmake&logoColor=white)

![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?logo=python&logoColor=white)

![Django](https://img.shields.io/badge/Django-backend-092E20?logo=django&logoColor=white)

![Tests](https://img.shields.io/badge/tests-GoogleTest-brightgreen)

![License](https://img.shields.io/badge/license-MIT-lightgrey)

Flux indexes PDF documents term-by-term with positional data, supporting search queries. A Django backend exposes the engine over HTTP.

---

## Features

- **Positional inverted index** — term → docId → posting list.
- **Phrase search** — consecutive-offset matching.
- **Boolean AND** — documents containing every term, via size-ordered intersection.
- **PDF tokenization** — page-by-page extraction (MuPDF) with stop-word flagging.
- **Django backend** — query the engine over the web.

## Architecture

At its core, Flux maintains a positional inverted index:

```
unordered_map<string, map<uint32_t, vector<Posting>>>
term       ->   docId  -> [{position, pageNo}, ...]
```

Each term maps to the documents it appears in, and within each document to the exact positions where it occurs.

## Core types

**`Posting`** — a single occurrence of a term in a document.

```cpp
struct Posting { uint32_t position; uint32_t pageNo; };
```

**`PhraseHit`** — a successful phrase match, tied to its document.

```cpp
struct PhraseHit { uint32_t docId; uint32_t position; uint32_t pageNo; };
```

**`Token`** — the tokenizer's output for one term.

```cpp
struct Token { uint32_t position; bool isStopWord; string text; uint32_t pageNo; };
```

## API

| Method                       | Description                    |
| ---------------------------- | ------------------------------ |
| `addDocument(docId, tokens)` | Index a document's tokens      |
| `getPosting(word)`           | Get postings for a term        |
| `phraseSearch(terms)`        | Match consecutive terms        |
| `andSearch(terms)`           | Find docs containing all terms |

## Query examples

```cpp
// Phrase: "quick brown" as consecutive tokens
auto hits = idx.phraseSearch({"quick", "brown"});   // vector<PhraseHit>

// Boolean AND: docs containing both terms, anywhere
auto docs = idx.andSearch({"quick", "lazy"});        // vector<uint32_t>
```

## Build & test

```bash
# Configure & build
cmake -B build -S .
cmake --build build

# Run the full test suite
ctest --test-dir build --output-on-failure

# Run individual suites
./build/indexer_test
./build/tokenizer_test
```

Test fixture: `engine/tests/test.pdf`, shared by both suites.

## Requirements

**C++17** · **CMake ≥ 3.15** · **Python ≥ 3.10**

| Library        | Purpose                       |
| -------------- | ----------------------------- |
| **MuPDF**      | PDF parsing & text extraction |
| **GoogleTest** | C++ unit testing              |
| **Django**     | Web backend (Python)          |

## Project layout

```
CMakeLists.txt              Top-level CMake config
engine/
├── include/
│   ├── indexer.hpp         Posting, PhraseHit, Indexer
│   └── tokenizer.hpp       Token, Tokenizer (MuPDF-based)
├── src/
│   ├── indexer.cpp         addDocument, phraseSearch, andSearch
│   └── tokenizer.cpp       page-by-page tokenization
└── tests/
    ├── indexer_test.cpp    Add, GetPosting, PhraseSearch, AndSearch
    ├── tokenizer_test.cpp  Tokenization, stop-word tests
    ├── test.pdf            Sample PDF fixture
    └── CMakeLists.txt      Per-target GTest linking

config/                     Django configuration
apps/                       Django applications
manage.py                   Django management script
pyproject.toml              Python project metadata
```
