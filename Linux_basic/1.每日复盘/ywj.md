# Linux基础重要提醒

# shell、vim、GNU

## 常用shell命令：

|           |                              |                                                              |
| --------- | ---------------------------- | ------------------------------------------------------------ |
| man       | 1-命令  2-系统调用  3-库函数 |                                                              |
| pwd       | 查看当前目录                 |                                                              |
| cd        | 进入某个目录                 | 直接cd是进入家目录                                           |
| mkdir     | 创建目录                     |                                                              |
| rmdir     | 删除目录                     |                                                              |
| ls        | 显示目录下的内容             | ll  ll -h  ls -al  ls -alh                                   |
| cp        | 复制文件                     |                                                              |
| mv        | 移动文件                     | 可以用来文件重命名                                           |
| rm        | 删除文件                     | rm  rm -r  rm -f                                             |
| chmod     | 设置权限                     |                                                              |
| touch     | 创建空文件                   |                                                              |
| echo      | 创建文件 >覆盖 >>追加        | echo "111">1.txt                                             |
| vim 1.txt | 创建并在vim打开              |                                                              |
| find      | 查找文件                     | find 路径 -name "文件名"                                     |
| cat       | 打印文件内容                 |                                                              |
| tail      | 看尾部内容                   | tail -n 100 -f error.log                                     |
| head      | 看头部内容                   |                                                              |
| less      | 看部分内容                   | 按需加载，全向导航                                           |
| grep      | 搜索内容或文件               | grep -nE "搜索内容" 文件名  <br />grep -nE -C 10 "搜索内容" 文件名（显示上下十行） |
| ifconfig  | 查看机器ip                   |                                                              |
| ping      | 确认电脑网络                 |                                                              |
| tree      | 树状格式展示文件夹内容       |                                                              |



## gcc指令：

`预处理：gcc -E hello.c -o hello.i`

`编译：gcc -S hello.i/hello.c -o hello.s`

`汇编：gcc -c hello.s -o hello.o`

`链接：gcc test1.c -o test1`

`一步到位：gcc hello.c -o hello -g -O0 -Wall`



## gdb操作命令：

|                 |                             |                                                  |
| --------------- | --------------------------- | ------------------------------------------------ |
| list/l          | 显示源码                    |                                                  |
| run/r           | 启动程序（以debug模式启动） |                                                  |
| kill/k          | 终止调试（不会退出GDB）     |                                                  |
| quit/q          | 退出gdb                     |                                                  |
| break/b         | 打断点                      | b 6在第六行打断点                                |
| tb              | 临时断点                    | 只生效一次                                       |
| info break/i b  | 查看断点                    |                                                  |
| d [Num]         | 删除断点                    | d 断点全删除                                     |
| step/s          | 遇到函数会进入              |                                                  |
| next/n          | 遇到函数不会进入            |                                                  |
| c               | 跳到下一次断点              |                                                  |
| finish/fin      | 跳出函数                    |                                                  |
| print/p express | 打印值                      | 可以打印表达式或变量，也能修改变量：print num=10 |
| display/disp    | 将值持续挂在输出上          |                                                  |
| undisp          | 取消持续输出                |                                                  |
| info disp       | 查看有哪些持续的输出        |                                                  |
| info args       | 输出参数                    |                                                  |
| info locals     | 输出局部变量                |                                                  |
| ignore N count  | 跳过N号断点count次          | 在循环中比较有用                                 |
| bt              | 显示函数调用栈              |                                                  |



# 文件系统编程

