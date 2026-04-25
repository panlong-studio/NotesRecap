# Linux C 线程池项目 GDB 调试教程

这份文档基于当前项目代码整理，目标不是讲完所有 `gdb` 命令，而是围绕这个线程池项目，带你完成一场约 45 分钟的组会分享：从单进程调试，过渡到 `fork`、多线程、条件变量、队列和信号退出的调试。

## 1. 分享目标

分享结束时，听众应当能够掌握下面几件事：

1. 知道如何给 C 项目生成带调试信息的可执行文件。
2. 知道如何在 `gdb` 里调试 `fork` 出来的子进程。
3. 知道如何查看线程、切换线程、打印调用栈。
4. 知道如何围绕“主线程入队，工作线程出队”这条主线调试线程池。
5. 知道如何定位信号触发的优雅退出流程。

## 2. 项目代码主线

先用 3 到 5 分钟把代码链路讲清楚，后面所有断点都围绕这条链路展开。

### 2.1 服务端主流程

服务端入口在 `server/main.c`：

1. `pipe(pipe_fd)` 创建管道。
2. `fork()` 之后，父进程负责接收 `SIGINT`，子进程负责真正的服务器逻辑。
3. 子进程调用 `init_thread_pool(&pool, 4)` 创建 4 个工作线程。
4. 子进程调用 `init_socket(&listen_fd, "192.168.100.128", "12345")` 建立监听。
5. 主线程进入 `epoll_wait()`。
6. 新连接到来后，主线程 `accept()`，然后把 `conn_fd` 入队。
7. 工作线程从队列取出 `fd`，调用 `send_file(fd)` 给客户端发文件。
8. 收到退出信号后，主线程设置 `pool.exitFlag = 1`，广播条件变量，等待工作线程退出。

### 2.2 工作线程主流程

工作线程入口在 `server/worker.c` 的 `thread_func()`：

1. 先拿锁。
2. 如果 `queue.size == 0 && exitFlag == 0`，就在 `pthread_cond_wait()` 睡眠。
3. 被唤醒后，如果 `exitFlag == 1`，线程退出。
4. 否则从队列头部取出 `fd`。
5. 解锁。
6. 调用 `send_file(fd)` 处理客户端请求。

### 2.3 队列操作

队列逻辑在 `server/queue.c`：

1. `enQueue()` 负责入队。
2. `deQueue()` 负责出队。
3. 这是最适合演示“共享数据变化”的位置。

## 3. 当前仓库的实际注意事项

这部分建议在分享前自己先准备好，避免现场翻车。

### 3.1 当前 `Makefile` 直接执行会失败

当前代码里很多头文件写法是：

```c
#include <my_header.h>
```

但 `my_header.h` 实际位于 `server/my_header.h`。所以在我当前环境中，下面两条命令都会失败：

```bash
cd server && make
cd client && make
```

原因是没有提供正确的头文件搜索路径。

### 3.2 建议使用下面的显式编译命令

服务端：

```bash
cd /home/jerry/learning/thread_pool_gdb/server
gcc -g -I. epoll.c main.c queue.c send_file.c socket.c thread_pool.c worker.c -o main -lpthread
```

客户端：

```bash
cd /home/jerry/learning/thread_pool_gdb/client
gcc -Wall -g -I../server client.c -o client -pthread
```

上面两条命令已经在当前环境验证通过，可以用于你的分享。

### 3.3 `ip` 地址是硬编码的

`server/main.c:46` 和 `client/client.c:10-11` 都写死了 `192.168.100.128`。

这意味着：

1. 如果你的机器没有这个地址，服务端 `bind()` 会失败。
2. 客户端 `connect()` 也会失败。

分享前请先确认：

```bash
ip addr
```

如果你的机器地址不是 `192.168.100.128`，有两种做法：

1. 你的环境之前就是用这个地址测试通过的，那保持原样即可。
2. 临时把服务端和客户端里的 IP 改成你本机实际地址，或者改成 `127.0.0.1` 再重新编译。

