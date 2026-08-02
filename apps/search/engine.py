import sys
import threading
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent.parent.parent / "build"
if str(BUILD_DIR) not in sys.path:
    sys.path.insert(0, str(BUILD_DIR))

import flux

_indexer = {}
_locks = {}
_lock = threading.Lock()


def _get_session_lock(session_key: str) -> threading.Lock:
    with _lock:
        if session_key not in _locks:
            _locks[session_key] = threading.Lock()
        return _locks[session_key]


def get_indexer(session_key: str) -> "flux.Indexer":
    with _lock:
        if session_key not in _indexer:
            _indexer[session_key] = flux.Indexer()
        return _indexer[session_key]


def add_document(session_key: str, doc_id: int, tokens, file_path: str) -> None:
    index = get_indexer(session_key)
    lock = _get_session_lock(session_key)
    with lock:
        index.add_document(doc_id, tokens)
        index.add_doc_path(doc_id, file_path)


def remove_document(session_key: str, doc_id: int) -> None:
    index = get_indexer(session_key)
    lock = _get_session_lock(session_key)
    with lock:
        index.remove_document(doc_id)


def phrase_search(session_key: str, query: str):
    index = get_indexer(session_key)
    terms = query.lower().split()
    if not terms:
        return []
    return index.ranked_phrase_search(terms)


def and_search(session_key: str, query: str):
    index = get_indexer(session_key)
    terms = query.lower().split()
    if not terms:
        return []
    return index.ranked_and_search(terms)


def or_search(session_key: str, query: str):
    index = get_indexer(session_key)
    terms = query.lower().split()
    if not terms:
        return []
    return index.ranked_or_search(terms)