> [!NOTE]
>
> **什么是流？**
> 流是一个简化的编程模型，是对数据传输过程的抽象。我们在操作文件的时候只用关心打开流，使用流读数据，读完后关闭流就行了，它让程序员不再关心数据是存在硬盘里、内存里，还是从网络另一端传过来的，你只需要像接水一样“读”或者“写”就行了。
>
> ------
>
> **核心特征：**
>
> **顺序性（FIFO）：**数据像排队一样，先进入流的数据先被读取。你通常不能像操作数组那样直接跳到中间某个位置（虽然有些流支持“寻址”，但其本质逻辑仍是顺序的）。
>
> **统一性：**无论底层是复杂的磁盘扇区，还是瞬息万变的网络数据包，在程序眼中，它们都被简化成了连续的字节序列（Byte Stream）或字符序列。
>
> **解耦性：**程序不需要等整个文件全部加载到内存。就像看在线视频（流媒体）一样，你可以边下载边播放，而不需要等几个GB的文件全部下完。
>
> ------
>
> **常见的“流”有哪些：**
>
> **文件流 (File Stream)**
> 这是最常见的流。当你打开一个文件时，操作系统会为你创建一个文件流。
> 输入流：把硬盘上的数据“流”进你的程序（如 C++ 中的 ifstream）。
> 输出流：把程序产生的结果“流”向硬盘（如 ofstream）。
>
> **标准输入/输出流 (Standard I/O Stream)**
> 这是每个进程启动时默认开启的“管道”：
> stdin (Standard Input)：通常连接到你的键盘。
> stdout (Standard Output)：通常连接到你的终端屏幕。
> stderr (Standard Error)：专门用来输出错误信息的流。
>
> **网络流 (Socket Stream)**
> 当你通过 TCP 连接发送数据时，数据被看作是一串永无止境的字节，从一台电脑的内存流向另一台电脑。
>
> **目录流 (Directory Stream)**
> 目录流（如 Linux 中的 DIR * 结构），实际上是将目录下的文件名看作一个序列。通过 readdir，可以一个接一个地取出文件名，就像从流中读取字节一样。
>
> ------
>
> **为什么需要“流”？**
> **内存压力：**如果要处理一个 100GB 的日志文件，没有流的话，你必须一次性把它塞进内存，这显然是不可能的。有了流，你每次只需要处理几个 KB。
> **代码复用：**你可以写一个排序函数，让它接收一个“流”作为参数。这样，无论这个流来自文件、键盘还是网络，你的排序逻辑都不需要改动。
> **异步与并发：**流允许程序在数据还没完全到达时就开始处理已经到达的部分，极大地提高了效率。
>
> ------
>
> 常用操作有 **打开（Open）**、**读/写（Read/Write）**、**刷新（Flush）**、**关闭（Close）**
>
> 在 Linux 系统中，“一切皆文件”的思想本质上就是“一切皆流”。无论是键盘、硬盘还是进程间通信（Pipe），在内核眼中都是可以读写的字节流。



## 文件系统编程总览

### 主线

文件系统编程最重要的是先把 4 个对象区分开：

1. **文件名 / 路径名**：如 `./a.txt`
2. **inode / 文件元数据**：权限、大小、时间戳等
3. **打开文件对象（open file description）**：内核中的“已打开文件”实体，包含当前偏移量 `offset`、状态标志等
4. **文件描述符（fd）**：进程里的一个小整数，是“指向打开文件对象”的索引

> [!IMPORTANT]
>
> 很多问题都卡在“谁共享 offset”这一点：
>
> - `open()` 两次得到两个不同的打开文件对象，`offset` 不共享
> - `fork()` 之后父子进程会复制文件描述符表，但它们常常指向**同一个**打开文件对象，因此 `offset` 共享
> - `dup()` / `dup2()` 复制的是文件描述符，不是重新打开文件，因此 `offset` 共享

### 速查目录

- 目录流：`opendir` / `readdir` / `closedir`
- 低级文件 I/O：`open` / `read` / `write` / `lseek` / `close`
- 文件属性：`stat` / `lstat` / `fstat` / `access` / `chmod`
- 文件与目录操作：`unlink` / `rename` / `mkdir` / `rmdir`
- 标准 I/O 文件流：`fopen` / `fgets` / `fputs` / `fread` / `fwrite` / `fflush` / `fclose`
- 重定向：`dup` / `dup2` / `dup3`
- 内存映射：`mmap` / `munmap` / `msync`
- I/O 多路复用：`select`

## 目录流

### 目录流是什么

目录本质上也是一种特殊文件，但通常**不能用普通文本方式理解**。遍历目录时，更常见的是使用目录流接口，把目录中的目录项一个一个取出来。

### 目录流核心函数速查

| 函数 | 原型 | 作用 | 返回值 |
| --- | --- | --- | --- |
| `opendir` | `DIR *opendir(const char *name);` | 打开目录流 | 成功返回 `DIR *`，失败返回 `NULL` |
| `readdir` | `struct dirent *readdir(DIR *dirp);` | 读取下一个目录项 | 成功返回目录项指针；读到结尾或出错返回 `NULL` |
| `closedir` | `int closedir(DIR *dirp);` | 关闭目录流 | 成功 `0`，失败 `-1` |
| `rewinddir` | `void rewinddir(DIR *dirp);` | 将目录流位置重置到开头 | 无返回值 |
| `telldir` | `long telldir(DIR *dirp);` | 获取目录流当前位置 | 返回位置值 |
| `seekdir` | `void seekdir(DIR *dirp, long loc);` | 跳到目录流某个位置 | 无返回值 |

