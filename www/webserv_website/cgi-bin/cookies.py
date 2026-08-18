#!/usr/bin/env python3
import os
import sys
from datetime import datetime
from urllib.parse import parse_qs

# retrieve environment variables passed by your C++ web server
query_string = os.environ.get('QUERY_STRING', '')
cookie_header = os.environ.get('HTTP_COOKIE', '')

# parse the query string into a dictionary
query_params = parse_qs(query_string)

name = "Guest"
set_cookie = False

# Phase 1: Check if 'name' is in the query string
if 'name' in query_params:
    name = query_params['name'][0]
    set_cookie = True
# Phase 2: Check if 'name' is in the Cookie header (subsequent requests)
elif 'name=' in cookie_header:
    cookies = cookie_header.split(';')
    for cookie in cookies:
        if cookie.strip().startswith('name='):
            name = cookie.strip().split('=')[1]

# Write headers to standard output
# HTTP headers strictly require \r\n line endings
if set_cookie:
    sys.stdout.write(f"Set-Cookie: name={name}; Max-Age=3600\r\n")

sys.stdout.write("Content-Type: text/html\r\n\r\n")

# Write the HTML body
html_body = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>webserv // CGI</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <header>
        <h1>webserv <span>CGI script output</span></h1>
        <p class="subtitle">Python CGI — Dynamic Cookie Generation</p>
    </header>
    <main>
        <section class="card">
            <h2>Hello, {name}!</h2>
            <p>Server time: <code>{datetime.now()}</code></p>
            <p>REQUEST_METHOD: <code>{os.environ.get('REQUEST_METHOD', 'unset')}</code></p>
            <p>QUERY_STRING: <code>{query_string}</code></p>
            <p>HTTP_COOKIE: <code>{cookie_header}</code></p>
            <button onclick="window.location.href='/'">Back Home</button>
        </section>
    </main>
</body>
</html>"""

sys.stdout.write(html_body)
