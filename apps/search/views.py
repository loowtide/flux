from django.db import transaction
from django.http import HttpRequest, JsonResponse
from django.shortcuts import redirect, render
from django.views import View

from . import engine
from .engine import add_document, remove_document
from .models import Document


class IndexView(View):
    def get(self, request: HttpRequest, *args, **kwargs):
        if not request.session.session_key:
            request.session.create()
        session_key = request.session.session_key

        documents = Document.objects.filter(session_key=session_key)
        return render(request, "search/index.html", {"documents": documents})


class DocumentUploadView(View):
    def post(self, request: HttpRequest, *args, **kwargs):
        if not request.session.session_key:
            request.session.create()
        session_key = request.session.session_key
        print("Upload:", session_key)
        upload = request.FILES.getlist("file")
        if not upload:
            return JsonResponse({"error": "No file provided"}, status=400)

        tokeniser = engine.flux.Tokenizer()
        for f in upload:
            doc = self._create_document(session_key, f)
            tokens = tokeniser.tokenize(doc.file.path)
            add_document(session_key, doc.id, tokens, doc.file.name)

        return redirect("search:index")

    def _create_document(self, session_key, upload):
        with transaction.atomic():
            last = (
                Document.objects.select_for_update()
                .filter(session_key=session_key)
                .order_by("-doc_id")
                .first()
            )
            next_id = (last.doc_id + 1) if last else 1
            return Document.objects.create(
                session_key=session_key,
                doc_id=next_id,
                file=upload,
                file_name=upload.name,
                file_size=upload.size,
            )


class DocumentDeleteView(View):
    def post(self, request: HttpRequest, doc_id: int, *args, **kwargs):
        session_key = request.session.session_key
        if not session_key:
            return JsonResponse({"error": "No session"}, status=400)

        doc = Document.objects.filter(session_key=session_key, doc_id=doc_id).first()
        if not doc:
            return JsonResponse({"error": "Document Not found"}, status=404)

        remove_document(session_key, doc.id)
        doc.file.delete(save=False)
        doc.delete()

        return redirect("search:index")


class SearchView(View):
    MODES = {
        "phrase": engine.phrase_search,
        "and": engine.and_search,
        "or": engine.or_search,
    }

    def get(self, request: HttpRequest, *args, **kwargs):
        if not request.session.session_key:
            request.session.create()
        session_key = request.session.session_key
        print("Search", session_key)
        query = request.GET.get("q", "").strip()
        mode = request.GET.get("mode", "and")

        if mode not in self.MODES:
            mode = "and"

        results = []
        if session_key and query:
            search_fn = self.MODES[mode]
            scores = search_fn(session_key, query)

            score_map = {}
            doc_ids = []
            for s in scores:
                doc_id = s.docId
                score = getattr(s, "score", None)
                doc_ids.append(doc_id)
                if score is not None:
                    score_map[doc_id] = score

            docs = Document.objects.filter(id__in=doc_ids)
            if score_map:
                results = sorted(
                    docs, key=lambda d: score_map.get(d.id, 0), reverse=True
                )
            else:
                results = list(docs)
        return render(
            request,
            "search/results.html",
            {"query": query, "mode": mode, "results": results},
        )