### `struct dirent` 常见字段

```c
struct dirent {
    ino_t          d_ino;    // inode 号
    off_t          d_off;    // 目录项偏移
    unsigned short d_reclen; // 记录长度
    unsigned char  d_type;   // 文件类型
    char           d_name[]; // 文件名
};
```

常见 `d_type`：

- `DT_REG`：普通文件
- `DT_DIR`：目录
- `DT_LNK`：符号链接
- `DT_FIFO`：有名管道
- `DT_SOCK`：套接字

> [!CAUTION]
>
> `d_type` 不是所有文件系统都可靠，最稳妥的做法仍然是配合 `stat()` / `lstat()` 判断文件类型。

### 遍历目录的标准写法

```c
DIR *dir = opendir(".");
if (dir == NULL) {
    perror("opendir");
    return 1;
}

struct dirent *ent = NULL;
while ((ent = readdir(dir)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
        continue;
    }
    printf("%s\n", ent->d_name);
}

closedir(dir);
```

### 目录流的易错点

- `readdir()` 返回的结构体指针通常指向库内部缓冲区，下一次调用可能覆盖上一次结果，不要长期保存裸指针
- 遍历目录时通常要手动跳过 `.` 和 `..`
- `opendir()` 成功后一定要 `closedir()`
- 遍历时如果还要拿文件详细属性，常和 `lstat()` / `stat()` 配合

## 文件描述符与低级文件 I/O

### 文件描述符的本质

文件描述符是进程维护的一张“打开文件表”的下标。最常见的 3 个标准文件描述符：

- `0`：`stdin`
- `1`：`stdout`
- `2`：`stderr`

### 核心系统调用速查

| 函数 | 原型 | 作用 | 常见要点 |
| --- | --- | --- | --- |
| `open` | `int open(const char *path, int flags, ...);` | 打开文件 | 可能需要第三个参数 `mode_t mode` |
| `read` | `ssize_t read(int fd, void *buf, size_t count);` | 从 fd 读数据 | 返回实际读到的字节数 |
| `write` | `ssize_t write(int fd, const void *buf, size_t count);` | 向 fd 写数据 | 返回实际写入字节数 |
| `lseek` | `off_t lseek(int fd, off_t offset, int whence);` | 移动文件偏移 | 可做随机访问 |
| `close` | `int close(int fd);` | 关闭文件描述符 | 关闭后 fd 失效 |

### `open()` 常用标志

| 标志 | 作用 |
| --- | --- |
| `O_RDONLY` | 只读 |
| `O_WRONLY` | 只写 |
| `O_RDWR` | 可读可写 |
| `O_CREAT` | 文件不存在则创建 |
| `O_EXCL` | 常与 `O_CREAT` 配合，要求必须新建 |
| `O_TRUNC` | 打开时清空文件 |
| `O_APPEND` | 追加写，写入位置总在文件尾 |
| `O_NONBLOCK` | 非阻塞 |

```c
int fd = open("a.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
```

> [!TIP]
>
> 只有在使用 `O_CREAT` 时，`open()` 才需要传第三个参数 `mode`。

### `read()` / `write()` 的关键理解

- 它们操作的是**字节流**
- 返回值是**实际处理的字节数**，不一定等于请求值
- `read()` 返回 `0` 表示读到文件末尾 EOF
- 返回 `-1` 表示出错，并设置 `errno`

### 健壮写法：循环写完整个缓冲区

```c
ssize_t total = 0;
while (total < len) {
    ssize_t n = write(fd, buf + total, len - total);
    if (n == -1) {
        perror("write");
        break;
    }
    total += n;
}
```

### `lseek()` 速查

```c
off_t lseek(int fd, off_t offset, int whence);
```

`whence` 常见取值：

- `SEEK_SET`：从文件开头算
- `SEEK_CUR`：从当前位置算
- `SEEK_END`：从文件尾算

常见用途：

- 获取文件大小：`lseek(fd, 0, SEEK_END)`
- 回到文件开头：`lseek(fd, 0, SEEK_SET)`
- 实现随机读写

> [!CAUTION]
>
> 管道、socket 等很多“流式 fd”不支持 `lseek()`。

## 文件属性与文件/目录操作

### `stat` 家族

