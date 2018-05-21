
class HttpResponse:
    # __slots__ = ('status_code', 'body', 'content_type', 'content_charset', 'content_encoding', 'content_length')

    @property
    def status_code(self):
        return self.status_code

    @status_code.setter
    def status_code(self, status_code):
        if status_code != 200 and status_code != 404 and status_code != 500:
            raise 
        