### 3.4 `send_file()` 依赖测试文件

`server/send_file.c` 中固定发送：

```c
char *file_name = "The_Holy_Bible.txt";
```

所以分享前要保证这个文件存在于服务端运行目录，也就是：

```bash
/home/jerry/learning/thread_pool_gdb/server/The_Holy_Bible.txt
```

否则工作线程走到 `open(file_name, O_RDWR)` 时，后续行为就不适合当作正常调试案例演示。

## 4. 45 分钟分享安排建议

可以按这个节奏讲：

| 时间 | 内容 |
| --- | --- |
| 0 - 5 分钟 | 项目结构、线程池主线、为什么适合练 `gdb` |
| 5 - 10 分钟 | `gdb` 常用命令速览 |
| 10 - 18 分钟 | 案例 1：跟踪 `fork` 和线程池初始化 |
| 18 - 30 分钟 | 案例 2：跟踪“主线程 accept -> 入队 -> 条件变量唤醒” |
| 30 - 40 分钟 | 案例 3：跟踪“工作线程出队 -> send_file” |
| 40 - 45 分钟 | 案例 4：跟踪 `SIGINT` 触发的优雅退出，总结 |

## 5. 先讲这几个 `gdb` 基础命令

这部分不要讲太多，够用就行。

### 5.1 断点和运行

```gdb
break main
break init_thread_pool
break thread_func
run
continue
next
step
finish
```

建议你现场解释：

1. `break` 是下断点。
2. `run` 是启动程序。
3. `continue` 是继续运行到下一个断点。
4. `next` 按行执行，不进入函数内部。
5. `step` 按行执行，且会进入函数内部。
6. `finish` 是把当前函数直接执行完并返回上层。

### 5.2 看变量和源码

```gdb
list
print pool
print pool.queue.size
print conn_fd
display pool.queue.size
```

建议你强调：

1. `print` 看当前值。
2. `display` 每次停下来都自动打印，更适合盯变量变化。

### 5.3 多线程常用命令

```gdb
info threads
thread 2
bt
thread apply all bt
```

讲法建议：

1. `info threads` 看当前所有线程。
2. `thread 2` 切到某个线程。
3. `bt` 看当前线程调用栈。
4. `thread apply all bt` 一次性看所有线程栈，非常适合排查“哪个线程卡住了”。

### 5.4 `fork` 调试最关键的两条命令

这个项目必须讲。

```gdb
set follow-fork-mode child
set detach-on-fork off
```

解释：

1. `follow-fork-mode child` 表示 `fork()` 后让 `gdb` 跟着子进程走，因为真正的服务器逻辑在子进程里。
2. `detach-on-fork off` 表示父子进程都不放掉，后面可以切换查看。

## 6. 调试前的终端准备

建议准备 3 个终端。

### 6.1 终端 1：调试服务端

```bash
cd /home/jerry/learning/thread_pool_gdb/server
gdb ./main
```

### 6.2 终端 2：运行客户端

```bash
cd /home/jerry/learning/thread_pool_gdb/client
./client
```

### 6.3 终端 3：辅助命令

这个终端用于：

```bash
ps -ef | grep main
kill -SIGINT <父进程pid>
ls -l
```

## 7. 案例 1：跟踪 `fork` 和线程池初始化

这个案例适合讲 `gdb` 基础、`fork` 跟踪、线程创建。

### 7.1 演示目标

回答三个问题：

1. 服务端到底在哪个进程里真正跑起来？
2. 线程池什么时候创建？
3. 4 个工作线程是否真的都创建成功？

### 7.2 推荐命令

进入 `gdb` 后输入：

```gdb
set follow-fork-mode child
set detach-on-fork off
break main
break init_thread_pool
break thread_func
run
```

### 7.3 你应该讲的观察点

程序先停在 `main()`，这时可以先 `list` 看主流程。

继续执行：

```gdb
next
next
next
```

当执行到 `fork()` 附近时，重点讲：

