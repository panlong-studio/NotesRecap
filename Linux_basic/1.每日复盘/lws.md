# I/O多路复用：select 

## 一、 `select()` 核心概念

### 1. 解决的痛点
- **传统阻塞问题**：默认情况下，如果程序调用 `read(STDIN_FILENO, ...)` 监听键盘输入，程序会被阻塞，此时如果管道（Pipe）或其他文件描述符（FD）中来了数据，程序也无法去处理。
- **`select` 的作用**：`select` 相当于一个“系统级监听器”，系统提供它用于**同时监听多个文件描述符**的状态。它会阻塞等待，直到被监听的文件描述符中**至少有一个可用**时，才结束阻塞并唤醒程序去“捞数据”。

### 2. 文件描述符“可用”的含义
- **`STDIN_FILENO` 可用**：代表标准输入缓冲区中有数据（即用户在键盘中输入了数据）。
- **`read_fd` (如管道/文件) 可用**：代表该管道或文件中有数据到达，调用 `read` 不会阻塞。

------

## 二、 文件描述符集合：`fd_set` 及操作宏

### 1. 核心数据结构
- **`fd_set`**：这是一个由系统提供的数据类型，本质上是一个位图（Bit Map），用于存放需要被 `select` 监听的文件描述符集合。
- **封装性**：系统不希望开发者直接去操作这个结构体内部的字段，而是提供了一系列专用的宏函数来管理它。

### 2. 常用操作宏
- **`FD_ZERO(fd_set *set)`**
  - **作用**：清空文件描述符集合，将集合中所有的标志位清零。
  - **注意**：在每次调用 `select` 前，或初始化 `fd_set` 变量时，**必须**先调用此宏。
- **`FD_SET(int fd, fd_set *set)`**
  - **作用**：将特定的文件描述符（如 `STDIN_FILENO` 或自定义的 `read_fd`）添加到监听集合中。
- **`FD_ISSET(int fd, fd_set *set)`**
  - **作用**：判断特定的文件描述符是否在集合中。
  - **应用场景**：在 `select` 结束阻塞后，用于测试到底是哪一个文件描述符触发了事件（即可用）。

------

## 三、 `select()` 函数调用机制

### 1. 函数原型与参数
虽然完整原型较复杂，但在监听“可读”事件时，常用如下结构：
```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

- **`nfds`**：需要监听的最大文件描述符的值 **加 1**。例如，同时监听 `STDIN_FILENO`(0) 和 `read_fd`(如 3)，则传入 `read_fd + 1`（即 4）。
- **`readfds`**：要监听的读操作文件描述符集合
- **`writefds`**：要监听的写操作文件描述符集合
- **`exceptfds`**：要监听的异常操作文件描述符集合
- **`timeout`**：监听时候的阻塞时间，NULL代表一直等待直到指定就绪,0代表不等待检查文件描述符立即返回

### 2. 传入传出参数特性 (⚠️ 重点)

- `select` 的第二个参数 `readfds` 是一个**传入传出参数**（入参未用 `const` 修饰）。
- **传入时**：它告诉内核我们需要监听哪些 FD。
- **传出时**：内核会修改这个集合，**将其覆盖为仅包含“已经就绪（可用）”的 FD 集合**。
- **结论**：正因为 `select` 会修改传入的 `fd_set`，所以在循环结构（如 `while(1)`）中使用 `select` 时，**每次循环都必须重新 `FD_ZERO` 并重新 `FD_SET`**。

------

## 四、 典型应用场景与最佳实践

### 1. 基础多路复用（双向聊天室模型）

利用 `select` 可以轻松实现两个终端（User A 和 User B）通过两个管道（1.pipe 和 2.pipe）进行无阻塞双向通信：

1. 打开相应的管道读写文件描述符。
2. 循环内部重置 `fd_set`。
3. 将 `STDIN_FILENO` 和 `read_fd` 加入集合。
4. 调用 `select` 阻塞等待。
5. 使用 `FD_ISSET` 分支处理：如果键盘输入就发给对方，如果管道有数据就打印到屏幕。

### 2. 对端关闭检测 (EOF 检测)

在进行 IPC (进程间通信) 时，如果通信的一方（写端）退出了，另一方（读端）需要能感知到：

- **原理**：当管道的所有写端都关闭时，`read()` 读取该管道会立刻返回，且返回值为 `0`。
- **处理方式**：在 `FD_ISSET` 触发管道可读事件后，需要接收 `read()` 的返回值并判断：

```c
if(FD_ISSET(read_fd, &set)){
    char buf[100] = {0};
    ssize_t count = read(read_fd, buf, sizeof(buf));
    
    // 关键判断：count == 0 说明对端已关闭写端
    if(count == 0){
        printf("Peer user closed.\n");
        break; // 退出循环
    }
    printf("msg from peer: %s\n", buf);
}
```

- **资源回收**：退出循环后，务必调用 `close(read_fd)` 和 `close(write_fd)` 关闭本端的文件描述符，避免资源泄漏。

------



# 创建进程：fork 

## 一、 进程创建：`fork()`

### 1. `fork()` 的作用

- `fork()` 用于**创建子进程**。
- 一个进程调用一次 `fork()` 后，系统会创建一个新的进程，这个新进程称为**子进程**，原来的进程称为**父进程**。
- 子进程几乎是父进程的一个副本，会从 `fork()` 调用处继续向下执行。

### 2. 函数原型

```c
#include <unistd.h>

pid_t fork(void);
```

### 3. 返回值含义

`fork()` 调用一次，但会返回两次：

- **在父进程中返回值 > 0**
  - 返回新创建的**子进程 PID**
- **在子进程中返回值 = 0**
  - 表示当前正在子进程中执行
- **返回值 < 0**
  - 表示创建失败，没有创建出子进程

常见写法：

```c
pid_t pid = fork();

if (pid < 0) {
    perror("fork");
} else if (pid == 0) {
    printf("当前是子进程\n");
} else {
    printf("当前是父进程，子进程 pid = %d\n", pid);
}
```

### 4. `fork()` 的执行特点

- `fork()` 之前的代码，父进程只执行一次。
- `fork()` 之后的代码，父子进程都会继续执行。
- 父子进程谁先执行，**由调度器决定**，是不确定的。
- 因此，涉及输出顺序时，**顺序通常不确定**，但“总次数”通常可以确定。

示例：

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("before fork\n");

    fork();

    printf("after fork\n");
    return 0;
}
```

可能输出：

```c
before fork
after fork
after fork
```

解释：

- `before fork` 在 `fork()` 前执行一次
- `after fork` 在 `fork()` 后，父子各执行一次，因此输出两次

### 5. 父子进程的基本特征

父子进程之间有“相同”也有“不同”。

**相同点：**

- 子进程会继承父进程的：
  - 代码段
  - 数据段
  - 堆
  - 栈
  - 环境变量
  - 打开的文件描述符
  - 当前工作目录
  - 用户 ID、组 ID、权限相关属性等

**不同点：**

- PID 不同
- 父进程 PID（PPID）不同
- 未决信号集通常清空
- 资源统计信息不同
- 调度状态不同
- 返回值不同：父进程中 `fork()` 返回子进程 PID，子进程中返回 0

### 6. `fork()` 的本质理解

可以把 `fork()` 理解成：

- **复制一个几乎一样的进程**
- 复制时，子进程从父进程当前执行的位置继续运行
- 但父子进程是**两个独立的进程**

所以要特别注意：

- 两个进程有各自独立的地址空间
- 后续修改变量时互不影响
- 输出到终端时会竞争 CPU，顺序可能交错

------

## 二、 `fork()` 之后的内存机制：写时拷贝（Copy-On-Write, COW）

### 1. 为什么不能真的完整复制一份内存

如果父进程很大，`fork()` 时把整个内存空间都复制一份给子进程，会非常低效：

- 时间开销大
- 内存开销大

因此现代 Linux 使用**写时拷贝**机制优化。

### 2. COW 的工作原理

`fork()` 发生时：

1. 内核创建子进程的 PCB（进程控制块）
2. 子进程复制父进程的页表等管理信息
3. 父子进程最开始**共享同一份物理内存页**
4. 这些共享页面被标记为只读
5. 当父或子某一方尝试修改数据时，才真正复制对应页面

这就是“写时拷贝”：

- **读共享**
- **写复制**

### 3. COW 的意义

- 提高 `fork()` 执行效率
- 节省内存
- 尤其适合 `fork()` 后立刻 `exec()` 的场景，因为很多情况下根本不需要真的复制大量内存

