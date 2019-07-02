# transfer
不知为何，我们的某些服务器间用ftp无法传输文件，倒是能执行ftp的某些命令，我看防火墙配置也是正确的，主动或被动模式都没法传输文件，一时不知为何？索性就写个基于tcp的文件传输吧：stransfer作为服务端，ctransfer作为客户端，由客户端向服务端传输文件。

**某些服务器间用ftp无法传输文件的问题找到了**

过了两天后，代码写完，本地测试都已通过，拿到服务器上跑，居然还是不能传输数据到目标服务器，一直阻塞在第一次传输文件的阶段，也不知道是客户端阻塞，还是服务端阻塞，再过一段时间链接居然断开了（connection time out），拼命想是不是哪里出问题了？

* 如果是接收缓冲区满而又不recv，这就会导致发送缓冲区满，客户端就会阻塞在send，可是我一直在recv啊，所以不是这种情况。
* 如果是因为限速了呢，我把配置文件中的buffer_size（每次传输的大小）从4k改到1k，发现还是会阻塞，但不是阻塞在第一次传输，而且服务端确实接收到了一些数据。再从1k改到512，发现还是会阻塞，但会阻塞在第N次传输（反正比1k，4k时要多）。
* 如果还是因为限速，客户端又是连续send文件内容，根据tcp的Nagle算法，他会将多次send的内容填满发送缓冲区后再发送出去（第一次send不遵守这个规则，具体算法要百度一下），就在此时，发送的数据大小已经超过限制了，接收端无法接收数据。好，感觉有戏了，我就把发送端的socket设置为TCP_NODELAY（意思就是说tcp不用等发送缓冲区满后再发送，send一次就发送一次），残念啊，还是不行。

明明在其他机器上都正常，为什么在这里就不正常，而且反复review代码，确实是在不断recv啊，如果是因为限速了，我把buffer_size都调整到了512了都还是不行，真是奇了怪了！

吃过饭，刷完新闻，就在倒头入睡之时，反应过来了：

服务端使用epoll每隔3s检查一次socket，客户端为了提升传输效率，会连续不断send文件内容。从这里可以得知，由于客户端send之迅猛，在3s内，服务端的接收缓冲区会填满，这时，tcp对端不再会发送数据过来，数据会在发送缓冲区中等待（即使客户端将socket设置为TCP_NODELAY，意思就是说tcp不用等发送缓冲区满后再发送，send一次就发送一次），3s过后，服务端开始recv，tcp协议发现接收缓冲区被消费了，告诉发送端可以发消息了，此时，发送端一口气把发送缓冲区的数据全部发送，这里就出问题了，因为超过规定的数据量了（缓冲区大小肯定是大于4k的，很有可能限速是低于4k），所以buffer_size为4k时，总是阻塞在第一次，buffer_size改为2K，阻塞在第N次，buffer_size改为512，阻塞在第N+M次。

怎么改呢？客户端不要连续send，send一次，等待服务端回答一次，然后循环往复；buffer_size改小一点儿。

# stransfer
使用epoll管理多个客户端链接，接收客户端传过来的文件，并将之存放在指定位置。

*project path is `/root/stransfer`*
* build  
    ```bash
    $ cd /root/stransfer
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
    $ make
    $ sudo make install
    ```
* conf
    ```bash
    $ cat /root/stransfer/conf/example.conf
    # 监听地址
    host={ip}
    # 监听端口
    port={port}
    # 认证口令（自定义，但是客户端必须与此一致）
    key={key}
    # 日志目录
    log={dir}
    ```
* execute  
    ```bash
    $ stransfer /root/stransfer/conf/example.conf
    ```
# ctransfer
使用select管理socket链接，使用inotify监听指定目录下的文件状态，并根据状态将文件传输给服务端。

*project path is `/root/ctransfer`*
* build  
    ```bash
    $ cd /root/ctransfer
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
    $ make
    $ sudo make install
    ```
* conf
    ```bash
    $ cat /root/ctransfer/conf/example.conf
    # 服务端地址
    host={ip}
    # 服务端端口
    port={port}
    # 认证口令（必须与服务端口令一致）
    key={key}
    # 不用传输的文件（正则表达式）
    ignore={regex}
    # 监控目录
    src_dir={dir}
    # 目标目录
    dst_dir={dir}
    # 每次传输的大小(1~4096)
    buffer_size={size}
    # 日志目录
    log={dir}
    ```
* execute  
    ```bash
    $ ctransfer /root/ctransfer/conf/example.conf
    ```