1. 父进程负责接收信号。
2. 子进程负责真正的服务器逻辑。
3. 如果不设置 `follow-fork-mode child`，你很容易跟错进程。

继续运行到 `init_thread_pool()`：

```gdb
continue
```

停住后执行：

```gdb
print num
print *pool
```

这里可以解释：

1. `num` 传入的是 4。
2. `pool` 里的锁、条件变量、队列和线程数组都将在这里初始化。

### 7.4 演示线程创建

在 `server/thread_pool.c` 中，线程创建逻辑在 `pthread_create()`。

你可以在 `thread_pool.c` 里单步：

```gdb
next
next
next
```

或者直接：

```gdb
until 27
next
```

每次经过 `pthread_create()` 后，观察：

```gdb
info threads
```

你会看到线程数逐渐增加。随后 `thread_func()` 断点会被命中。

这时讲两点：

1. 新线程不是“抽象概念”，在 `gdb` 里可以真实看到。
2. 工作线程刚启动后会很快进入 `pthread_cond_wait()` 等任务。

### 7.5 这一段分享的总结话术

可以用一句话收尾：

“这个项目不是单进程单线程，所以调试第一步不是盲目下断点，而是先搞清楚我现在跟着的是哪个进程、哪个线程。”

## 8. 案例 2：跟踪主线程 `accept -> 入队 -> 唤醒工作线程`

这个案例最适合讲“共享资源”和“生产者-消费者模型”。

### 8.1 演示目标

回答四个问题：

1. 客户端连上来后，主线程停在哪？
2. `conn_fd` 是多少？
3. 队列长度什么时候从 0 变成 1？
4. 条件变量什么时候唤醒工作线程？

### 8.2 推荐断点

```gdb
break server/main.c:91
break server/main.c:97
break server/main.c:100
commands
silent
printf "queue.size = %d\n", pool.queue.size
continue
end
```

如果你的 `gdb` 不接受 `server/main.c:91` 这种写法，也可以直接写：

```gdb
break main.c:91
break main.c:97
break main.c:100
```

### 8.3 推荐演示步骤

1. 先让服务端在 `epoll_wait()` 附近跑起来。
2. 在服务端 `gdb` 里输入：

```gdb
display pool.queue.size
continue
```

3. 然后在终端 2 运行客户端：

```bash
cd /home/jerry/learning/thread_pool_gdb/client
./client
```

4. 服务端会在 `accept()` 之后停住。

### 8.4 现场重点打印的变量

在 `accept()` 之后停住时，输入：

```gdb
print conn_fd
print pool.queue.size
```

然后单步到入队：

```gdb
next
step
```

进入 `enQueue()` 后，再看：

```gdb
print *pQueue
print fd
```

你可以强调：

1. 主线程本质上是生产者。
2. 它并不处理文件发送，只负责把任务封装成 `fd` 放进队列。

### 8.5 入队后观察队列变化

在 `enQueue()` 中推荐讲这几行：

1. `pNew = calloc(...)`
2. `pQueue->head = pNew`
3. `pQueue->end = pNew`
4. `pQueue->size++`

可用命令：

```gdb
next
print pQueue->size
next
print pQueue->size
finish
```

回到 `main()` 后继续：

```gdb
print pool.queue.size
next
```

执行到 `pthread_cond_signal(&pool.cond)` 时，重点说明：

1. 入队本身不会让任务被处理。
2. 真正触发工作线程开始竞争任务的是条件变量通知。

## 9. 案例 3：跟踪工作线程 `等待 -> 唤醒 -> 出队 -> send_file`

这是整场分享最核心的一段，建议多留时间。

### 9.1 演示目标

回答五个问题：

1. 工作线程平时睡在哪里？
2. 被唤醒后先检查什么条件？
3. 它从哪里拿到任务 `fd`？
4. 队列长度什么时候减 1？
5. 最终是谁调用了 `send_file()`？

### 9.2 推荐断点

```gdb
break worker.c:24
break worker.c:27
break worker.c:35
break worker.c:37
break worker.c:43
```

