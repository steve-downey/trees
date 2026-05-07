;;; set-footer.el --- Set the footers for slides  -*- lexical-binding: t; -*-

;;; Commentary:
;;


;;; Code:
(require 'bbg-footer)

(setq bbg-footer-text "© 2026 Bloomberg Finance L.P. All rights reserved.")
(setq bbg-footer-left-logo "etc/assets/TechAtBloomberg_black.png")
(setq bbg-footer-right-logo "etc/assets/BBGEngineering_black.png")
(setq org-re-reveal-postamble "<style type=\"text/css\">
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
<div id=\"footer-left\"><img src='etc/assets/TechAtBloomberg_black.png' alt='' style='height:35px;width:auto;display:block;'><div id=\"footer-left-text\">© 2026 Bloomberg Finance L.P. All rights reserved.</div></div>
<div id=\"footer-right\"><img src='etc/assets/BBGEngineering_black.png' alt='' style='height:45px;width:auto;display:block;'></div>
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
</script>")

(format org-re-reveal-postamble)q

(provide 'set-footer)

;;; set-footer.el ends here
