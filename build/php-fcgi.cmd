
set PHP_FCGI_MAX_REQUESTS=100000

pushd php
php-cgi -b 127.0.0.1:9900
popd
