
class HttpResponse:
    __slots__ = ('status_code', 'body', 'content_type', 'content_charset', 'content_encoding', 'content_length')

    @status_code.setter
    def set_status_code(status_code):
        
