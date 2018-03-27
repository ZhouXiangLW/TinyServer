HttpRequest类

## 概述
1. HttpRequest类是一个有穷状态机，状态机每读取一行解析一行，遇到空行时结束。
2. 该类只读取HTTP请求，并将请求整理归类，不做实质性的解析。
3. 读取采用非阻塞形式，并交于epoll管理。

## 设计
### 请求报文格式
```
请求行： <请求方法> <URL> <协议版本><cr><lf>
首部行： <头部字段名>: <值><cr><lf>
        ...
        <头部字段名>: <值><cr><lf>
        <cr><lf>
请求体： 仅存在于某些请求报文中，无固定格式
```

### 状态机设计
HttpRequest类采用非阻塞设计，一次性可能无法读取一个完整的请求，所以该类还需要记录读取的状态。
状态类：
```
enum LineStatus
{
    LINE_OK,
    LINE_OPEN,
    LINE_BAD
};
enum HttpCode
{
    GET_REQUEST,
    BAD_REQUEST,
    CLOSED_CONNECTION
}
```
#### 状态机描述
