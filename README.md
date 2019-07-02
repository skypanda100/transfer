# transfer
不知为何，我们的某些服务器间用ftp无法传输文件，倒是能执行ftp的某些命令，我看防火墙配置也是正确的，主动或被动模式都没法传输文件，一时不知为何？索性就写个基于tcp的文件传输吧：stransfer作为服务端，ctransfer作为客户端，由客户端向服务端传输文件。

**某些服务器间用ftp无法传输文件的问题找到了**

参考：https://skypanda100.github.io/2019/07/02/linux.tcp.transfer/

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
