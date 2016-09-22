# proxy_cpp

A simple tcp proxy which provides a hex dump of all traffic. Implemented using boost asio

Usage:
proxy <remote host> <remote host> <local port>

Sample output using echo client/server provided with boost asio:
```
mmcilroy@dev01:~/share/proxy_cpp/build$ ./proxy localhost 14002 14001
accept starting
accepted
connecting to localhost:14002
2 bytes from local
  0000  68 69                                            hi
2 bytes from remote
  0000  68 69                                            hi
5 bytes from local
  0000  74 68 65 72 65                                   there
5 bytes from remote
  0000  74 68 65 72 65                                   there
5 bytes from local
  0000  62 75 64 64 79                                   buddy
5 bytes from remote
  0000  62 75 64 64 79                                   buddy
```