### 9.3 先看线程状态

当服务端停住时，先输入：

```gdb
info threads
thread apply all bt
```

你大概率会看到：

1. 主线程在 `epoll_wait()`。
2. 若干工作线程在 `pthread_cond_wait()`。

这时你可以直接解释线程池本质：

“线程池的工作线程不是一直忙，它们大部分时间都阻塞在条件变量上等待任务。”

### 9.4 工作线程被唤醒后如何观察

当客户端连接后，某个工作线程会命中 `worker.c` 断点。

先看当前线程是谁：

```gdb
info threads
bt
```

然后打印关键变量：

```gdb
print pool->queue.size
print pool->exitFlag
print fd
```

### 9.5 重点讲 `while` 而不是 `if`

`server/worker.c:22` 这一行很值得讲：

```c
while(pool->queue.size == 0 && pool->exitFlag == 0)
```

这里你可以结合 `gdb` 说：

1. 工作线程醒来后并不代表一定有任务。
2. 条件变量可能出现虚假唤醒。
3. 所以必须重新检查条件，这就是为什么这里必须用 `while` 而不是 `if`。

### 9.6 看出队动作

停在 `worker.c:35` 附近时，执行：

```gdb
print pool->queue.head->fd
print pool->queue.size
next
step
```

进入 `deQueue()` 后：

```gdb
print *pQueue
next
next
print pQueue->size
finish
```

回到 `thread_func()` 后，再执行：

```gdb
print pool->queue.size
```

这里是整场分享中最清楚的“队列长度变化”展示点。

### 9.7 跟踪 `send_file()`

停在 `worker.c:43` 后，执行：

```gdb
step
```

进入 `send_file()` 后可以展示：

```gdb
print file_name
next
print file_fd
next
print st.st_size
```

你要讲清楚：

1. 工作线程拿到任务后，真正干活的是 `send_file(fd)`。
2. 它先发文件名长度，再发文件名，再发文件大小，最后 `sendfile()` 发送内容。

### 9.8 这一段结束时的总结

可以用下面这句话：

“到这里，线程池的完整消费路径就打通了：主线程生产任务，工作线程被唤醒，拿锁取任务，解锁后执行真正业务。”

## 10. 案例 4：跟踪 `SIGINT` 触发的优雅退出

这段非常适合收尾，因为它把信号、管道、广播唤醒、线程退出串起来了。

### 10.1 演示目标

回答四个问题：

1. `Ctrl+C` 或 `SIGINT` 到底是谁接收？
2. 父进程怎样通知子进程？
3. 子进程如何让所有工作线程退出？
4. 为什么退出时要 `pthread_join()`？

### 10.2 推荐断点

```gdb
break func
break main.c:68
break main.c:77
break worker.c:27
```

### 10.3 这个案例的一个关键提醒

在 `gdb` 里直接按 `Ctrl+C`，很多时候是先打断 `gdb` 本身，而不是把 `SIGINT` 正确送给你的被调试程序。

更稳妥的做法是：

1. 让服务端在 `gdb` 里正常运行。
2. 在终端 3 里找到父进程 PID：

```bash
ps -ef | grep /home/jerry/learning/thread_pool_gdb/server/main
```

3. 手工发送信号：

```bash
kill -SIGINT <父进程pid>
```

### 10.4 调试路径

先在 `func()` 命中断点，讲清楚：

1. 父进程收到 `SIGINT`。
2. 处理函数 `func(int num)` 往 `pipe_fd[1]` 写一个字节。
3. 子进程的主线程在 `epoll` 中监听 `pipe_fd[0]`。

继续运行后，子进程会在：

```c
if(fd == pipe_fd[0])
```

这段逻辑停住。

此时可打印：

```gdb
print pool.exitFlag
next
next
print pool.exitFlag
```

你会看到它从 `0` 变成 `1`。

### 10.5 为什么要 `pthread_cond_broadcast`

停在：

```c
pthread_cond_broadcast(&pool.cond);
```

这里重点讲：

