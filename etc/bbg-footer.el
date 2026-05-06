;;; bbg-footer.el --- Org-re-reveal per-deck footer injection  -*- lexical-binding: t; -*-

;;; Commentary:
;;
;; Injects header/footer bars into every reveal.js slide via
;; `org-re-reveal-postamble'.  Three variables control content and are
;; set per deck via the org file's Local Variables block:
;;
;;   # Local Variables:
;;   # bbg-footer-text:       "Steve Downey — My Talk — Conference 2026"
;;   # bbg-footer-left-logo:  "etc/assets/TechAtBloomberg_black.png"
;;   # bbg-footer-right-logo: "etc/assets/BBGEngineering_black.png"
;;   # End:
;;
;; Left side (bottom-left): optional logo image stacked above the text.
;; Right side (bottom-right): optional logo image.
;; The HTML/JS injection mechanism stays here; only content moves per deck.

;;; Code:

(defvar bbg-footer-text ""
  "Footer text shown bottom-left beneath the left logo.
Set in the org file's Local Variables block.")

(defvar bbg-footer-left-logo ""
  "Path to the image shown bottom-left above `bbg-footer-text'.
Relative to the exported HTML file.  Empty string omits the image.")

(defvar bbg-footer-right-logo ""
  "Path to the image shown bottom-right.
Relative to the exported HTML file.  Empty string omits the image.")

(defun bbg-footer--img (src alt height)
  "Return an <img> tag for SRC at HEIGHT px, or empty string if SRC is blank."
  (if (string-blank-p src)
      ""
    (format "<img src=\"%s\" alt=\"%s\" style=\"height:%dpx;width:auto;display:block;\">"
            src alt height)))

(defun bbg-footer--build-postamble ()
  "Return the postamble HTML string from the current footer variables."
  (let ((left-logo  (bbg-footer--img bbg-footer-left-logo  "" 35))
        (right-logo (bbg-footer--img bbg-footer-right-logo "" 45)))
    (format
     "<style type=\"text/css\">
    #header-left  { position: absolute; top: 0%%; left: 0%%; }
    #header-right { position: absolute; top: 0%%; right: 0%%; }
    #footer-left {
        position: absolute;
        bottom: 0%%;
        left: 0%%;
        padding: 4px;
        display: flex;
        flex-direction: column;
        align-items: flex-start;
        gap: 3px;
    }
    #footer-left-text {
        font-size: 0.3em;
        line-height: 1;
        white-space: nowrap;
    }
    #footer-right {
        position: absolute;
        bottom: 0%%;
        right: 0%%;
        padding: 4px;
        display: flex;
        align-items: flex-end;
    }
</style>

<div id=\"hidden\" style=\"display:none;\">
<div id=\"header\">
<div id=\"footer-left\">%s<div id=\"footer-left-text\">%s</div></div>
<div id=\"footer-right\">%s</div>
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
     left-logo
     bbg-footer-text
     right-logo)))

(defun bbg-footer--apply (&rest _)
  "Set `org-re-reveal-postamble' from the current footer variables.
Called via `org-export-before-processing-hook', after local variables
in the visiting buffer are already in effect."
  (setq-local org-re-reveal-postamble (bbg-footer--build-postamble)))

(add-hook 'org-export-before-processing-hook #'bbg-footer--apply)

(provide 'bbg-footer)

;;; bbg-footer.el ends here