### 4. 示例说明

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    int x = 10;
    pid_t pid = fork();

    if (pid == 0) {
        x = 20;
        printf("子进程 x = %d\n", x);
    } else {
        sleep(1);
        printf("父进程 x = %d\n", x);
    }

    return 0;
}
```

结果通常是：

```c
子进程 x = 20
父进程 x = 10
```

说明：

- 虽然子进程最初继承了 `x`
- 但子进程修改的是自己的副本
- 父进程中的 `x` 不受影响

------

## 三、 父子进程与变量、地址、文件描述符

### 1. 变量是否共享

`fork()` 后，父子进程的普通变量**逻辑上不共享**。

示例：

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    int a = 100;
    pid_t pid = fork();

    if (pid == 0) {
        a++;
        printf("子进程: a = %d\n", a);
    } else {
        sleep(1);
        printf("父进程: a = %d\n", a);
    }

    return 0;
}
```

输出：

```c
子进程: a = 101
父进程: a = 100
```

### 2. 地址看起来相同，但不是共享变量

有时打印变量地址，会发现父子进程中地址值一样：

```c
printf("%p\n", (void *)&a);
```

这并不表示它们在共享同一块真实物理内存，而是因为：

- 父子进程的**虚拟地址空间布局相似**
- 但它们属于两个不同进程
- 最终映射到的物理页可以不同

### 3. 文件描述符的继承

父进程 `fork()` 之后，子进程会继承父进程打开的文件描述符。

这意味着：

- 父子进程都能访问同一个打开的文件
- 它们往往共享同一个文件偏移量（因为底层共享同一个 open file description）

示例：

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        write(fd, "child\n", 6);
    } else {
        write(fd, "parent\n", 7);
    }

    close(fd);
    return 0;
}
```

说明：

- 父子进程都持有 `fd`
- 写入顺序不确定
- 但本质上访问的是同一个已打开文件对象

------

## 四、 `fork()` 与缓冲区问题

### 1. `printf()` 为什么会影响结果

`printf()` 使用的是**标准 I/O 缓冲区**。

不同情况：

- 输出到终端时，遇到 `\n` 常常会刷新（行缓冲）
- 输出到文件时，可能是全缓冲
- 没有换行且没手动刷新时，内容可能仍留在缓冲区

### 2. `fork()` 会复制缓冲区状态

如果在 `fork()` 之前，缓冲区中已经有尚未刷新的内容，那么：

- `fork()` 之后，父子进程都会继承这份缓冲内容
- 之后一旦各自退出并刷新缓冲区，就可能把相同内容都输出一遍

这正是很多 `fork()+printf()` 题目的核心。

### 3. 示例：有无换行的区别

示例 1：

```c
printf("a\n");
fork();
```

通常：

- `a\n` 已经被刷新
- 不会因为 `fork()` 被重复带走

示例 2：

```c
printf("a");
fork();
```

通常：

- `"a"` 还在缓冲区里
- `fork()` 后父子各带一份
- 退出时都刷新，可能看到两个 `a`

### 4. `fflush(stdout)` 的作用

```c
fflush(stdout);
```

作用：

- 立即刷新标准输出缓冲区
- 避免缓冲内容在 `fork()` 时被复制并重复输出

### 5. `_exit()` 和 `exit()` 的区别与缓冲

- `exit()` 会刷新 stdio 缓冲区
- `_exit()` 不会刷新 stdio 缓冲区

所以在 `fork()` 后的子进程中，如果不希望重复刷新继承来的缓冲区，常用 `_exit()`。

------

## 五、 `fork()` 的典型使用方式

### 1. 基本创建子进程

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        printf("我是子进程，pid = %d, ppid = %d\n", getpid(), getppid());
    } else {
        printf("我是父进程，pid = %d, child pid = %d\n", getpid(), pid);
    }

    return 0;
}
```

### 2. `fork()` 后父子执行不同逻辑

```c
pid_t pid = fork();

if (pid == 0) {
    // 子进程逻辑
} else if (pid > 0) {
    // 父进程逻辑
}
```

### 3. 创建多个子进程

```c
for (int i = 0; i < 3; i++) {
    pid_t pid = fork();
    if (pid == 0) {
        printf("我是第 %d 次创建出来的子进程\n", i);
        break;
    }
}
```

注意：

- 是否 `break`，会直接影响后续是否继续裂变产生更多进程

------

## 六、 `fork()` 的常见考点与易错点

### 1. 调用一次，返回两次

这是最核心、最基础的考点。

### 2. `fork()` 后代码执行次数翻倍

每执行一次 `fork()`：

- 当前“活着”的每个进程，都会再产生一个子进程
- 因此进程数会翻倍

如果有 `n` 次无条件 `fork()`，最终进程总数通常是：

```c
2^n
```

新增子进程数是：

```c
2^n - 1
```

### 3. 不要只看代码顺序，要看“在哪个进程里执行”

例如：

```c
fork();
printf("a\n");
```

这里 `printf` 是父子都会执行，所以是两次。

### 4. 输出顺序不确定，但输出总量可确定

考试或做题时要区分：

- **输出顺序**：通常不确定
- **输出总次数**：通常可精确计算

### 5. `\n`、无 `\n`、`fflush()` 的影响很大

这是 `fork()+printf()` 题最容易错的地方：

- 有 `\n`：通常立即刷新
- 无 `\n`：可能留在缓冲区，被 `fork()` 复制
- `fflush(stdout)`：强制清空缓冲区

### 6. 循环里 `fork()` 会产生“裂变效应”

例如循环执行 3 次，每次都 `fork()`：

- 第 1 次后：2 个进程
- 第 2 次后：4 个进程
- 第 3 次后：8 个进程

所以最终执行后续代码的进程数通常是 8 个

------

## 七、 学习 `fork()` 总结

### 1. 核心结论

- `fork()`：创建子进程
- 调用一次，返回两次
- `fork()` 后父子进程都继续执行后续代码
- 每执行一次无条件 `fork()`，进程数翻倍
- 父子进程变量彼此独立，底层依赖写时拷贝
- `fork()` 与 `wait()`、`exec()`、`exit()` / `_exit()` 经常联合考察
- `printf` 缓冲区问题是 `fork()` 题目的重点和难点

### 2. 做题口诀

- **先数进程，再看输出**
- **先看 `fork()` 前后，再看缓冲是否刷新**
- **有换行看次数，无换行防复制**
- **先输出后 `fork()`，最容易翻车**

### 3. 需要掌握的知识点

学完 `fork()` 后，你至少应该能独立判断：

- 某段代码最终会产生几个进程
- 父子进程分别执行哪些语句
- 最终会输出多少次
- 僵尸进程和孤儿进程是怎么来的
- 为什么 `fork()` 后常接 `exec()` 和 `wait()`
- 为什么有时要用 `_exit()` 而不是 `exit()`

------



# 进程管理与控制

## 一、 进程创建：`fork()`

### 1. 核心概念

- **执行流**：`fork()` 被调用一次，但会**返回两次**。从 `fork()` 返回之后，父子进程都会继续执行 `fork()` 后面的代码，并且都具备独立运行资格，由系统调度器决定谁先运行。
- **逻辑效果**：子进程会得到父进程当前进程空间的一份副本（包括代码段、数据段、BSS 段、堆、栈、全局变量、局部变量等）。

### 2. 内存机制：写时拷贝 (Copy-On-Write, COW)

- **传统机制的缺陷**：早期的 Unix 会把父进程的所有内存完全复制一份给子进程，极其低效。
- **现代 Linux 优化**：
  1. `fork` 瞬间，内核为子进程创建进程控制块 (PCB)，并复制父进程的页表。
  2. 此时，父子进程先**共享同一份物理内存页**，并且这块内存被标记为“只读”。
  3. 只有当父或子进程试图“修改”某个变量时（触发缺页中断），操作系统才会真正在物理内存中复制出那一页，让修改者在副本上操作。
  4. **结论**：如果只读不写，父子进程共享内存，`fork` 速度极快；各自修改变量时互不影响。

### 3. 返回值规则

通过 `fork()` 的返回值可以区分当前代码是在父进程还是子进程中运行：

- **返回 `0`**：当前在子进程中。
- **返回 `> 0`**：当前在父进程中，返回的值是**子进程的 PID**。
- **返回 `< 0`**：创建子进程失败（常见原因：系统进程数达到上限、内存不足等）。

### 4. 孤儿进程 (Orphan Process)

- 如果父进程先于子进程结束，子进程就会失去父进程，成为“孤儿进程”。
- Ubuntu 等 Linux 系统中，系统的 `init` 进程（通常是 PID 为 1 的 `systemd`）会自动“收养”孤儿进程并在其结束时回收资源。

------

## 二、 程序替换：`execl()`

### 1. 核心概念

- **作用**：用“一个新程序”替换“当前进程”的内存空间内容。
- **执行逻辑**：`execl()` 调用成功后，当前代码后面的语句就不会再执行了，因为当前进程的实体已经被替换成了另一个程序。只有在执行失败时，才会继续往下执行并返回错误。

### 2. 函数原型与参数

```c
int execl(const char *path, const char *arg, ..., NULL);
```

