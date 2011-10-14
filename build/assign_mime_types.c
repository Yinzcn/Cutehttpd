
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


void
assign_mime_types(struct htdx_t *htdx)
{
    if (!htdx) {
        return;
    }
    mime_type_assign(htdx, ".pdf"     , "application/pdf");
    mime_type_assign(htdx, ".sig"     , "application/pgp-signature");
    mime_type_assign(htdx, ".spl"     , "application/futuresplash");
    mime_type_assign(htdx, ".class"   , "application/octet-stream");
    mime_type_assign(htdx, ".ps"      , "application/postscript");
    mime_type_assign(htdx, ".torrent" , "application/x-bittorrent");
    mime_type_assign(htdx, ".dvi"     , "application/x-dvi");
    mime_type_assign(htdx, ".gz"      , "application/x-gzip");
    mime_type_assign(htdx, ".pac"     , "application/x-ns-proxy-autoconfig");
    mime_type_assign(htdx, ".swf"     , "application/x-shockwave-flash");
    mime_type_assign(htdx, ".tar.gz"  , "application/x-tgz");
    mime_type_assign(htdx, ".tgz"     , "application/x-tgz");
    mime_type_assign(htdx, ".tar"     , "application/x-tar");
    mime_type_assign(htdx, ".zip"     , "application/zip");
    mime_type_assign(htdx, ".mp3"     , "audio/mpeg");
    mime_type_assign(htdx, ".m3u"     , "audio/x-mpegurl");
    mime_type_assign(htdx, ".wma"     , "audio/x-ms-wma");
    mime_type_assign(htdx, ".wax"     , "audio/x-ms-wax");
    mime_type_assign(htdx, ".ogg"     , "application/ogg");
    mime_type_assign(htdx, ".wav"     , "audio/x-wav");
    mime_type_assign(htdx, ".bmp"     , "image/bmp");
    mime_type_assign(htdx, ".gif"     , "image/gif");
    mime_type_assign(htdx, ".jpg"     , "image/jpeg");
    mime_type_assign(htdx, ".jpeg"    , "image/jpeg");
    mime_type_assign(htdx, ".png"     , "image/png");
    mime_type_assign(htdx, ".xbm"     , "image/x-xbitmap");
    mime_type_assign(htdx, ".xpm"     , "image/x-xpixmap");
    mime_type_assign(htdx, ".xwd"     , "image/x-xwindowdump");
    mime_type_assign(htdx, ".css"     , "text/css");
    mime_type_assign(htdx, ".html"    , "text/html");
    mime_type_assign(htdx, ".htm"     , "text/html");
    mime_type_assign(htdx, ".js"      , "text/javascript");
    mime_type_assign(htdx, ".asc"     , "text/plain");
    mime_type_assign(htdx, ".c"       , "text/plain");
    mime_type_assign(htdx, ".cpp"     , "text/plain");
    mime_type_assign(htdx, ".h"       , "text/plain");
    mime_type_assign(htdx, ".log"     , "text/plain");
    mime_type_assign(htdx, ".conf"    , "text/plain");
    mime_type_assign(htdx, ".text"    , "text/plain");
    mime_type_assign(htdx, ".txt"     , "text/plain");
    mime_type_assign(htdx, ".spec"    , "text/plain");
    mime_type_assign(htdx, ".dtd"     , "text/xml");
    mime_type_assign(htdx, ".xml"     , "text/xml");
    mime_type_assign(htdx, ".mpeg"    , "video/mpeg");
    mime_type_assign(htdx, ".mpg"     , "video/mpeg");
    mime_type_assign(htdx, ".mov"     , "video/quicktime");
    mime_type_assign(htdx, ".qt"      , "video/quicktime");
    mime_type_assign(htdx, ".avi"     , "video/x-msvideo");
    mime_type_assign(htdx, ".asf"     , "video/x-ms-asf");
    mime_type_assign(htdx, ".asx"     , "video/x-ms-asf");
    mime_type_assign(htdx, ".wmv"     , "video/x-ms-wmv");
    mime_type_assign(htdx, ".bz2"     , "application/x-bzip");
    mime_type_assign(htdx, ".tbz"     , "application/x-bzip-compressed-tar");
    mime_type_assign(htdx, ".tar.bz2" , "application/x-bzip-compressed-tar");
    mime_type_assign(htdx, ".rpm"     , "application/x-rpm");
    mime_type_assign(htdx, "*"        , "application/octet-stream");
}
