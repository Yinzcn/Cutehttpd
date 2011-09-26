@echo off
set n=10000
set c=1000
if not #%1==# set n=%1
if not #%2==# set c=%2
@echo on
ab -n %n% -c %c% http://127.0.0.1:8080/%3
