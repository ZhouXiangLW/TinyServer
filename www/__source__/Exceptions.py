
class SourceNotFoundError(Exception):
    '''
    This error will return 404 page
    '''
    def __init__(self, message=''):
        super(SourceNotFoundError, self).__init__(message)

class ServerFaultError(Exception):
    '''
    
    This error will return 500 page
    '''
    def __init__(self, message=''):
        super(SourceNotFoundError, self).__init__(message)

class NotSupportError(Exception):
    
    def __init__(self, content_type):
        super(NotSupportError, self).__init__('TinyServer do not support ' + content_type)