- `path`：要执行的程序文件路径（例如 `"./test6_calculate"`）。
- `arg` 及后续参数：传递给新程序的命令行参数（即新程序的 `argv[0]`, `argv[1]`...）。
  - **⚠️ 核心注意**：第二个参数通常作为新程序的 `argv[0]`，按惯例应写成**程序名或程序路径本身**，以保证和 Shell 调用习惯一致。
- `NULL`：最后一个参数**必须**是 `NULL`，用于标志参数列表的结束。

**正确调用示例**：

```c
// 相当于在 shell 中执行: ./test6_calculate 8 7
execl("./test6_calculate", "./test6_calculate", "8", "7", (void *)0);
```

------

## 三、 进程终止：`exit` 族函数

下面三种结束进程的方式行为并不完全一致：

### 1. `exit(EXIT_SUCCESS)`

- **归属**：标准 C 库函数。
- **特点**：“正常终止进程”，会先做一系列善后工作。
- **清理流程**：
  1. 调用通过 `atexit()` 注册的退出处理函数。
  2. **刷新并关闭标准 I/O 缓冲区**（例如 `printf` 没有 `\n` 时，内容也会被强制输出）。
  3. 执行 C 运行库层面的清理工作。

### 2. `_exit(EXIT_SUCCESS)`

- **归属**：POSIX 系统调用接口（常用于 Linux/Unix）。
- **特点**：“立刻结束进程”，不做 C 库层面的善后。
- **行为**：不调用 `atexit()` 注册的函数，**不刷新 stdio 缓冲区**，直接交还控制权给内核。
- **场景**：常用于 `fork()` 后子进程执行失败需要马上退出，且避免把从父进程继承来的缓冲区再次刷新一遍。

### 3. `_Exit(EXIT_SUCCESS)`

- **归属**：标准 C (C99) 提供的函数。
- **特点**：语义与 `_exit()` 几乎一致，是标准 C 版本的“立刻退出”。

------

## 四、 进程等待与回收

子进程结束后会暂时变成“僵尸进程”，等待父进程读取其退出状态。如果父进程不回收，会造成资源泄露。

### 1. 阻塞等待：`wait()`

```c
pid_t wait(int *status);
```

- **核心作用**：父进程暂停执行，等待**任意一个**子进程结束，并回收它。
- **参数**：如果传入 `NULL`，表示只回收资源，不关心子进程的退出状态。
- **返回值**：成功返回被回收的子进程 PID，失败返回 `-1`。

### 2. 精确与非阻塞等待：`waitpid()`

```c
pid_t waitpid(pid_t pid, int *wstatus, int options);
```

- **参数 `pid`**：
  - `> 0`：等待指定 PID 的子进程。
  - `-1`：等待任意子进程（同 `wait()`）。
- **参数 `options`**：
  - `0`：默认**阻塞等待**。
  - `WNOHANG`：**非阻塞等待**。若子进程尚未结束，`waitpid` 会立即返回 `0`，不会卡住父进程。
- **返回值**：
  - `> 0`：成功回收，返回结束的子进程 PID。
  - `0`：设置了 `WNOHANG` 且子进程还在运行。
  - `-1`：调用出错（如 PID 不合法或已被回收）。

### 3. 经典模式：轮询 + 非阻塞等待

父进程不想被卡死，又想回收子进程时，常结合 `sleep()` 进行轮询：

```c
for(int i=0; i<60; i++){
    pid_t ret = waitpid(fork_ret, NULL, WNOHANG);
    if(ret > 0) {
        // 成功回收到子进程
        break; 
    } else if (ret == 0) {
        // 子进程还在运行，父进程可以做别的事
    } else {
        // 出错
        break;
    }
    sleep(1); // 避免 CPU 空转
}
```

------

## 五、 进程组与会话管理

### 1. 进程组 (Process Group)

- **核心概念**：把若干个相关进程归为一组，方便统一管理（如 Shell 将前台任务放进同一进程组，方便用 Ctrl+C 发送信号给整个组）。
- **`getpgrp()`**：获取当前进程所属的进程组 ID。该 ID 通常等于**进程组组长的 PID**。
- **`setpgid(pid, pgid)`**：设置进程组。
  - `setpgid(0, 0)`：让当前进程创建一个新的进程组，并成为组长（此时进程的 pgrp == 自己的 pid）。

### 2. 会话 (Session)

- **核心概念**：一个登录终端下的一组相关进程的组织单位。通常你在终端启动的程序及其子进程默认都在同一个会话中。
- **`getsid(0)`**：获取当前进程所属的会话 ID (SID)。很多时候 SID 等于启动该会话的“会话首进程”的 PID。

### 3. 创建新会话：`setsid()` (守护进程核心)

- **作用**：创建一个新会话，让当前进程脱离原来的控制终端，成为：
  1. 新会话的会话首进程 (Session Leader)。
  2. 新进程组的组长进程 (Process Group Leader)。
- **限制前提**：**调用进程不能是当前的进程组组长**。
- **经典用法**：先 `fork()`，父进程退出，在子进程中调用 `setsid()`。这样子进程不仅在后台运行，而且在“会话层面”与原本的 Shell 彻底剥离，即使终端关闭也不会受影响。

------



# 管道通信与文件操作

## 一、 匿名管道 (Anonymous Pipe)

#### 1. 核心概念
* **本质**：内核中的一块缓冲区，没有文件系统中的实体名字（不会出现在目录树中）。
* **访问方式**：只能通过创建时返回的**文件描述符 (File Descriptor)** 进行访问。
* **生命周期**：通常与进程和文件描述符绑定，进程结束或相关 fd 关闭后释放。
* **特点**：
  1. **半双工（单向）通信**：一条管道只能实现单向数据流动（要么 A 写 B 读，要么 B 写 A 读）。
  2. **亲缘关系**：只能用于有亲缘关系的进程间通信（最常见的是父子进程，通过 `fork()` 继承共享）。

### 2. 创建管道：`pipe()`

```c
int pipe(int pipefd[2]);
```

- **功能**：直接在内核中创建一条管道，并返回两个文件描述符。
- **参数**：
  - `pipefd[0]`：管道的**读端**。
  - `pipefd[1]`：管道的**写端**。

### 3. 经典使用场景：`pipe()` + `fork()`

父子进程通信的黄金组合。`fork()` 之后，子进程会复制父进程的文件描述符表，此时父子进程同时持有同一条管道的读写端。

**⚠️ 最佳实践：关闭不用的端口**

在单向通信中（例如子进程写，父进程读），必须关闭各自不使用的那一端（子进程 `close(fd[0])`，父进程 `close(fd[1])`）。

- **原因 1**：节省宝贵的文件描述符资源。
- **原因 2**：明确当前进程职责，防止代码层面的误操作。
- **原因 3（最重要）**：防止阻塞死锁。如果不关闭，`read()` 或 `write()` 可能会因为“对端仍然被（包括自己在内的）某个进程持有”而持续阻塞等待，迟迟不返回 EOF 或引发 SIGPIPE。

### 4. 双向通信的实现

由于单条匿名管道是半双工的，如果要实现父子进程的双向对话（A 发给 B，B 也能回给 A），**必须创建两条管道**。

- **管道1**：`child_to_father_arr`（子进程用写端 `[1]`，父进程用读端 `[0]`）
- **管道2**：`father_to_child_arr`（父进程用写端 `[1]`，子进程用读端 `[0]`）
- **注意事项**：此时每个进程手里会有 4 个文件描述符，务必精准关闭每个进程不需要的 2 个端口。	

### 5. 单向通信的数据流示意

当使用一条匿名管道进行父子进程单向通信时，通常的数据流如下：

```
子进程 ----写入----> [ 管道缓冲区 ] ----读取----> 父进程
             fd[1]                      fd[0]
```

或者反过来：

```
父进程 ----写入----> [ 管道缓冲区 ] ----读取----> 子进程
             fd[1]                      fd[0]
```

### 6. 匿名管道的局限性

匿名管道虽然简单高效，但也存在明显限制：

1. **没有路径名**，无法被无关进程“重新打开”。
2. **只能依赖继承**，因此通常局限于父子进程或兄弟进程之间使用。
3. **默认按字节流处理数据**，没有“消息边界”的概念，读写双方需要自行约定格式。
4. **半双工**，复杂通信时要额外建立第二条管道。

------

## 二、 命名管道 (Named Pipe / FIFO)

### 1. 核心概念

- **本质**：一种特殊文件，数据以先进先出 (First In First Out) 的方式传输，常用于“一个写、另一个读”的 IPC。
- **与普通文件对比**：普通文件用于持久化保存数据到磁盘，而 FIFO 主要用于实时通信。
- **可见性**：在文件系统中存在实体路径，通过 `ls -l` 查看时，文件类型标识为 `p`（例如：`prw-r--r--`）。

### 2. 创建命名管道：`mkfifo()`

```
int mkfifo(const char *pathname, mode_t mode);
```

- **参数**：
  - `pathname`：要创建的 FIFO 文件路径（如 `"2.pipe"`）。
  - `mode`：权限掩码（如 `0666`）。**注意**：最终生成的文件权限会受到执行进程的 `umask` 影响（最终权限 = `mode & ~umask`）。
