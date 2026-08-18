console.log("Content-Type: text/html", '\n')

console.log("<html><body style='font-family:sans-serif;background:#0f1115;color:#e6e6e6;padding:2rem;'>");
console.log("<h1>CGI is alive</h1>");
console.log(`<p>Server time: ${new Date().toString()}</p>`);
console.log(`<p>REQUEST_METHOD: ${process.env.REQUEST_METHOD || 'unset'}</p>`);
console.log(`<p>QUERY_STRING: ${process.env.QUERY_STRING || 'unset'}</p>`);
console.log("<p><a href='/'>back home</a></p>");
console.log("</body></html>");
