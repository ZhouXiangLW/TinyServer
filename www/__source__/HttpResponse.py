

class HttpResponse:

    def __init__(self, body, status_code, body_type):
        self.body = body
        self.status_code = status_code
        self.body_type = body_type

    def __get_response_text__(self):
        resp = ''
        if self.status_code == 200:
            resp = resp + 'HTTP/1.1 200 OK\r\n' 
        elif self.status_code == 404:
            resp = resp + 'HTTP/1.0 404 NOT FOUND\r\n'
        elif self.status_code == 500:
            resp = resp + 'HTTP/1.0 404 Internal Server Error\r\n'
        resp = resp + 'Server: TinyServer 1.0\r\n'
        if self.body_type == 'static_file':
            resp = resp + 'Content-Type: text/html;charset=UTF-8\r\n'
        elif self.body_type == 'json':
            resp = resp + 'Content-Type: application/json\r\n'
        resp = resp + 'Content-length: %d\r\n' % len(self.body)
        resp = resp + '\r\n'
        resp = resp + self.body
        self.resp = resp

    def send_to_server(self):
        self.__get_response_text__()
        print('%10d' % len(self.resp), end='')
        print(self.resp, end='')