- **返回值**：成功返回 `0`；失败返回 `-1` 并设置 `errno`。
- **常见失败原因**：
  1. 同名文件已存在（`EEXIST`）。
  2. 对所在目录没有写权限。
  3. 路径非法或父目录不存在。

### 3. 使用场景与优势

创建后，**任意无亲缘关系的进程**只要有权限，都可以通过标准的 `open("2.pipe", O_RDONLY)` 或 `open("2.pipe", O_WRONLY)` 来打开它进行双向或单向通信，只要 FIFO 文件还在，就能持续作为通信桥梁。

### 4. 命名管道与匿名管道的区别

| 对比项               | 匿名管道 `pipe()`          | 命名管道 `mkfifo()`              |
| -------------------- | -------------------------- | -------------------------------- |
| 是否有文件名         | 没有                       | 有                               |
| 是否出现在文件系统中 | 不会                       | 会                               |
| 适用进程关系         | 有亲缘关系                 | 可无亲缘关系                     |
| 创建方式             | 内核直接创建               | 先在文件系统中创建特殊文件       |
| 生命周期             | 通常随进程和 fd 结束而结束 | 只要 FIFO 文件还在，就可继续使用 |

### 5. FIFO 的典型使用方式

命名管道创建完成后，通常通过 `open()` 打开，再配合 `read()`、`write()` 完成通信。

示意如下：

```
int fd = open("2.pipe", O_WRONLY);
write(fd, msg, strlen(msg));
close(fd);
```

或：

```
int fd = open("2.pipe", O_RDONLY);
read(fd, buf, sizeof(buf));
close(fd);
```

### 6. 查看 FIFO 文件

使用如下命令可以查看命名管道文件：

```
ls -l
```

若结果中文件类型首字符为 `p`，则说明它是一个 FIFO 文件，例如：

```
prw-r--r-- 1 user user 0 Mar 24 2.pipe
```

其中开头的 `p` 就表示 **pipe/FIFO 类型文件**。

------

## 三、 文件删除底层机制：`unlink()`

### 1. 核心机制：什么是“删除”？

在 Linux/Unix 文件系统中，**文件名**和**文件内容（inode 数据块）**是分离的。目录本质上保存的是 `文件名 -> inode` 的映射（硬链接）。

- `unlink()` 的字面意思是“取消链接”。
- **它的本质不是立刻擦除磁盘上的物理数据**，而是删除指定文件名所在的目录项，让这个名字和 inode 之间的硬链接断开。

### 2. 函数说明

```
int unlink(const char *pathname);
```

- **适用对象**：普通文件、命名管道文件（FIFO）、符号链接（软链接）。
- **限制**：**不能直接删除目录**。删除目录必须使用 `rmdir()`。成功返回 `0`，失败返回 `-1` 并设置 `errno`。

### 3. Linux 延迟回收机制（重点）

文件的数据究竟什么时候才会被真正从磁盘释放？必须**同时满足**以下两个条件：

1. **硬链接数为 0**：没有任何文件名再指向这个 inode（即被 `unlink` 彻底断开了映射）。
2. **打开引用计数为 0**：当前系统内**没有任何进程**正打开着这个文件。

**💡 关键特性**：

如果你 `unlink` 了一个文件，但此时某个进程仍在使用 `open()` 返回的 fd 读写它，这个文件的数据**并不会立即丢失**。该进程依然可以正常操作，直到它调用 `close()` 关闭该文件，系统内核才会真正回收这块存储空间。这为创建“进程退出即自动销毁的临时文件”提供了完美的底层支持。

### 4. `unlink()` 的常见使用场景

1. 删除普通文件。
2. 删除不再需要的 FIFO 文件。
3. 删除临时文件名，但保留当前进程对该文件的读写能力。
4. 配合 `open()` 实现“匿名临时文件”效果。

### 5. `unlink()` 的常见失败原因

1. 目标文件不存在。
2. 没有目录写权限。
3. 传入路径是目录。
4. 路径名非法。
5. 程序没有正确传入命令行参数，例如误用 `argv[1]` 时未检查 `argc`。

### 6. 一个很容易忽略的点

`unlink()` 删除的是“名字”，不是“文件内容对象本身”。

这意味着：

- 一个 inode 可以被多个文件名（硬链接）引用。
- 删除其中一个名字，不代表数据一定被销毁。
- 只有当**名字全没了**，并且**没有进程再打开它**，内核才会真正清理资源。

------

# pthread 核心函数

> POSIX Threads（pthread）是 POSIX 标准定义的线程接口，广泛用于 Unix/Linux 系统。
> 编译时需链接 `-lpthread`，如：`gcc -o app app.c -lpthread`。

---

## 一、线程管理函数 (Thread Management)

### 1. `pthread_create` — 创建线程

#### 函数原型

```c
#include <pthread.h>

int pthread_create(
    pthread_t            *thread,
    const pthread_attr_t *attr,
    void                 *(*start_routine)(void *),
    void                 *arg
);
```

#### 作用

创建一个新线程，新线程从 `start_routine` 函数处开始执行，`arg` 作为唯一参数传入。调用成功后，新线程立即处于可运行状态，与调用线程并发执行，具体调度顺序由操作系统决定。

#### 参数详解

| 参数            | 类型                     | 说明                                                         |
| --------------- | ------------------------ | ------------------------------------------------------------ |
| `thread`        | `pthread_t *`            | **输出参数**，成功后存储新线程的线程 ID（TID）               |
| `attr`          | `const pthread_attr_t *` | 线程属性对象，传 `NULL` 使用默认属性（joinable、默认栈大小、继承调度策略） |
| `start_routine` | `void *(*)(void *)`      | 线程入口函数，签名必须为 `void *func(void *)`                |
| `arg`           | `void *`                 | 传给 `start_routine` 的参数，可传结构体指针以携带多个值；不需要时传 `NULL` |

#### 返回值

- **`0`**：成功
- **非零错误码**（**不**通过 `errno` 返回）：
  - `EAGAIN`：系统资源不足，无法创建更多线程（超过系统或进程线程数上限）
  - `EINVAL`：`attr` 参数无效
  - `EPERM`：没有权限设置 `attr` 中指定的调度策略或优先级

#### 使用要点

1. **`arg` 的生命周期**：`arg` 指向的内存必须在线程使用期间保持有效。若传递栈上局部变量地址，当主线程栈帧销毁后将造成悬空指针。推荐使用堆分配（`malloc`）或全局/静态变量。
2. **多线程传参**：向多个线程分别传递不同参数时，必须为每个线程分配**独立**的参数结构体，绝不能让多个线程共用同一块内存并在创建后立即修改（典型竞态条件）。
3. **线程 ID 比较**：`pthread_t` 是不透明类型，比较两个线程 ID 应使用 `pthread_equal(t1, t2)`，而非直接 `==`。
4. **默认可汇合**：默认属性下线程是**可汇合的**（joinable），必须被 `pthread_join` 等待或事先设为分离状态，否则线程终止后资源无法回收（类似僵尸进程）。
5. **错误码直接返回**：pthread 函数错误码作为返回值返回，而非设置 `errno`。

#### 示例代码

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { int id; double value; } ThreadArg;

void *worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    printf("线程 %d: value=%.2f\n", a->id, a->value);
    free(a);            // 在线程内释放堆内存，避免泄漏
    return NULL;
}

int main(void) {
    pthread_t tid;
    ThreadArg *arg = malloc(sizeof(ThreadArg));
    arg->id = 1; arg->value = 3.14;

    int ret = pthread_create(&tid, NULL, worker, arg);
    if (ret != 0) { fprintf(stderr, "创建失败: %d\n", ret); return 1; }
    pthread_join(tid, NULL);
    return 0;
}
```

---

### 2. `pthread_exit` — 终止当前线程

#### 函数原型

```c
void pthread_exit(void *retval);
```

#### 作用

终止调用线程，并将 `retval` 作为退出值传递给等待该线程的 `pthread_join`。该函数不返回，调用后线程立即终止。

#### 参数详解

| 参数     | 类型     | 说明                                                         |
| -------- | -------- | ------------------------------------------------------------ |
| `retval` | `void *` | 线程退出状态值，可被 `pthread_join` 的第二个参数接收。**不能指向线程栈上的局部变量** |

#### 返回值

无返回值（函数不返回）。

#### 使用要点

1. **vs `return`**：线程入口函数 `return retval` 与 `pthread_exit(retval)` 效果等价，但 `pthread_exit` 可在**任意调用深度**终止线程（类似 `exit()` 对进程的作用），而 `return` 只能在入口函数层级使用。
2. **主线程的特殊语义**：主线程若调用 `pthread_exit`，主线程退出，但进程**不会**立即终止，其他线程继续运行直至全部结束；若主线程执行 `return` 或 `exit()`，则**整个进程（含所有线程）立即终止**。
3. **触发清理处理程序**：`pthread_exit` 会触发 `pthread_cleanup_push` 注册的清理函数（LIFO 顺序），以及线程特定数据（TSD）的析构函数。
4. **`retval` 不能指向局部变量**：线程退出后其栈帧被销毁，若 `retval` 指向栈数据，`pthread_join` 读取时将产生悬空指针，行为未定义。应使用堆分配或全局/静态存储。

#### 示例代码

```c
void *worker(void *arg) {
    int *result = malloc(sizeof(int));
    *result = 42;
    pthread_exit(result);   // 等效于 return result;
}

