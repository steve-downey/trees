(require 'org-id)

(defun surround-region-with-uuids ()
  "Surround the active region with hard-coded strings"
  (interactive)
  (let* ((uuid (org-id-uuid))
         (name (buffer-name))
         (beginning (region-beginning))
         (end (region-end))
         (region-content (buffer-substring-no-properties beginning end))
         (prefixed-content (concat "\n// " uuid "\n" region-content "// " uuid " end\n"))
         (transclude (concat "#+transclude: [[file:" name "::" uuid "]] :lines 2- :src cpp :end \"" uuid " end")))
    (save-excursion
      (delete-region beginning end)
      (goto-char (region-beginning))
      (insert prefixed-content))
    (kill-new transclude)))