| 函数 | 作用 | 特点 |
| --- | --- | --- |
| `stat(path, &st)` | 获取路径对应文件信息 | 若路径是符号链接，拿到的是链接目标的信息 |
| `lstat(path, &st)` | 获取路径对应文件信息 | 若路径是符号链接，拿到的是“链接本身”的信息 |
| `fstat(fd, &st)` | 获取已打开 fd 的文件信息 | 不依赖路径名 |

常见原型：

```c
#include <sys/stat.h>
int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
```

`struct stat` 常关注：

- `st_mode`：文件类型 + 权限位
- `st_size`：文件大小
- `st_ino`：inode 编号
- `st_nlink`：硬链接数
- `st_uid` / `st_gid`：属主 / 属组
- `st_atime` / `st_mtime` / `st_ctime`：时间戳

### 判断文件类型的宏

- `S_ISREG(st.st_mode)`：普通文件
- `S_ISDIR(st.st_mode)`：目录
- `S_ISLNK(st.st_mode)`：符号链接
- `S_ISCHR(st.st_mode)`：字符设备
- `S_ISBLK(st.st_mode)`：块设备
- `S_ISFIFO(st.st_mode)`：FIFO
- `S_ISSOCK(st.st_mode)`：套接字

### 文件与目录常见操作

| 函数 | 作用 | 说明 |
| --- | --- | --- |
| `access(path, mode)` | 检查访问权限 | `F_OK` / `R_OK` / `W_OK` / `X_OK` |
| `chmod(path, mode)` | 改权限 | 修改权限位 |
| `chown(path, uid, gid)` | 改属主属组 | 常需要权限 |
| `unlink(path)` | 删除目录项 | 普通文件常用删除函数 |
| `remove(path)` | 删除文件或空目录 | 标准库接口 |
| `rename(old, new)` | 重命名 / 移动 | 同一文件系统内常是原子操作 |
| `mkdir(path, mode)` | 创建目录 | 例如 `0755` |
| `rmdir(path)` | 删除空目录 | 目录必须为空 |

> [!IMPORTANT]
>
> `unlink()` 删除的是“目录项到 inode 的链接关系”，不是简单“抹掉文件内容”。只有当链接数变成 0 且没有进程继续打开它时，文件数据才真正被回收。

## 文件流（标准 I/O，`FILE *`）

### 系统调用 I/O 与标准 I/O 的区别

| 维度 | 低级 I/O | 标准 I/O |
| --- | --- | --- |
| 代表接口 | `open/read/write/close` | `fopen/fgets/fprintf/fread/fwrite/fclose` |
| 操作对象 | 文件描述符 `fd` | 文件流 `FILE *` |
| 所属层次 | 系统调用 | C 标准库 |
| 缓冲 | 一般直接面向内核 | 用户态缓冲更明显 |
| 适用场景 | 精细控制、进程间通信、重定向 | 文本处理、格式化读写更方便 |

### 文件流核心函数速查

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `fopen` | `FILE *fopen(const char *path, const char *mode);` | 打开文件流 |
| `fclose` | `int fclose(FILE *stream);` | 关闭文件流 |
| `fgets` | `char *fgets(char *s, int size, FILE *stream);` | 按行读字符串 |
| `fputs` | `int fputs(const char *s, FILE *stream);` | 写字符串 |
| `fprintf` | `int fprintf(FILE *stream, const char *format, ...);` | 格式化输出 |
| `fscanf` | `int fscanf(FILE *stream, const char *format, ...);` | 格式化输入 |
| `fread` | `size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);` | 二进制块读 |
| `fwrite` | `size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);` | 二进制块写 |
| `fflush` | `int fflush(FILE *stream);` | 刷新用户缓冲区 |
| `fseek` | `int fseek(FILE *stream, long offset, int whence);` | 移动流位置 |
| `ftell` | `long ftell(FILE *stream);` | 获取流位置 |
| `rewind` | `void rewind(FILE *stream);` | 回到开头 |

### `FILE *` 与 `fd` 的桥接函数

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `fileno` | `int fileno(FILE *stream);` | 从文件流拿到底层 fd |
| `fdopen` | `FILE *fdopen(int fd, const char *mode);` | 把已有 fd 包装成 `FILE *` |

> [!CAUTION]
>
> `fdopen()` 不会重新打开文件，而是让 `FILE *` 复用这个 fd。后续关闭时要注意不要把同一个底层 fd 重复关闭。

### `fopen()` 常见模式

| 模式 | 说明 |
| --- | --- |
| `"r"` | 只读，文件必须存在 |
| `"w"` | 只写，不存在则创建，存在则清空 |
| `"a"` | 追加写，不存在则创建 |
| `"r+"` | 读写，文件必须存在 |
| `"w+"` | 读写，存在则清空，不存在则创建 |
| `"a+"` | 读写追加 |
| `"rb"` / `"wb"` | 二进制模式，跨平台场景更规范 |