int main(void) {
    pthread_t tid; void *retval;
    pthread_create(&tid, NULL, worker, NULL);
    pthread_join(tid, &retval);
    printf("退出值: %d\n", *(int *)retval);
    free(retval);
    return 0;
}
```

---

### 3. `pthread_join` — 等待线程结束

#### 函数原型

```c
int pthread_join(pthread_t thread, void **retval);
```

#### 作用

阻塞调用线程，直到目标线程 `thread` 终止。同时**回收目标线程的资源**，并可获取其退出值。



#### 参数详解

| 参数     | 类型        | 说明                                                         |
| -------- | ----------- | ------------------------------------------------------------ |
| `thread` | `pthread_t` | 要等待的目标线程 ID                                          |
| `retval` | `void **`   | **输出参数**，接收目标线程的退出值（即 `pthread_exit` 的参数或 `return` 的值）；不关心退出值时传 `NULL` |

#### 返回值

- **`0`**：成功
- **`EDEADLK`**：检测到死锁（如线程等待自身，或两线程互相等待）
- **`EINVAL`**：目标线程不可汇合（已处于分离状态）
- **`ESRCH`**：目标线程 ID 不存在

#### 使用要点

1. **资源回收义务**：对于 joinable 线程，`pthread_join` 是回收线程资源（线程栈、线程描述符等）的**唯一**标准途径。若不调用，线程终止后资源永久泄漏。
2. **一次性**：每个可汇合线程只能被 join 一次，重复 join 同一线程行为未定义（通常返回 `ESRCH`）。
3. **取消线程的退出值**：若目标线程被 `pthread_cancel` 取消，`*retval` 会被设置为宏常量 `PTHREAD_CANCELED`（通常为 `(void *)-1`），可用此判断线程是否被取消。
4. **替代方案**：若不需要等待线程结束，可在创建前设置分离属性或调用 `pthread_detach(tid)` 将线程设为分离状态，线程结束后自动回收资源，但不能再 join。
5. **阻塞特性**：`pthread_join` 是一个取消点，被阻塞时可响应取消请求。

#### 示例代码

```c
pthread_t tid;
void *retval;
pthread_create(&tid, NULL, worker, NULL);

int err = pthread_join(tid, &retval);
if (err == 0) {
    if (retval == PTHREAD_CANCELED)
        printf("线程被取消\n");
    else
        printf("线程退出值: %p\n", retval);
}
```

---

### 4. `pthread_cancel` — 请求取消线程

#### 函数原型

```c
int pthread_cancel(pthread_t thread);
```

#### 作用

向目标线程发送**取消请求**。目标线程不一定立即终止，何时响应取决于其**取消状态**（是否接受取消）和**取消类型**（延迟 or 异步）。

#### 参数详解

| 参数     | 类型        | 说明                |
| -------- | ----------- | ------------------- |
| `thread` | `pthread_t` | 要取消的目标线程 ID |

#### 返回值

- **`0`**：取消请求成功发送（**不代表线程已终止**）
- **`ESRCH`**：目标线程不存在

#### 相关控制函数

```c
// 设置取消状态（是否接受取消请求，默认启用）
int pthread_setcancelstate(int state, int *oldstate);
// state: PTHREAD_CANCEL_ENABLE（默认）| PTHREAD_CANCEL_DISABLE

// 设置取消类型（何时响应取消请求）
int pthread_setcanceltype(int type, int *oldtype);
// type: PTHREAD_CANCEL_DEFERRED（默认，仅在取消点处响应）
//     | PTHREAD_CANCEL_ASYNCHRONOUS（立即响应，极度危险）

// 手动插入一个取消点（在没有系统取消点时检测挂起的取消请求）
void pthread_testcancel(void);
```

#### 取消点（Cancellation Points）

POSIX 规定了若干函数为取消点，线程在这些函数阻塞期间会响应取消请求，常见取消点包括：

| 类别         | 函数                                                         |
| ------------ | ------------------------------------------------------------ |
| pthread 函数 | `pthread_join`、`pthread_cond_wait`、`pthread_cond_timedwait`、`pthread_testcancel` |
| 文件 I/O     | `read`、`write`、`open`、`close`、`select`、`poll`           |
| 时间         | `sleep`、`nanosleep`、`usleep`                               |
| 网络         | `accept`、`connect`、`recv`、`send`                          |

#### 使用要点

1. **异步取消极其危险**：`PTHREAD_CANCEL_ASYNCHRONOUS` 可在任意指令处取消，若在 `malloc`/`free`、互斥锁持有期间被取消，将导致资源泄漏、死锁或数据损坏。**几乎不应使用**。
2. **清理资源是必须的**：被取消的线程会**自动执行** `pthread_cleanup_push` 注册的清理函数，应将锁释放、内存释放等操作注册为清理处理程序。
3. **取消不等于 kill**：取消是协作式的（默认延迟），目标线程在取消点才真正终止，调用方无法强制立即终止目标线程。
4. **禁用取消的用法**：在持有互斥锁的关键区段，可先禁用取消 `pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, ...)` 保证临界区原子性，操作完成后再重新启用。

#### 示例代码

```c
void cleanup(void *arg) { printf("清理资源\n"); free(arg); }

void *worker(void *arg) {
    pthread_cleanup_push(cleanup, malloc(64));
    sleep(10);              // 此处是取消点，将在此响应取消请求
    pthread_cleanup_pop(1); // 1=执行清理函数并弹出
    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, worker, NULL);
    sleep(1);
    pthread_cancel(tid);    // 发送取消请求
    pthread_join(tid, NULL);
    return 0;
}
```

---

## 二、互斥锁函数 (Mutex Locks)

> 互斥锁（Mutex）用于保护共享资源，确保同一时刻只有一个线程进入临界区，防止数据竞争（Data Race）。

### 1. `pthread_mutex_init` — 初始化互斥锁

#### 函数原型

```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

// 静态初始化（仅适用于静态/全局 mutex，使用默认属性）
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

#### 作用

初始化 `mutex` 指向的互斥锁对象，使其进入**未锁定**状态，准备好被后续的 lock/unlock 操作使用。

#### 参数详解

| 参数    | 类型                          | 说明                                                  |
| ------- | ----------------------------- | ----------------------------------------------------- |
| `mutex` | `pthread_mutex_t *`           | 要初始化的互斥锁对象指针                              |
| `attr`  | `const pthread_mutexattr_t *` | 互斥锁属性，`NULL` 使用默认属性（普通类型、进程私有） |

#### 互斥锁类型（通过 attr 设置）

| 类型宏                     | 说明                                           |
| -------------------------- | ---------------------------------------------- |
| `PTHREAD_MUTEX_NORMAL`     | 默认类型，同一线程重复加锁会**死锁**           |
| `PTHREAD_MUTEX_ERRORCHECK` | 检错类型，重复加锁返回错误 `EDEADLK`           |
| `PTHREAD_MUTEX_RECURSIVE`  | 递归类型，同一线程可重复加锁（需对应次数解锁） |
| `PTHREAD_MUTEX_DEFAULT`    | 实现定义，通常等同于 `NORMAL`                  |

#### 返回值

- **`0`**：成功
- **`EINVAL`**：`attr` 无效
- **`ENOMEM`**：内存不足

#### 使用要点

1. **动态 vs 静态初始化**：全局/静态 mutex 可用宏 `PTHREAD_MUTEX_INITIALIZER` 初始化，无需调用 `init`/`destroy`。堆分配或栈上（局部）mutex 必须用 `pthread_mutex_init` 并配套调用 `pthread_mutex_destroy`。
2. **不能复制**：已初始化的 mutex 不能通过赋值或 `memcpy` 复制，行为未定义。
3. **不能重复初始化**：对已初始化（且未销毁）的 mutex 再次调用 `init`，行为未定义。
4. **属性对象生命周期**：`init` 调用后，attr 对象即可销毁，mutex 本身不引用 attr。


---

### 2. `pthread_mutex_lock` — 加锁（阻塞）

