 # mymuduo  one thread per loop 
#go语言的思想，不要用共享去通信（即生产者消费者模型），使用通信去共享。   
Socket类：封装服务器fd，提供bind，listen，accept方法监听新连接。构造函数参数：sockFd  
InetAddress类：封装sockaddr_in ipv4地址，提供把ip和端口转换为字符串。构造函数参数：端口和ip  
Channel类：封装需要监听的fd并绑定loop，注册在epoll，和设置事件发生的回调。构造函数参数：loop和fd。  
EpollPoller类：poller的具体类，绑定loop，创建epoll对象，注册事件发生数组。提供updata成员函数执行epollctl注册channel到epoll和pool成员函数传出有事件发生的channel。  
Eventloop类：事件循环类，用于管理channel和epoll，实现channel注册和epoll响应，提供loop函数，调用epoll注册fd和监听fd事件发生，同时添加不属于该reactor的回调，分发给子reactor，并且该类对象绑定在一个线程上。子loop为什么可以正确执行回调，eventloop对象包含一个回调函数的动态数组，当当前线程和对象不匹配时，把需要执行的回调加入动态数组并通过wakeupfd唤醒该loop对象绑定的线程。  
thread类：创建一个线程，并把使用该线程执行给EventLoopThread使用。成员函数start，开启一个新的线程，并在新线程中执行EventLoopThread类的回调，绑定loop，开启reactor；  
EventLoopThread类，绑定线程和loop对象，startLoop返回一个绑定了线程的loop给线程池统一管理。  
EventLoopThreadPool类，管理线程和loop对象，提供轮询方法调用线程工作，getNextLoop成员函数首先保留主loop指针，要是没有其他线程则使用主loop执行回调，返回一个loop。  
Acceptor类：构造函数绑定loop，设置监听的fd（即在哪个fd上监听），设置channel（绑定loop和fd），设置回调（注册新连接的回调）。TCP的listen和accept都在这实现  
TcpServer类：该类直接提供给用户使用，构造函数初始化loop（主reactor），初始化accepor对象，初始化线程池。并且设置新连接的回调，即有新连接时调用。向用户提供start函数，该函数执行代表在loop这个线程上在sockfd上listen，因为该fd对应的channel已经注册了可读事件，当发生可读事件时（即新连接到来）调用newConnection，轮询找loop来执行这个连接，并使用Tcpconnection管理这个连接，注册对应的连接回调。  
TcpConnection类：构造函数：对应的loop，sockfd是之前acceptor对象accept函数生成连接fd（对该fd进行读写就是对客户端进行读写），提供读写回调，操作已经建立的连接，把数据存入buffer类对象。  
TimeStamp类：构造函数传入一个时间戳（一个距离1970的64位整数的毫秒数），并且重写<和==运算符  
Timer类：构造函数入参：回调函数，过期时间，超时间隔（对于重复定时器每隔多久超时一次），是否是重复定时器，定时器序号，静态变量定时器数量。用于和fd绑定    
TimerQueue类：构造函数传入参数loop对象，定时fd，定时fd的channel，一个Timer类对象的set，相当于在一个timefd上设置多个定时任务，把timefd注册在epoll上，再使用loop调用channel处理事件回调。  
Buffer类：只在ioloop中使用，只用于处理已经建立的连接，即在Tcpconnection中使用，用于数据缓存，对于TCP连接，buffer是应用层的缓存，从tcp的内核缓冲区取数据，使用包长解决粘包，使用读写指针读写缓冲区。在写内核缓冲时，没有写完的内容写入应用层buffer，并注册可写事件。  
