from django.conf import settings
from django.conf.urls.static import static
from django.urls import path

from .views import DocumentDeleteView, DocumentUploadView, IndexView, SearchView

app_name = "search"

urlpatterns = [
    path("", IndexView.as_view(), name="index"),
    path("upload/", DocumentUploadView.as_view(), name="upload"),
    path("delete/<int:doc_id>/", DocumentDeleteView.as_view(), name="delete"),
    path("search/", SearchView.as_view(), name="search"),
]

if settings.DEBUG:
    urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