#### 函数原型

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
```

#### 作用

尝试对 `mutex` 加锁。若 mutex **未被锁定**，则立即加锁并返回；若 mutex **已被其他线程锁定**，则调用线程**阻塞**，直到 mutex 被解锁并由调用线程获取。

#### 参数详解

| 参数    | 类型                | 说明                   |
| ------- | ------------------- | ---------------------- |
| `mutex` | `pthread_mutex_t *` | 要加锁的互斥锁对象指针 |

#### 返回值

- **`0`**：成功获取锁
- **`EINVAL`**：mutex 未初始化，或调度优先级超界
- **`EDEADLK`**：（检错或递归类型）检测到当前线程重复加锁

#### 使用要点

1. **配对使用**：每次成功的 lock 必须对应一次 unlock，且必须是**同一个线程**执行 unlock（互斥锁不可跨线程解锁）。
2. **持锁时间最小化**：临界区应尽量短小，只保护必要的共享数据读写，避免在持锁期间进行 I/O、网络请求、耗时计算等操作。
3. **避免死锁**：多个 mutex 时，所有线程应以**相同顺序**加锁（锁排序策略）；或使用 `pthread_mutex_trylock` 配合退避策略。
4. **与条件变量配合**：条件变量的 `pthread_cond_wait` 会原子地释放 mutex 并等待，被唤醒时重新获取 mutex。

#### 示例代码

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void *increment(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&lock);
        counter++;                  // 临界区：保护共享变量
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
```

---

### 3. `pthread_mutex_trylock` — 尝试加锁（非阻塞）

#### 函数原型

```c
int pthread_mutex_trylock(pthread_mutex_t *mutex);
```

#### 作用

尝试对 `mutex` 加锁，与 `pthread_mutex_lock` 的区别在于：若 mutex **已被锁定**，**立即返回错误**而不阻塞。

#### 参数详解

| 参数    | 类型                | 说明                       |
| ------- | ------------------- | -------------------------- |
| `mutex` | `pthread_mutex_t *` | 要尝试加锁的互斥锁对象指针 |

#### 返回值

- **`0`**：成功获取锁
- **`EBUSY`**：mutex 当前已被锁定，未能获取（**最常见的非错误返回值**）
- **`EINVAL`**：mutex 未初始化
- **`EDEADLK`**：（检错类型）当前线程已持有该锁

#### 使用要点

1. **典型用途**：实现非阻塞尝试获取资源，在获取不到时执行替代逻辑（忙等、退避、处理其他任务），避免线程长时间阻塞。
2. **死锁检测**：配合多个 trylock 可实现死锁检测与退避：若任一锁获取失败，则释放已持有的所有锁后重试。
3. **轮询（Polling）模式**：`trylock` 配合循环可实现轮询，但需注意 CPU 占用，适当加入 `usleep` 退避。
4. **返回值必须检查**：调用后必须检查返回值，`EBUSY` 不是错误，而是"未获取锁"的正常通知。

#### 示例代码

```c
// 非阻塞尝试加锁，失败则执行其他工作
if (pthread_mutex_trylock(&lock) == 0) {
    // 成功获取锁，进入临界区
    do_critical_work();
    pthread_mutex_unlock(&lock);
} else {
    // 锁被占用，执行替代工作
    do_alternative_work();
}
```

---

### 4. `pthread_mutex_unlock` — 解锁

#### 函数原型

```c
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

#### 作用

释放调用线程持有的 `mutex` 锁，使其他等待该锁的线程之一得以获取锁并继续执行。

#### 参数详解

| 参数    | 类型                | 说明                   |
| ------- | ------------------- | ---------------------- |
| `mutex` | `pthread_mutex_t *` | 要解锁的互斥锁对象指针 |

#### 返回值

- **`0`**：成功解锁
- **`EINVAL`**：mutex 未初始化
- **`EPERM`**：（检错类型）当前线程不持有该锁

#### 使用要点

1. **所有权原则**：只有**持有锁的线程**才能解锁（普通类型 mutex 不检查所有权，但解锁非自己加的锁行为未定义）。
2. **异常路径也要解锁**：每个可能的代码路径（包括 `return` 和错误处理路径）都必须执行 unlock，否则造成永久死锁。可配合 `pthread_cleanup_push` 注册解锁函数。
3. **唤醒顺序不确定**：解锁后，操作系统决定哪个等待线程获得锁，POSIX 不保证 FIFO 顺序（依赖调度策略）。
4. **与条件变量**：在条件变量的典型模式中，`pthread_cond_signal`/`broadcast` 通常在持锁状态下调用，信号发出后解锁。

---

### 5. `pthread_mutex_destroy` — 销毁互斥锁

#### 函数原型

```c
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

#### 作用

销毁 mutex 对象，释放其关联的内部资源。销毁后的 mutex 在重新初始化之前不得使用。

#### 参数详解

| 参数    | 类型                | 说明                   |
| ------- | ------------------- | ---------------------- |
| `mutex` | `pthread_mutex_t *` | 要销毁的互斥锁对象指针 |

#### 返回值

- **`0`**：成功
- **`EBUSY`**：mutex 当前被锁定或有线程正在等待，不能销毁
- **`EINVAL`**：mutex 无效

#### 使用要点

1. **销毁前必须未锁定**：对处于锁定状态或有线程在等待的 mutex 调用 destroy，行为未定义（部分实现返回 `EBUSY`，部分实现崩溃）。
2. **与 init 配对**：动态初始化的 mutex 必须在不再需要时调用 destroy，静态初始化（`PTHREAD_MUTEX_INITIALIZER`）则不需要。
3. **销毁后置 NULL**：`destroy` 后建议将 mutex 指针或内容置为无效标记，防止误用。
4. **局部变量 mutex**：栈上的 mutex 在函数返回时会自动失效，但如果其他线程还在使用，这就是 bug，务必确保所有使用者已完成。

#### 完整示例（mutex 生命周期）

```c
pthread_mutex_t *create_mutex(void) {
    pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);   // attr 用完即可销毁
    return m;
}

void destroy_mutex(pthread_mutex_t *m) {
    pthread_mutex_destroy(m);
    free(m);
}
```

---

## 三、条件变量函数 (Condition Variables)

> 条件变量（Condition Variable）允许线程在某个条件不满足时**挂起等待**，直到其他线程改变条件并发出通知，从而实现线程间的**协调与同步**。
> 条件变量**必须与互斥锁配合使用**，用于保护条件的读写。

### 1. `pthread_cond_init` — 初始化条件变量

#### 函数原型

```c
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

// 静态初始化（静态/全局条件变量，使用默认属性）
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```

#### 作用

初始化条件变量 `cond`，使其准备好被 `pthread_cond_wait`/`signal`/`broadcast` 使用。

#### 参数详解

| 参数   | 类型                         | 说明                                                        |
| ------ | ---------------------------- | ----------------------------------------------------------- |
| `cond` | `pthread_cond_t *`           | 要初始化的条件变量对象指针                                  |
| `attr` | `const pthread_condattr_t *` | 条件变量属性，`NULL` 使用默认属性（进程私有、系统默认时钟） |

#### 返回值

- **`0`**：成功
- **`EINVAL`**：`attr` 无效
- **`ENOMEM`**：内存不足

#### 使用要点

与 mutex 类似：全局/静态可用 `PTHREAD_COND_INITIALIZER`；堆分配或局部必须用 `init`/`destroy` 配对；不能复制或重复初始化。


---

### 2. `pthread_cond_wait` — 等待条件变量（无超时）

#### 函数原型

```c
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
```

#### 作用

原子地**释放 mutex 并进入等待状态**，直到条件变量 `cond` 收到通知（signal 或 broadcast）为止。被唤醒后，**重新获取 mutex** 然后返回。

#### 参数详解

| 参数    | 类型                | 说明                                                   |
| ------- | ------------------- | ------------------------------------------------------ |
| `cond`  | `pthread_cond_t *`  | 要等待的条件变量                                       |
| `mutex` | `pthread_mutex_t *` | 与条件变量关联的互斥锁，**调用前必须已由当前线程持有** |

#### 返回值

- **`0`**：被唤醒并重新获取了 mutex（**注意：不代表条件一定已满足，存在虚假唤醒**）
- **`EINVAL`**：cond 或 mutex 无效，或不同线程使用了不同的 mutex 等待同一 cond

#### 核心工作机制

```
调用 pthread_cond_wait 的内部逻辑（原子操作）：
  1. 将当前线程放入 cond 的等待队列
  2. 释放 mutex（此步骤与步骤1原子完成，不存在竞态窗口）
  3. 当前线程进入睡眠，等待通知
  ──────── 其他线程修改条件并调用 signal/broadcast ────────
  4. 当前线程被唤醒，重新竞争获取 mutex
  5. 成功获取 mutex 后，pthread_cond_wait 返回
```

#### 使用要点

1. **虚假唤醒（Spurious Wakeup）**：`pthread_cond_wait` 可能在**没有任何线程发出通知**的情况下返回（这是 POSIX 标准允许的行为，源于 Linux 信号等机制）。因此，返回后**必须重新检查条件**，正确模式是将 wait 置于 `while` 循环中，而非 `if`。