1. 如果工作线程都睡在 `pthread_cond_wait()`，单纯把 `exitFlag` 设成 `1` 是不够的。
2. 必须广播，把所有睡眠线程唤醒。
3. 工作线程醒来后会走到 `worker.c:27` 的 `if(pool->exitFlag == 1)`，然后退出。

### 10.6 看所有线程退出前的栈

这里推荐展示：

```gdb
thread apply all bt
```

能让听众看到：

1. 主线程在等子线程回收。
2. 工作线程有的在等待，有的刚被唤醒准备退出。

### 10.7 这一段结束时的总结

可以收束成一句话：

“这个退出流程很适合用 `gdb` 学信号调试，因为它把父子进程通信、`epoll` 监听、条件变量广播和线程回收都串起来了。”

## 11. 分享时可以直接抄的命令清单

如果你怕现场忘命令，可以单独把这一段打印出来。

### 11.1 启动前编译

```bash
cd /home/jerry/learning/thread_pool_gdb/server
gcc -g -I. epoll.c main.c queue.c send_file.c socket.c thread_pool.c worker.c -o main -lpthread

cd /home/jerry/learning/thread_pool_gdb/client
gcc -Wall -g -I../server client.c -o client -pthread
```

### 11.2 服务端进入 `gdb`

```bash
cd /home/jerry/learning/thread_pool_gdb/server
gdb ./main
```

### 11.3 `gdb` 常用命令

```gdb
set follow-fork-mode child
set detach-on-fork off
break main
break init_thread_pool
break thread_func
run
continue
next
step
finish
list
print pool.queue.size
display pool.queue.size
info threads
thread 2
bt
thread apply all bt
```

### 11.4 客户端触发一次任务

```bash
cd /home/jerry/learning/thread_pool_gdb/client
./client
```

### 11.5 发送退出信号

```bash
ps -ef | grep /home/jerry/learning/thread_pool_gdb/server/main
kill -SIGINT <父进程pid>
```

## 12. 这几个点最容易被问到

分享时大概率会被问下面这些问题，你可以提前准备。

### 12.1 为什么 `pthread_cond_wait()` 前面要用 `while`？

答：

因为线程被唤醒后，不代表条件一定满足，可能是虚假唤醒，也可能是别的线程已经把任务拿走了，所以必须重新检查 `queue.size` 和 `exitFlag`。

### 12.2 为什么主线程入队后不直接处理任务？

答：

这是典型的线程池设计。主线程负责接收连接并分发任务，真正的业务处理交给工作线程，这样主线程不会被文件传输阻塞。

### 12.3 为什么退出时要先 `broadcast`，再 `join`？

答：

因为线程可能都睡在条件变量上。如果不先唤醒，它们不会检查 `exitFlag`，主线程就可能永远卡在 `pthread_join()`。

### 12.4 这个项目为什么适合学 `gdb`？

答：

因为它同时覆盖了几个调试高频场景：`fork`、多线程、条件变量、共享队列、文件发送、信号退出。比纯算法例子更接近真实 Linux 服务端程序。

## 13. 你分享时可以直接用的结尾总结

最后 1 到 2 分钟，可以用下面这段思路收尾：

1. `gdb` 不只是“程序崩了再看栈”，更重要的是理解程序运行过程。
2. 在线程池项目里，最重要的是先分清进程、线程、共享数据和同步机制。
3. 调试复杂程序时，不要一上来乱打断点，要先想清楚主线。
4. 这个项目最值得反复练的是三条线：
   `fork` 线、任务队列线、信号退出线。

## 14. 建议你分享前至少彩排一次

最低彩排标准：

1. 能成功编译服务端和客户端。
2. 能确认服务端绑定 IP 正确。
3. 能确认 `The_Holy_Bible.txt` 存在。
4. 能至少完整跑通一次“客户端连接 + 工作线程发文件”。
5. 能完整演示一次 `SIGINT` 退出。

如果这五步都能跑通，这场 45 分钟分享基本不会失控。
