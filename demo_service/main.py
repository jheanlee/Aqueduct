from http.server import HTTPServer, BaseHTTPRequestHandler

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    # Handle GET requests
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"<html><body><h1>Hello, World!</h1></body></html>")

    # Handle POST requests
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))  # Get the size of data
        post_data = self.rfile.read(content_length)  # Read the posted data
        print(f"Received POST data: {post_data.decode('utf-8')}")

        # Send a response back to the client
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        response = b'{"status": "success", "message": "Data received"}'
        self.wfile.write(response)

# Server setup
def run(server_class=HTTPServer, handler_class=SimpleHTTPRequestHandler, port=8000):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting server on port {port}...")
    httpd.serve_forever()

if __name__ == "__main__":
    run()