### 关于缓冲区的辨析

标准 I/O 的重点不是“能不能读写”，而是“**它会自己先在用户态做缓存**”。

常见 3 种缓冲模式：

- **全缓冲**：缓冲区满了才真正写
- **行缓冲**：遇到换行或缓冲区满时写出，终端输出常见
- **无缓冲**：几乎立即写出，`stderr` 常见

### `fflush()` 到底刷新的是什么

- `fflush(FILE *stream)` 刷新的是 **C 标准库的用户态缓冲区**
- 它不是直接等价于“数据已经写到磁盘”
- 如果追求刷到内核缓冲区之后再尽量落盘，还常会结合 `fsync(fd)` / `fdatasync(fd)`

> [!CAUTION]
>
> 混用 `read/write` 和 `fread/fwrite` 操作同一文件时要非常小心，因为它们的缓冲和偏移管理不完全一致，容易出现顺序错乱。除非很清楚内部机制，否则尽量不要混用。

## 重定向与文件描述符复制

### 重定向的本质

所谓输入输出重定向，本质上就是：**让某个标准文件描述符（0/1/2）不再指向终端，而改为指向别的打开文件对象**。

### 核心函数速查

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `dup` | `int dup(int oldfd);` | 复制一个 fd，返回当前可用的最小新 fd |
| `dup2` | `int dup2(int oldfd, int newfd);` | 让 `newfd` 指向和 `oldfd` 相同的打开文件对象 |
| `dup3` | `int dup3(int oldfd, int newfd, int flags);` | 类似 `dup2`，可附带标志 |

### `dup` / `dup2` 关键结论

- 新旧 fd 指向同一个打开文件对象
- 因此共享文件偏移 `offset`
- 共享文件状态标志，如追加写等
- 但它们是两个不同的 fd，关闭其中一个不影响另一个继续存在

### 标准输出重定向示例

```c
int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd == -1) {
    perror("open");
    return 1;
}

if (dup2(fd, STDOUT_FILENO) == -1) {
    perror("dup2");
    return 1;
}

printf("this goes to out.txt\n");
close(fd);
```

### 为什么 shell 中 `>` 能工作

shell 在执行命令前，通常会先：

1. `open()` 打开目标文件
2. `dup2(fd, STDOUT_FILENO)` 把标准输出接过去
3. 再 `exec()` 启动真正的程序

于是程序仍然只是在往 `stdout` 写，但 `stdout` 早就不再连着终端了。

## 内存映射（`mmap`）

### 基本思想

`mmap` 的核心是：把文件的一段内容映射到进程虚拟地址空间中，之后可以像访问普通内存一样访问文件内容。

它非常适合：

- 读取大文件
- 随机访问文件内容
- 进程间共享内存
- 避免频繁 `read` / `write` 拷贝

### 重要原型

```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);
```

### `mmap()` 参数速查

| 参数 | 含义 | 常见写法 |
| --- | --- | --- |
| `addr` | 希望映射到的地址 | 通常写 `NULL`，让内核决定 |
| `length` | 映射长度 | 通常写文件大小或需要映射的字节数 |
| `prot` | 访问权限 | `PROT_READ`、`PROT_WRITE`、`PROT_EXEC` |
| `flags` | 映射方式 | `MAP_SHARED`、`MAP_PRIVATE` |
| `fd` | 被映射文件的 fd | 通常来自 `open()` |
| `offset` | 从文件哪个偏移开始映射 | 常写 `0`，且通常要求页对齐 |

### 返回值

- 成功：返回映射区起始地址
- 失败：返回 `MAP_FAILED`，不是 `NULL`

### `MAP_SHARED` 与 `MAP_PRIVATE`

- `MAP_SHARED`：映射区修改会反映到文件，也可能被其他映射同一文件的进程看到
- `MAP_PRIVATE`：写时复制，修改只影响当前进程，不回写原文件

### 一个高频结论

如果只是“把整个文件映射进来然后读写”：

- `addr` 写 `NULL`
- `length` 写文件大小
- `prot` 常写 `PROT_READ | PROT_WRITE`
- `flags` 常写 `MAP_SHARED`
- `fd` 用 `open()` 打开的文件描述符
- `offset` 写 `0`

### `mmap` 的常见坑

