/*
-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

                                                    __
                                                  `    `.
                                                 /      |
                                                        | 
                                                 `     .
                                     _            `.  . 
                ........---       .`   `.      ..-`   `..._
                |          |     (      |     `            `
                .          |      )     /    |              |
                |  .-`w`   |     <       --..              / 
                | ` MMML.  |      \ /                     /
                |  . MMML  .       ` /                   /
                ._..  MMM``.--..  .-`                   /
                    `  MMI`-```--`                     ( 
                     . . \.       __                    \
                      `    ---````  L....................
                U B E R  L E E T  H A C K E R  F O R C E
 
           This is official Uber Leet Hacker Force source code
	    This is free software, read LICENSE for details
 
 http://pgp.mit.edu:11371/pks/lookup?op=get&search=0x34964E67B47CF3DB
*/ 
#define DELICIOUS_COOKIES \
  "GET /thumbnails.php HTTP/1.1\r\n" \
  "Host: www.unixporn.ml\r\n" \
  "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
  "Accept-Language: en-us,en:q=0.5\r\n" \
  "Accept-Encoding: deflate\r\n" \
  "Accept-Charset: ISO-8859-1,utf-8:q=0.7,*;q=0.7\r\n" \
  "Keep-Alive: 300\r\n" \
  "Connection: keep-alive\r\n" \
  "\r\n"

#define LOGIN_A \
 "GET /login.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"\
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Connection: keep-alive\r\n" \
 "\r\n"


#define LOGIN \
  "POST /login.php" \
  " HTTP/1.1\r\n" \
  "Host: www.unixporn.ml\r\n" \
  "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
  "Accept-Language: en-us,en;q=0.5\r\n" \
  "Accept-Encoding: deflate\r\n" \
  "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
  "Keep-Alive: 300\r\n" \
  "Connection: keep-alive\r\n" \
  "Referer: http://www.unixporn.ml/login.php?referer=thumbnails.php?%%3Falbum%%3Dlastup%%26cat%%3D0\r\n" \

#define LOGOUT \
  "GET /logout.php?referer=thumbnails.php%%3Dlastup%%26cat%%3D0 HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \
 "Referer: http://www.unixporn.ml/thumbnails.php?album=lastup&cat=0\r\n"

#define UPLOAD_TOKEN \
 "POST /upload.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \
 "Referer: http://www.unixporn.ml/upload.php\r\n"


#define UPLOAD_A \
 "POST /db_input.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \
 "Referer: http://www.unixporn.ml/upload.php\r\n"

#define UPLOAD_B \
 "Content-Disposition: form-data; name=\"file_upload_array[]\"; filename=\"%s\"\r\n" \
 "Content-Type: image/jpeg\r\n\r\n"

#define UPLOAD_NOFILE \
 "Content-Disposition: form-data; name=\"file_upload_array[]\"; filename=\"\"\r\n" \
 "Content-Type: application/octet-stream\r\n" \
 "\r\n\r\n"

#define UPLOAD_URI \
 "Content-Disposition: form-data; name=\"URI_array[]\"\r\n" \
 "\r\n\r\n" 

#define UPLOAD_PHASE \
 "Content-Disposition: form-data; name=\"control\"\r\n" \
 "\r\n" \
 "phase_1\r\n"

#define CONTINUE \
 "POST /upload.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \
 "Referer: http://www.unixporn.ml/upload.php\r\n"

#define ALBUM \
 "POST /upload.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \
 "Referer: http://www.unixporn.ml/upload.php\r\n"

#define ALBUM_B \
 "GET /albmgr.php HTTP/1.1\r\n" \
 "Host: www.unixporn.ml\r\n" \
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
 "Accept-Language: en-us,en;q=0.5\r\n" \
 "Accept-Encoding: deflate\r\n" \
 "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
 "Keep-Alive: 300\r\n" \
 "Connection: keep-alive\r\n" \

/*
  Thank you for your attention
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.10 (OpenBSD)

iQEcBAEBAgAGBQJLvx04AAoJEDC1QXa8JnD676MH/10rMoYdE8xp2S1xzSSGX8Ws
rspor9zxDZSuZ7NT1NEPfHYfXrv8GBDlPvzmjgRJp677nrdrpYynwV/8VtiJVYK7
+tJgGwJpaoW714TUtyMf1UlteCCPFTttzIDXdlzVqvxk6nxBLElTJfSbdqPBVfZH
AqpfMOTAxlYV/SU4U7yJRXhzT5btelo+mMJXCKnmLNOidPSC95HVz5YZ5CaB4CtU
RTC1DX/XoYWqVVrNz1D/mqrpzaJK4lbfSUgtMNq5q9p6qwGnV8MEuIDMhcG7pfnp
h6uMBXFT6hL8B34KDWRjvkQEsDxXcZZNTMfzzyTlvKYw38PdU71t34+/jNPp5i8=
=aq8n
-----END PGP SIGNATURE-----
*/
