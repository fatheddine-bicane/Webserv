console.log("Content-Type: text/html", '\n')

console.log("<html><body style='font-family:sans-serif;background:#0f1115;color:#e6e6e6;padding:2rem;'>")
console.log("<h1>CGI is alive</h1>")
console.log("<p>Server time: {datetime.now()}</p>")
console.log("<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'unset')}</p>")
console.log("<p>QUERY_STRING: {os.environ.get('QUERY_STRING', 'unset')}</p>")
console.log("<p><a href='/'>back home</a></p>")
console.log("</body></html>")
