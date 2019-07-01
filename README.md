# transfer
基于tcp实现文件传输，stransfer作为服务端，ctransfer作为客户端，客户端向服务端传输文件。
# stransfer
使用epoll管理多个客户端链接，接收客户端传过来的文件，并将之存放在指定位置。

*project path is `/root/stransfer`*
* build  
    ```bash
    $ cd /root/stransfer
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
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
    $ cd /root/stransfer/build
    $ ./stransfer /root/stransfer/conf/example.conf
    ```
# ctransfer
使用select管理socket链接，使用inotify监听指定目录下的文件状态，并根据状态将文件传输给服务端。

*project path is `/root/ctransfer`*
* build  
    ```bash
    $ cd /root/ctransfer
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
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
    # 日志目录
    log={dir}
    ```
* execute  
    ```bash
    $ cd /root/ctransfer/build
    $ ./ctransfer /root/ctransfer/conf/example.conf
    ```