- `offset` 常常要求按页大小对齐
- 映射长度不能随便超过文件有效大小，否则访问可能触发 `SIGBUS`
- `mmap()` 成功后依然要 `munmap()`
- 映射成功后通常可以关闭 `fd`，映射关系仍然存在，但更稳妥的学习阶段可以先保持“用完再关”
- 如果要确保修改尽快同步回文件，可调用 `msync()`

## I/O 多路复用：`select`

### `fd_set` 与 4 个宏

`fd_set` 本质上是文件描述符集合，常与以下宏配合使用：

| 宏 | 作用 |
| --- | --- |
| `FD_ZERO(&set)` | 清空集合 |
| `FD_SET(fd, &set)` | 添加 fd |
| `FD_CLR(fd, &set)` | 删除 fd |
| `FD_ISSET(fd, &set)` | 判断 fd 是否就绪 |

### 原型

```c
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

### 参数理解

- `nfds`：被监听的最大 fd 加 1
- `readfds`：监听读事件
- `writefds`：监听写事件
- `exceptfds`：监听异常事件
- `timeout`：
  - `NULL`：一直阻塞
  - `{0, 0}`：立即返回
  - 具体值：最多等待对应时间

### 返回值

- `> 0`：有若干 fd 就绪
- `0`：超时
- `-1`：出错，注意 `EINTR`

> [!CAUTION]
>
> `select()` 返回后，传入的 `fd_set` 会被内核改写，只保留“就绪的那些 fd”，下一轮循环前往往要重新 `FD_ZERO` 和 `FD_SET`。

# 进程与线程

## 进程基础

### 进程是什么

进程是程序的一次执行实例。它拥有独立的虚拟地址空间、文件描述符表、信号处理状态、工作目录、环境变量等运行时资源。

### 常用进程标识函数

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `getpid` | `pid_t getpid(void);` | 获取当前进程 PID |
| `getppid` | `pid_t getppid(void);` | 获取父进程 PID |
| `getuid` | `uid_t getuid(void);` | 获取真实用户 ID |
| `geteuid` | `uid_t geteuid(void);` | 获取有效用户 ID |

## `fork()`：创建子进程

### 原型

```c
pid_t fork(void);
```

### 返回值必须背熟

- 在父进程中：返回子进程 PID
- 在子进程中：返回 `0`
- 失败：返回 `-1`

### `fork()` 的工作原理

`fork()` 之后会创建一个子进程，子进程最初几乎是父进程的副本。现代系统通常采用**写时复制（Copy-On-Write, COW）**，不是一上来就把整块内存都复制一份，而是父子先进共享物理页，谁先修改谁再复制。

### `fork()` 之后哪些东西“像复制了”，哪些东西“其实共享”

- 代码段、数据段、堆、栈从逻辑上看都复制了，但底层常通过 COW 延迟复制
- 文件描述符表被复制了
- 但复制出来的 fd 往往仍指向**同一个打开文件对象**
- 因此 `offset` 共享

> [!IMPORTANT]
>
> `open()` 后再 `fork()`：
>
> - 父子进程中的对应 fd 通常共享同一个打开文件对象
> - 所以共享 `offset`
>
> `fork()` 后再各自 `open()`：
>
> - 父子进程分别创建自己的打开文件对象
> - 两边 `offset` 不共享

## `exec` 函数族：用新程序替换当前进程

### 核心理解

`exec` 不会创建新进程，它是用一个新程序映像替换当前进程的代码和数据。也就是说，通常是“先 `fork()`，子进程里再 `exec()`”。

### 常见函数

| 函数 | 特点 |
| --- | --- |
| `execl` | 参数逐个列出，最后以 `NULL` 结尾 |
| `execv` | 参数以数组形式传入 |
| `execlp` | 会按 `PATH` 查找程序 |
| `execvp` | 数组参数 + 按 `PATH` 查找 |
| `execle` | 可显式传环境变量 |
| `execve` | 底层核心接口 |

### 常见原型

```c
int execl(const char *path, const char *arg, ...);
int execv(const char *path, char *const argv[]);
int execlp(const char *file, const char *arg, ...);
int execvp(const char *file, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
```

### `exec` 的返回规律

- 成功：**不会返回**
- 失败：返回 `-1`，并设置 `errno`

### 最经典组合

```c
pid_t pid = fork();
if (pid == 0) {
    execlp("ls", "ls", "-l", NULL);
    perror("execlp");
    _exit(1);
}
```

> [!CAUTION]
>
> 子进程 `exec()` 失败后，常使用 `_exit()` 直接退出，而不是 `exit()`，避免把父进程继承来的标准 I/O 缓冲再次刷出。

## 进程回收：`wait` / `waitpid`

### 为什么必须回收子进程

子进程退出后，如果父进程不回收，它会短暂保留退出信息，形成**僵尸进程**。僵尸进程本身不运行，但会占用内核中的进程表项。

### 核心函数

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `wait` | `pid_t wait(int *status);` | 等待任意一个子进程退出 |
| `waitpid` | `pid_t waitpid(pid_t pid, int *status, int options);` | 等待指定子进程或按条件等待 |

### `waitpid()` 常见参数

- `pid > 0`：等待指定 PID
- `pid == -1`：等待任意子进程，类似 `wait`
- `options == 0`：阻塞等待
- `options == WNOHANG`：非阻塞等待

### 解析退出状态的宏

- `WIFEXITED(status)`：是否正常退出
- `WEXITSTATUS(status)`：正常退出码
- `WIFSIGNALED(status)`：是否被信号杀死
- `WTERMSIG(status)`：终止它的信号编号

## 进程终止相关

| 函数 | 作用 | 特点 |
| --- | --- | --- |
| `exit(int status)` | 正常结束进程 | 会刷新标准 I/O 缓冲、调用 `atexit` |
| `_exit(int status)` | 立即结束进程 | 直接回到内核，常用于 `fork` 后子进程 |
| `abort(void)` | 异常终止 | 会触发 `SIGABRT` |

## 常见进程控制函数补充

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `kill` | `int kill(pid_t pid, int sig);` | 给指定进程或进程组发送信号 |
| `raise` | `int raise(int sig);` | 给当前进程发送信号 |
| `alarm` | `unsigned int alarm(unsigned int seconds);` | 设置定时发送 `SIGALRM` |
| `pause` | `int pause(void);` | 挂起当前进程直到收到信号 |

> [!TIP]
>
> `kill(pid, sig)` 的名字虽然叫 “kill”，但它并不一定是“杀死进程”，本质上只是“发送一个信号”。例如 `kill(pid, SIGTERM)` 是请求优雅终止，`kill(pid, 0)` 常被用来探测进程是否存在。

## 管道、重定向与进程间通信

### 匿名管道 `pipe`

```c
int pipe(int pipefd[2]);
```

- `pipefd[0]`：读端
- `pipefd[1]`：写端
- 典型用途：有亲缘关系进程间通信

### 匿名管道的关键特征

- 半双工，单向通信
- 适合父子进程、兄弟进程
- 读完写端关闭后，读端 `read()` 返回 `0` 表示 EOF
- 如果读端都关闭了，继续写通常会触发 `SIGPIPE` 或写失败

### 有名管道 FIFO

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `mkfifo` | `int mkfifo(const char *pathname, mode_t mode);` | 创建有名管道 |

FIFO 存在于文件系统中，因此**没有亲缘关系的进程**也能通过同一路径通信。

### 管道和重定向经常一起考

shell 中的 `cmd1 | cmd2` 本质上通常是：

1. `pipe()`
2. `fork()` 出多个子进程
3. 对前一个命令 `dup2(pipefd[1], STDOUT_FILENO)`
4. 对后一个命令 `dup2(pipefd[0], STDIN_FILENO)`
5. 分别 `exec()`

## 线程基础

### 进程和线程的区别

| 项目 | 进程 | 线程 |
| --- | --- | --- |
| 地址空间 | 独立 | 同一进程内共享 |
| 创建开销 | 较大 | 较小 |
| 数据共享 | 需要 IPC | 天然共享全局区、堆、文件描述符 |
| 崩溃影响 | 常局限于本进程 | 一个线程崩溃可能拖垮整个进程 |

### 线程中哪些资源共享

同一进程中的线程通常共享：

- 虚拟地址空间
- 全局变量 / 堆区
- 打开的文件描述符
- 信号处理方式

但通常各自拥有：

- 线程 ID
- 寄存器上下文
- 栈
- 线程局部存储

## POSIX 线程核心函数速查

### 线程创建与回收

| 函数 | 原型 | 作用 |
| --- | --- | --- |
| `pthread_create` | `int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);` | 创建线程 |
| `pthread_join` | `int pthread_join(pthread_t thread, void **retval);` | 等待线程结束并回收 |
| `pthread_detach` | `int pthread_detach(pthread_t thread);` | 让线程变成分离态 |
| `pthread_exit` | `void pthread_exit(void *retval);` | 主动结束当前线程 |
| `pthread_self` | `pthread_t pthread_self(void);` | 获取当前线程 ID |

### 线程创建示例

```c
void *worker(void *arg) {
    printf("thread running\n");
    return NULL;
}

