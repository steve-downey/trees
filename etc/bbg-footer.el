;;; bbg-footer.el --- Org-re-reveal per-deck footer injection  -*- lexical-binding: t; -*-

;;; Commentary:
;;
;; Injects a footer bar into every reveal.js slide via
;; `org-re-reveal-postamble'.  The visible text is controlled by the
;; buffer-local variable `bbg-footer-text', which should be set per
;; deck in the org file's Local Variables block:
;;
;;   # Local Variables:
;;   # bbg-footer-text: "© 2026 Steve Downey. CppNow 2026."
;;   # End:
;;
;; The HTML/JS injection mechanism is handled here; only the text
;; content moves into each deck.

;;; Code:

(defvar bbg-footer-text ""
  "Footer text injected into each reveal.js slide.
Override per deck via the org file's Local Variables block.")

(defun bbg-footer--build-postamble ()
  "Return postamble HTML, interpolating the current `bbg-footer-text'."
  (format
   "<style type=\"text/css\">
    #header-left  { position: absolute; top: 0%%; left: 0%%; }
    #header-right { position: absolute; top: 0%%; right: 0%%; }
    #footer-left  {
        position: absolute;
        bottom: 0%%;
        left: 0%%;
        font-size: 0.3em;
        height: 50px;
        width: 300px;
    }
    #footer-left img { width: 100%%; height: 100%%; object-fit: contain; }
</style>

<div id=\"hidden\" style=\"display:none;\">
<div id=\"header\">
<div id=\"footer-left\">%s</div>
</div>
</div>

<script src=\"https://code.jquery.com/jquery-2.2.4.min.js\"></script>
<script type=\"text/javascript\">
var header = $('#header').html();
if ( window.location.search.match( /print-pdf/gi ) ) {
    Reveal.addEventListener( 'ready', function( event ) {
        $('.slide-background').append(header);
    });
} else {
    $('div.reveal').append(header);
}
</script>"
   bbg-footer-text))

(defun bbg-footer--apply (&rest _)
  "Set `org-re-reveal-postamble' from the current `bbg-footer-text'.
Called via `org-export-before-processing-hook', after local variables
in the visiting buffer are already in effect."
  (setq-local org-re-reveal-postamble (bbg-footer--build-postamble)))

(add-hook 'org-export-before-processing-hook #'bbg-footer--apply)

(provide 'bbg-footer)

;;; bbg-footer.el ends here
