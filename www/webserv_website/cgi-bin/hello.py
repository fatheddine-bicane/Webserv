#!/usr/bin/env python3
import os
from datetime import datetime

print("Content-Type: text/html")
print()  # blank line ends CGI headers

print("<html><body style='font-family:sans-serif;background:#0f1115;color:#e6e6e6;padding:2rem;'>")
print("<h1>CGI is alive</h1>")
print(f"<p>Server time: {datetime.now()}</p>")
print(f"<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'unset')}</p>")
print(f"<p>QUERY_STRING: {os.environ.get('QUERY_STRING', 'unset')}</p>")
print("<p><a href='/'>back home</a></p>")
print("</body></html>")