pthread_t tid;
pthread_create(&tid, NULL, worker, NULL);
pthread_join(tid, NULL);
```

> [!IMPORTANT]
>
> 一个线程要么 `join`，要么 `detach`，否则其结束后相关资源无法及时回收，类似“线程版僵尸”问题。
>
> 编译线程程序时通常要带上 `-pthread`，它不仅负责链接 pthread 库，还会打开一些与线程相关的编译选项。

## 线程同步：互斥锁与条件变量

### 为什么需要同步

线程共享地址空间，所以多个线程同时访问同一份共享数据时，容易发生**竞态条件（race condition）**。

### 互斥锁 `pthread_mutex`

| 函数 | 作用 |
| --- | --- |
| `pthread_mutex_init` | 初始化互斥锁 |
| `pthread_mutex_destroy` | 销毁互斥锁 |
| `pthread_mutex_lock` | 加锁 |
| `pthread_mutex_unlock` | 解锁 |
| `pthread_mutex_trylock` | 尝试加锁 |

常见原型：

```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

### 条件变量 `pthread_cond`

条件变量不是锁，它通常和互斥锁配合使用，用于“线程等待某个条件成立”。

| 函数 | 作用 |
| --- | --- |
| `pthread_cond_init` | 初始化条件变量 |
| `pthread_cond_wait` | 等待条件成立，等待时会原子地释放互斥锁 |
| `pthread_cond_signal` | 唤醒一个等待线程 |
| `pthread_cond_broadcast` | 唤醒所有等待线程 |
| `pthread_cond_destroy` | 销毁条件变量 |

