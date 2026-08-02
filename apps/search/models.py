from django.db import models


def document_upload_path(instance, filename) -> str:
    return f"documents/{instance.session_key}/{filename}"


class Document(models.Model):
    session_key = models.CharField(max_length=42, db_index=True)
    doc_id = models.PositiveIntegerField()
    file_name = models.CharField(max_length=200)
    file_size = models.PositiveIntegerField(help_text="Size in bytes")
    file = models.FileField(upload_to=document_upload_path)

    uploaded_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ["-uploaded_at"]
        constraints = [
            models.UniqueConstraint(fields=["session_key", "doc_id"], name="unique_key")
        ]