2. **标准使用模式（等待者）**：

   ```c
   pthread_mutex_lock(&mutex);
   while (!condition_is_true())          // 用 while 而非 if！
       pthread_cond_wait(&cond, &mutex);
   // 条件已满足，在持锁状态下操作共享数据
   pthread_mutex_unlock(&mutex);
   ```

3. **mutex 必须预先持有**：调用前必须已持有 mutex，否则行为未定义。释放和等待是原子完成的，不存在"释放锁后但还未进入等待"的竞态窗口。

4. **是一个取消点**：等待期间可响应 `pthread_cancel`，被取消时会重新获取 mutex 后再执行取消动作。

#### 完整示例（生产者-消费者模型）

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
int data_ready = 0;
int data = 0;

// 消费者线程
void *consumer(void *arg) {
    pthread_mutex_lock(&mutex);
    while (!data_ready)                     // while 循环防止虚假唤醒
        pthread_cond_wait(&cond, &mutex);
    printf("消费数据: %d\n", data);
    data_ready = 0;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// 生产者线程
void *producer(void *arg) {
    pthread_mutex_lock(&mutex);
    data = 42;
    data_ready = 1;
    pthread_cond_signal(&cond);             // 通知消费者
    pthread_mutex_unlock(&mutex);
    return NULL;
}
```

---

### 3. `pthread_cond_timedwait` — 带超时的条件等待

#### 函数原型

```c
int pthread_cond_timedwait(pthread_cond_t        *cond,
                           pthread_mutex_t        *mutex,
                           const struct timespec  *abstime);
```

#### 作用

与 `pthread_cond_wait` 相同，但增加了**绝对超时时间**限制。若在 `abstime` 指定的时刻之前未收到通知，则超时返回。

#### 参数详解

| 参数      | 类型                      | 说明                                                         |
| --------- | ------------------------- | ------------------------------------------------------------ |
| `cond`    | `pthread_cond_t *`        | 要等待的条件变量                                             |
| `mutex`   | `pthread_mutex_t *`       | 关联的互斥锁，调用前必须已持有                               |
| `abstime` | `const struct timespec *` | **绝对超时时间**（从 Epoch 1970-01-01 00:00:00 UTC 起的秒和纳秒），而非相对时间 |

#### `struct timespec` 结构

```c
struct timespec {
    time_t tv_sec;   // 秒
    long   tv_nsec;  // 纳秒（0 ~ 999,999,999）
};
```

#### 返回值

- **`0`**：成功被唤醒（仍需用 while 循环检查条件，存在虚假唤醒）
- **`ETIMEDOUT`**：等待超时，条件变量未被通知，已重新获取 mutex 并返回
- **`EINVAL`**：参数无效（如 `abstime->tv_nsec` 超出范围）

#### 使用要点

1. **使用绝对时间（Absolute Time）**：`abstime` 是**绝对时间戳**，不是等待的时长。正确做法是先获取当前时间，再加上期望的等待时长：

   ```c
   struct timespec ts;
   clock_gettime(CLOCK_REALTIME, &ts);   // 获取当前时间
   ts.tv_sec += 5;                       // 加上 5 秒超时
   int ret = pthread_cond_timedwait(&cond, &mutex, &ts);
   ```

2. **纳秒进位处理**：当 `tv_nsec` 加法溢出 999,999,999 时，需手动进位到 `tv_sec`，否则 `EINVAL`。

3. **超时后仍需解锁**：超时返回 `ETIMEDOUT` 时，mutex 已被重新获取，仍需调用 `pthread_mutex_unlock`。

4. **时钟类型**：默认使用 `CLOCK_REALTIME`，若需要不受系统时间调整影响的超时，可通过 `pthread_condattr_setclock` 改为 `CLOCK_MONOTONIC`（需要属性对象支持）。

5. **超时也要检查条件**：`ETIMEDOUT` 后仍应检查条件是否已满足（竞态：信号在超时判断前发出），模板如下：

   ```c
   while (!condition && ret != ETIMEDOUT)
       ret = pthread_cond_timedwait(&cond, &mutex, &ts);
   if (!condition) { /* 超时且条件未满足，处理超时逻辑 */ }
   ```

---

### 4. `pthread_cond_signal` — 唤醒一个等待线程

#### 函数原型

```c
int pthread_cond_signal(pthread_cond_t *cond);
```

#### 作用

唤醒**至少一个**正在等待 `cond` 的线程。若没有线程在等待，则信号丢失（无记忆性）。

#### 参数详解

| 参数   | 类型               | 说明                 |
| ------ | ------------------ | -------------------- |
| `cond` | `pthread_cond_t *` | 要发送通知的条件变量 |

#### 返回值

- **`0`**：成功（无论是否有线程在等待）
- **`EINVAL`**：cond 未初始化

#### 使用要点

1. **唤醒"至少一个"**：POSIX 规定唤醒至少一个等待线程，实际上通常只唤醒一个，但不保证。适用于**资源只够一个线程使用**的场景（如生产一个消息通知一个消费者）。
2. **无记忆性**：若发出 signal 时无线程在等待，信号**丢失**，后来进入 wait 的线程不会被这个旧信号唤醒。这也是为什么要用 while 循环保护条件检查。
3. **持锁还是释放后 signal**：两种方式均合法，但**持锁状态下 signal** 更安全，可避免"信号丢失"竞态（等待线程还未进入 wait 就被 signal）。
4. **vs broadcast**：若多个线程等待同一条件，`signal` 只唤醒其中一个，其他继续等待；而 `broadcast` 唤醒所有。

---

### 5. `pthread_cond_broadcast` — 唤醒所有等待线程

#### 函数原型

```c
int pthread_cond_broadcast(pthread_cond_t *cond);
```

#### 作用

唤醒**所有**正在等待 `cond` 的线程。所有线程被唤醒后，依次竞争获取 mutex，各自重新检查条件。

#### 参数详解

| 参数   | 类型               | 说明                 |
| ------ | ------------------ | -------------------- |
| `cond` | `pthread_cond_t *` | 要广播通知的条件变量 |

#### 返回值

- **`0`**：成功（无论是否有线程在等待）
- **`EINVAL`**：cond 未初始化

#### 使用要点

1. **适用场景**：适用于条件变化可满足**多个**等待线程时（如共享资源空间充足、状态变更影响所有等待者、程序即将退出通知所有线程退出）。
2. **"惊群效应"（Thundering Herd）**：大量线程被唤醒后只有一个能实际获得资源，其他线程重新进入等待，造成不必要的上下文切换开销。在高并发场景中应权衡使用。
3. **比 signal 更保守、更安全**：当不确定等待线程数量或条件变更能满足多少个等待者时，使用 broadcast 是更安全的选择，可避免"漏唤醒"问题。
4. **退出通知模式**：程序关闭时，通常将退出标志置为 true 后 broadcast 所有等待线程，使其检测到退出条件后退出。

---

### 6. `pthread_cond_destroy` — 销毁条件变量

#### 函数原型

```c
int pthread_cond_destroy(pthread_cond_t *cond);
```

#### 作用

销毁条件变量 `cond`，释放其关联的内部资源。销毁后不得再使用，需重新初始化。

#### 参数详解

| 参数   | 类型               | 说明                     |
| ------ | ------------------ | ------------------------ |
| `cond` | `pthread_cond_t *` | 要销毁的条件变量对象指针 |

#### 返回值

- **`0`**：成功
- **`EBUSY`**：有线程正在等待该条件变量，不能销毁
- **`EINVAL`**：cond 无效

#### 使用要点

1. **销毁前确认无等待者**：必须确保没有线程正在 `pthread_cond_wait` 或 `pthread_cond_timedwait` 等待该 cond，否则行为未定义。
2. **与 init 配对**：动态初始化的 cond 必须在不再需要时调用 destroy；静态初始化（`PTHREAD_COND_INITIALIZER`）不需要调用 destroy。


---

## 四、线程清理处理程序 (Cleanup Handlers)

> 清理处理程序（Cleanup Handler）是注册到线程的回调函数，在线程**异常退出**（被取消）或**主动退出**（`pthread_exit`）时自动被调用，用于释放资源、解锁互斥锁等，防止资源泄漏。

### 1. `pthread_cleanup_push` — 注册清理处理程序

#### 函数原型

```c
void pthread_cleanup_push(void (*routine)(void *), void *arg);
```

> ⚠️ 注意：`pthread_cleanup_push` 在大多数实现中是一个**宏**，它会展开为一段包含 `{` 的代码块，因此必须与 `pthread_cleanup_pop` 在**同一词法作用域**内配对使用。

#### 作用

将清理函数 `routine` 和参数 `arg` 压入当前线程的清理处理程序**栈**（Stack）中。当线程因以下任一原因退出时，已注册的清理函数将按 **LIFO（后进先出）** 顺序自动执行：

- 调用 `pthread_exit`
- 响应 `pthread_cancel` 的取消请求（在取消点处）
- 调用 `pthread_cleanup_pop` 并传入非零参数

#### 参数详解

| 参数      | 类型               | 说明                                                      |
| --------- | ------------------ | --------------------------------------------------------- |
| `routine` | `void (*)(void *)` | 清理函数指针，签名为 `void func(void *)`                  |
| `arg`     | `void *`           | 传递给 `routine` 的参数（如互斥锁指针、分配的内存地址等） |

#### 返回值

无返回值（宏展开，无返回值语义）。

#### 使用要点

1. **必须与 `pthread_cleanup_pop` 配对**：`push`/`pop` 必须在同一函数的同一词法层次（同一 `{...}` 块）内成对出现，否则编译错误（因为宏展开含括号）。
2. **注册时机**：应在获取资源（加锁、`malloc`等）之后立即注册清理函数，确保资源在任何退出路径上都被释放。
3. **仅对 `pthread_exit` 和取消有效**：对普通 `return` 无效（`return` 不触发清理函数）。若要在 `return` 路径也执行清理，使用 `pthread_cleanup_pop(1)` 手动触发。
4. **嵌套注册**：可多次调用 `push` 注册多个清理函数，形成清理函数栈，按 LIFO 顺序执行（最后注册的最先执行）。

---

### 2. `pthread_cleanup_pop` — 执行并弹出清理处理程序

#### 函数原型

```c
void pthread_cleanup_pop(int execute);
```

> ⚠️ 同样是**宏**，展开含 `}`，必须与 `pthread_cleanup_push` 在同一词法作用域内配对。

#### 作用

从当前线程的清理处理程序栈中弹出最近注册的清理函数。若 `execute` 非零，则在弹出前**立即执行**该清理函数；若为零，则仅弹出不执行。

#### 参数详解

| 参数      | 类型  | 说明                                                         |
| --------- | ----- | ------------------------------------------------------------ |
| `execute` | `int` | 非零（通常为 `1`）：执行清理函数后弹出；零（`0`）：仅弹出不执行 |

#### 返回值

无返回值。

#### 使用要点

1. **`execute=1` vs `execute=0`**：
   - `execute=1`：正常结束时主动触发清理（如 `return` 前），确保资源被释放
   - `execute=0`：不需要清理时仅弹出（如错误分支中资源未分配成功）
2. **与正常返回配合**：在线程函数正常 `return` 之前调用 `pthread_cleanup_pop(1)` 可以复用清理逻辑，避免重复代码。
3. **宏配对约束**：`push` 和 `pop` 展开后分别是 `{` 和 `}`，之间的代码是一个代码块，不能跨函数或条件分支不对称使用。

#### 完整示例（push/pop 典型用法）

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// 清理函数：用于解锁互斥锁
void unlock_mutex(void *arg) {
    printf("[清理] 解锁 mutex\n");
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

// 清理函数：用于释放堆内存
void free_buffer(void *arg) {
    printf("[清理] 释放内存\n");
    free(arg);
}

void *worker(void *arg) {
    char *buf = malloc(256);
    if (!buf) return NULL;

    pthread_cleanup_push(free_buffer, buf);         // 注册：最后弹出，最先执行（LIFO）

    pthread_mutex_lock(&g_mutex);
    pthread_cleanup_push(unlock_mutex, &g_mutex);   // 注册：先弹出，最先执行

    // ------ 临界区开始 ------
    // 模拟工作，此处可能被取消
    // 若调用 pthread_exit 或被 cancel，两个清理函数都会自动执行
    // ------ 临界区结束 ------

    pthread_cleanup_pop(1);  // 执行 unlock_mutex 并弹出（对应第二个 push）
    pthread_cleanup_pop(1);  // 执行 free_buffer 并弹出（对应第一个 push）

    return NULL;
}
```

#### 执行顺序图示

```
注册顺序：push(free_buffer) → push(unlock_mutex)
执行顺序（LIFO）：unlock_mutex → free_buffer
```

---

## 五、函数一览表

### 5.1 线程管理函数

| 函数             | 作用                   | 关键入参                                                     | 返回值要点                                                   |
| ---------------- | ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `pthread_create` | 创建新线程             | `thread`（输出TID）、`attr`（属性/NULL）、`start_routine`（入口函数）、`arg`（参数） | `0` 成功；`EAGAIN` 资源不足；`EINVAL` attr无效               |
| `pthread_exit`   | 终止当前线程           | `retval`（退出值，不可指向栈变量）                           | 无返回（不返回）                                             |
| `pthread_join`   | 等待线程结束并回收资源 | `thread`（目标线程ID）、`retval`（输出：退出值/NULL）        | `0` 成功；`EDEADLK` 死锁；`EINVAL` 不可汇合；`ESRCH` 线程不存在 |
| `pthread_cancel` | 发送取消请求           | `thread`（目标线程ID）                                       | `0` 请求已发送；`ESRCH` 线程不存在                           |

### 5.2 互斥锁函数

| 函数                    | 作用               | 关键入参                               | 返回值要点                                                   |
| ----------------------- | ------------------ | -------------------------------------- | ------------------------------------------------------------ |
| `pthread_mutex_init`    | 初始化互斥锁       | `mutex`（锁指针）、`attr`（属性/NULL） | `0` 成功；`EINVAL` attr无效；`ENOMEM` 内存不足               |
| `pthread_mutex_lock`    | 加锁（阻塞等待）   | `mutex`（锁指针）                      | `0` 成功获锁；`EINVAL` 未初始化；`EDEADLK` 重复加锁（检错模式） |
| `pthread_mutex_trylock` | 尝试加锁（非阻塞） | `mutex`（锁指针）                      | `0` 成功获锁；`EBUSY` 锁已被占用（正常，不是错误）           |
| `pthread_mutex_unlock`  | 解锁               | `mutex`（锁指针）                      | `0` 成功；`EINVAL` 未初始化；`EPERM` 非锁持有者解锁（检错模式） |
| `pthread_mutex_destroy` | 销毁互斥锁         | `mutex`（锁指针）                      | `0` 成功；`EBUSY` 锁仍被持有或有线程等待；`EINVAL` 无效      |

### 5.3 条件变量函数

| 函数                     | 作用                           | 关键入参                                         | 返回值要点                                             |
| ------------------------ | ------------------------------ | ------------------------------------------------ | ------------------------------------------------------ |
| `pthread_cond_init`      | 初始化条件变量                 | `cond`（cond指针）、`attr`（属性/NULL）          | `0` 成功；`EINVAL` attr无效；`ENOMEM` 内存不足         |
| `pthread_cond_wait`      | 原子释放锁并等待通知（无超时） | `cond`（cond指针）、`mutex`（必须预先持有）      | `0` 被唤醒（需 while 再检查条件）；`EINVAL` 参数无效   |
| `pthread_cond_timedwait` | 带超时的条件等待               | `cond`、`mutex`、`abstime`（**绝对**超时时间戳） | `0` 被唤醒；`ETIMEDOUT` 超时；`EINVAL` 参数无效        |
| `pthread_cond_signal`    | 唤醒至少一个等待线程           | `cond`（cond指针）                               | `0` 成功（即使无线程在等待也返回0）；`EINVAL` 未初始化 |
| `pthread_cond_broadcast` | 唤醒所有等待线程               | `cond`（cond指针）                               | `0` 成功（即使无线程在等待也返回0）；`EINVAL` 未初始化 |
| `pthread_cond_destroy`   | 销毁条件变量                   | `cond`（cond指针）                               | `0` 成功；`EBUSY` 有线程在等待；`EINVAL` 无效          |

### 5.4 线程清理处理程序

| 函数/宏                | 作用                         | 关键入参                                          | 返回值要点         |
| ---------------------- | ---------------------------- | ------------------------------------------------- | ------------------ |
| `pthread_cleanup_push` | 向清理栈压入清理函数         | `routine`（清理函数）、`arg`（传给routine的参数） | 无（宏，无返回值） |
| `pthread_cleanup_pop`  | 弹出（并可执行）栈顶清理函数 | `execute`（非零=执行后弹出，零=仅弹出）           | 无（宏，无返回值） |

### 5.5 关键规则速记

| 场景                          | 正确做法                                                     |
| ----------------------------- | ------------------------------------------------------------ |
| 条件变量等待                  | **while 循环**检查条件，防虚假唤醒                           |
| 线程退出值                    | 不能指向线程栈局部变量，用堆或全局存储                       |
| joinable 线程                 | 必须 `pthread_join` 或 `pthread_detach`，否则资源泄漏        |
| 多锁加锁顺序                  | 所有线程保持相同加锁顺序，防死锁                             |
| `pthread_cond_timedwait` 超时 | `abstime` 是绝对时间戳，用 `clock_gettime` + 偏移量计算      |
| `pthread_cancel` 清理         | 配合 `pthread_cleanup_push` 注册解锁/释放函数                |
| 静态初始化                    | 全局/静态 mutex/cond 用 `_INITIALIZER` 宏，无需 `init`/`destroy` |
| `pthread_cleanup_push/pop`    | 必须在同一词法作用域内成对出现（宏展开含括号）               |