### `pthread_cond_wait()` 的关键理解

```c
pthread_mutex_lock(&mutex);
while (!ready) {
    pthread_cond_wait(&cond, &mutex);
}
/* 条件满足后继续执行 */
pthread_mutex_unlock(&mutex);
```

为什么常写成 `while` 而不是 `if`：

- 被唤醒后需要重新检查条件
- 可能存在“虚假唤醒”
- 也可能多个线程被唤醒后，只有一个线程真正拿到资源

## 线程同步补充：读写锁与信号量

### 读写锁 `pthread_rwlock`

适合“读多写少”的场景：

- 多个读线程可并发进入
- 写线程需要独占

常见函数：

- `pthread_rwlock_init`
- `pthread_rwlock_rdlock`
- `pthread_rwlock_wrlock`
- `pthread_rwlock_unlock`
- `pthread_rwlock_destroy`

### 信号量 `sem_t`

信号量本质上是一个计数器，既可用于互斥，也可用于控制资源数量。

常见函数：

- `sem_init`
- `sem_wait`
- `sem_post`
- `sem_destroy`

## 文件系统编程与进程线程的联系

这一部分很容易综合出题，重点记下面几条：

1. `fork()` 之后父子进程的相关 fd 常共享同一个打开文件对象，因此共享 `offset`
2. `dup()` / `dup2()` 只是复制 fd，不会重新打开文件，因此也共享 `offset`
3. 同一进程内多个线程共享文件描述符表，所以线程间操作同一个 fd 时也要考虑同步与偏移影响
4. `exec()` 会替换进程映像，但默认保留未设置 `close-on-exec` 的文件描述符
5. 重定向、管道、守护进程、shell 执行模型，本质上都离不开 `fork + dup2 + exec + wait`

## 一页速记版

### 文件系统编程最常用函数

- 目录遍历：`opendir`、`readdir`、`closedir`
- 文件打开读写：`open`、`read`、`write`、`lseek`、`close`
- 文件属性：`stat`、`lstat`、`fstat`
- 文件流：`fopen`、`fgets`、`fputs`、`fread`、`fwrite`、`fflush`、`fclose`
- 重定向：`dup`、`dup2`
- 映射：`mmap`、`munmap`、`msync`
- 多路复用：`select`

### 进程线程最常用函数

- 进程创建：`fork`
- 程序替换：`exec*`
- 进程回收：`wait`、`waitpid`
- 管道：`pipe`、`mkfifo`
- 线程创建回收：`pthread_create`、`pthread_join`、`pthread_detach`
- 互斥同步：`pthread_mutex_lock`、`pthread_mutex_unlock`
- 条件等待：`pthread_cond_wait`、`pthread_cond_signal`

# 网络编程







