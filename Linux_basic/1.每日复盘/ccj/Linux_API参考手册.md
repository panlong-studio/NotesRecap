# Linux C API 参考手册

本文档整理了Linux系统编程中常用的C API函数，包括函数定义、参数类型及结构体关系，供开发时参考。

---

# 目录

[TOC]



---

## 一、 字符串处理

### 1 .字符串长度

#### strlen - 字符串长度

```c
#include <string.h>

size_t strlen(const char *s);
```

---

### 2 .字符串复制

#### strcpy/strncpy - 复制

```c
#include <string.h>

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
```

| 函数    | 说明                          |
| ------- | ----------------------------- |
| strcpy  | 复制整个src（含\0）           |
| strncpy | 最多复制n字节，可能不以\0结尾 |

**建议用法：**

```c
strncpy(dest, src, sizeof(dest)-1);
dest[sizeof(dest)-1] = '\0';
```

---

### 3 .字符串连接

#### strcat/strncat - 连接

```c
#include <string.h>

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
```

**安全用法：**

```c
n = sizeof(dest) - strlen(dest) - 1;
strncat(dest, src, n);
```

---

### 4 .字符串比较

#### strcmp/strncmp - 比较

```c
#include <string.h>

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
```

| 返回值 | 说明    |
| ------ | ------- |
| 0      | 相等    |
| <0     | s1 < s2 |
| >0     | s1 > s2 |

---

### 5. 字符分类（ctype.h）

```c
#include <ctype.h>

int islower(int c);   // 小写字母
int isupper(int c);   // 大写字母
int isalpha(int c);  // 字母
int isdigit(int c);  // 数字
int isalnum(int c);   // 字母或数字
int isspace(int c);   // 空白字符
int toupper(int c);   // 转大写
int tolower(int c);   // 转小写
```

------

## 二、文件操作

### 1. 无缓冲文件流

无缓冲文件流是直接与内核交互的文件操作方式，使用文件描述符。

#### open - 打开/创建文件

```c
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 文件路径名 |
| flags | int | 打开标志 |
| mode | mode_t | 文件权限（使用O_CREAT时必需）|

**flags常用标志：**

| 标志 | 说明 |
|------|------|
| O_RDONLY | 只读 |
| O_WRONLY | 只写 |
| O_RDWR | 可读可写 |
| O_CREAT | 文件不存在则创建 |
| O_EXCL | 与O_CREAT配合，文件存在则失败 |
| O_TRUNC | 截断文件为空 |
| O_APPEND | 追加模式 |

**返回值：** 成功返回文件描述符（非负整数），失败返回-1

---

#### close - 关闭文件描述符

```c
#include <unistd.h>

int close(int fd);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 要关闭的文件描述符 |

**返回值：** 成功返回0，失败返回-1

---

#### read - 读取数据

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 文件描述符 |
| buf | void * | 接收数据的缓冲区 |
| count | size_t | 要读取的字节数 |

**返回值：** 成功返回实际读取的字节数，到达文件末尾返回0，出错返回-1

---

#### write - 写入数据

```c
#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 文件描述符 |
| buf | const void * | 要写入的数据缓冲区 |
| count | size_t | 要写入的字节数 |

**返回值：** 成功返回实际写入的字节数，失败返回-1

---

#### lseek - 移动文件指针

```c
#include <sys/types.h>
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 文件描述符 |
| offset | off_t | 偏移量 |
| whence | int | 基准位置 |

**whence取值：**

| 值 | 说明 |
|------|------|
| SEEK_SET | 相对于文件开头 |
| SEEK_CUR | 相对于当前位置 |
| SEEK_END | 相对于文件末尾 |

**返回值：** 成功返回新位置（相对于文件开头的字节数），失败返回-1

---

#### ftruncate - 改变文件大小

```c
#include <unistd.h>

int ftruncate(int fd, off_t length);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 文件描述符 |
| length | off_t | 新的文件大小 |

**返回值：** 成功返回0，失败返回-1

---

#### fileno - 获取文件流的文件描述符

```c
#include <stdio.h>

int fileno(FILE *stream);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| stream | FILE * | 文件流指针 |

**返回值：** 返回文件描述符

---

### 2. 目录流

目录流用于读取目录内容。

#### opendir - 打开目录

```c
#include <dirent.h>
#include <sys/types.h>

DIR *opendir(const char *name);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| name | const char * | 目录路径 |

**返回值：** 成功返回目录流指针，失败返回NULL

---

#### closedir - 关闭目录

```c
#include <sys/types.h>
#include <dirent.h>

int closedir(DIR *dirp);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dirp | DIR * | 目录流指针 |

**返回值：** 成功返回0，失败返回-1

---

#### readdir - 读取目录项

```c
#include <dirent.h>

struct dirent *readdir(DIR *dirp);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dirp | DIR * | 目录流指针 |

**返回值：** 成功返回目录项结构体指针，目录读完或出错返回NULL

**dirent结构体：**

```c
struct dirent {
    ino_t          d_ino;       // inode编号
    off_t          d_off;        // 到下一个目录项的偏移
    unsigned short  d_reclen;     // 目录项实际大小
    unsigned char   d_type;       // 文件类型
    char            d_name[256];  // 文件名
};
```

**d_type取值（文件类型）：**

| 值 | 类型 |
|------|------|
| DT_BLK | 块设备 |
| DT_CHR | 字符设备 |
| DT_DIR | 目录 |
| DT_FIFO | 有名管道 |
| DT_LNK | 符号链接 |
| DT_REG | 普通文件 |
| DT_SOCK | 套接字 |
| DT_UNKNOWN | 未知 |

---

#### telldir - 获取目录流位置

```c
#include <dirent.h>

long telldir(DIR *dirp);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dirp | DIR * | 目录流指针 |

**返回值：** 返回目录流当前位置标记

---

#### seekdir - 设置目录流位置

```c
#include <dirent.h>

void seekdir(DIR *dirp, long loc);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dirp | DIR * | 目录流指针 |
| loc | long | telldir返回的位置标记 |

---

#### rewinddir - 重置目录流

```c
#include <dirent.h>

void rewinddir(DIR *dirp);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dirp | DIR * | 目录流指针 |

---

### stat相关 - 获取文件信息

#### stat - 获取文件状态

```c
#include <sys/stat.h>

int stat(const char *path, struct stat *buf);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| path | const char * | 文件路径 |
| buf | struct stat * | 存储文件信息的结构体 |

**返回值：** 成功返回0，失败返回-1

**stat结构体（主要成员）：**

```c
struct stat {
    mode_t    st_mode;    // 文件类型和权限
    nlink_t   st_nlink;    // 硬链接数
    uid_t     st_uid;      // 所有者用户ID
    gid_t     st_gid;      // 所有者组ID
    off_t     st_size;     // 文件大小
    struct timespec st_mtim; // 最后修改时间
};
```

#### fstat - 通过文件描述符获取文件状态

```c
#include <sys/stat.h>

int fstat(int fd, struct stat *buf);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| fd | int | 文件描述符 |
| buf | struct stat * | 存储文件信息的结构体 |

**返回值：** 成功返回0，失败返回-1

---

#### chmod - 改变文件权限

```c
#include <sys/stat.h>

int chmod(const char *pathname, mode_t mode);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 文件路径 |
| mode | mode_t | 新权限（八进制如0644）|

**返回值：** 成功返回0，失败返回-1

---

#### mkdir - 创建目录

```c
#include <sys/stat.h>
#include <sys/types.h>

int mkdir(const char *pathname, mode_t mode);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 目录路径 |
| mode | mode_t | 目录权限 |

**返回值：** 成功返回0，失败返回-1

---

#### rmdir - 删除空目录

```c
#include <unistd.h>

int rmdir(const char *pathname);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 目录路径 |

**返回值：** 成功返回0，失败返回-1

---

#### getcwd - 获取当前工作目录

```c
#include <unistd.h>

char *getcwd(char *buf, size_t size);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| buf | char * | 存储路径的缓冲区 |
| size | size_t | 缓冲区大小 |

**返回值：** 成功返回buf，失败返回NULL

---

#### chdir - 改变当前工作目录

```c
#include <unistd.h>

int chdir(const char *path);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| path | const char * | 新工作目录路径 |

**返回值：** 成功返回0，失败返回-1

---

### 3. 文件映射

文件映射允许像访问内存一样访问文件内容。

#### mmap - 建立文件映射

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| addr | void * | 建议映射地址（通常NULL让系统选择） |
| length | size_t | 映射区域大小（字节） |
| prot | int | 映射区域权限 |
| flags | int | 映射特性 |
| fd | int | 文件描述符 |
| offset | off_t | 文件内偏移量 |

**prot取值：**

| 值 | 说明 |
|------|------|
| PROT_READ | 可读 |
| PROT_WRITE | 可写 |
| PROT_READ\|PROT_WRITE | 可读可写 |

**flags取值：**

| 值 | 说明 |
|------|------|
| MAP_SHARED | 修改反映到文件，对其他进程可见 |
| MAP_PRIVATE | 修改不反映到文件，私有副本 |

**返回值：** 成功返回映射内存地址，失败返回MAP_FAILED

---

#### munmap - 解除内存映射

```c
#include <sys/mman.h>

int munmap(void *addr, size_t length);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| addr | void * | 映射区域起始地址（mmap返回值） |
| length | size_t | 映射区域大小 |

**返回值：** 成功返回0，失败返回-1

---

### 4. 重定向

#### dup - 复制文件描述符

```c
#include <unistd.h>

int dup(int oldfd);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| oldfd | int | 要复制的文件描述符 |

**返回值：** 成功返回新的文件描述符（最小可用），失败返回-1

---

#### dup2 - 复制文件描述符到指定值

```c
#include <unistd.h>

int dup2(int oldfd, int newfd);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| oldfd | int | 要复制的文件描述符 |
| newfd | int | 指定的新文件描述符 |

**返回值：** 成功返回newfd，失败返回-1

---

**标准文件描述符常量：**

```c
STDIN_FILENO   0   // 标准输入
STDOUT_FILENO 1   // 标准输出
STDERR_FILENO 2   // 标准错误
```

---

## 三、进程管理

### 1.进程创建与控制

#### fork - 创建子进程

```c
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void);
```

**返回值：** 子进程中返回0，父进程中返回子进程PID，出错返回-1

---

#### system - 执行shell命令

```c
#include <stdlib.h>

int system(const char *command);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| command | const char * | shell命令字符串 |

**返回值：** 成功返回命令退出状态，失败返回-1

---

#### exit - 正常终止进程

```c
#include <stdlib.h>

void exit(int status);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| status | int | 退出状态码 |

---

#### _exit / _Exit - 立即终止进程

```c
#include <stdlib.h>
void _Exit(int status);

#include <unistd.h>
void _exit(int status);
```

---

### 2.进程标识

```c
#include <sys/types.h>

pid_t getpid(void);    // 获取当前进程ID
pid_t getppid(void);    // 获取父进程ID
uid_t getuid(void);     // 获取真实用户ID
gid_t getgid(void);     // 获取真实组ID
```

---

### 3.进程等待

#### wait - 等待子进程退出

```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *wstatus);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| wstatus | int * | 存储退出状态（可NULL） |

**返回值：** 成功返回子进程PID，失败返回-1

**wstatus宏：**

| 宏 | 说明 |
|------|------|
| WIFEXITED(wstatus) | 正常退出时为真 |
| WEXITSTATUS(wstatus) | 获取退出码 |
| WIFSIGNALED(wstatus) | 被信号终止时为真 |
| WTERMSIG(wstatus) | 获取信号编号 |
| WIFSTOPPED(wstatus) | 暂停时为真 |

---

#### waitpid - 等待指定子进程

```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *wstatus, int options);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pid | pid_t | 子进程PID |
| wstatus | int * | 存储退出状态（可NULL） |
| options | int | 选项 |

**pid取值：**

| 值 | 说明 |
|------|------|
| < -1 | 等待进程组ID等于pid绝对值的进程 |
| -1 | 等待任一子进程 |
| 0 | 等待同一进程组的任意子进程 |
| > 0 | 等待指定PID的进程 |

**options取值：**

| 值 | 说明 |
|------|------|
| 0 | 阻塞等待 |
| WNOHANG | 非阻塞 |

---

### 4.exec函数族

```c
#include <unistd.h>

int execl(const char *path, const char *arg0, ... /* (char *)0 */);
int execv(const char *path, char *const argv[]);
```

| 变体 | 说明 |
|------|------|
| execl | 参数以列表形式传入，以NULL结尾 |
| execv | 参数以指针数组形式传入 |

---

### 5.进程组与会话

#### getpgrp - 获取进程组ID

```c
#include <unistd.h>

pid_t getpgrp(void);
```

---

#### setpgid - 设置进程组

```c
#include <sys/types.h>
#include <unistd.h>

int setpgid(pid_t pid, pid_t pgid);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pid | pid_t | 目标进程（0表示当前进程） |
| pgid | pid_t | 新进程组ID（0表示使用pid作为进程组ID） |

**返回值：** 成功返回0，失败返回-1

---

#### getsid - 获取会话ID

```c
#include <sys/types.h>
#include <unistd.h>

pid_t getsid(pid_t pid);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pid | pid_t | 进程ID（0表示当前进程） |

**返回值：** 成功返回会话ID，失败返回-1

---

#### setsid - 创建新会话

```c
#include <sys/types.h>
#include <unistd.h>

pid_t setsid(void);
```

**返回值：** 成功返回新会话ID，失败返回-1

---

### 6.守护进程

#### syslog - 写系统日志

```c
#include <syslog.h>

void syslog(int priority, const char *format, ...);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| priority | int | 日志级别 |
| format | const char * | 格式字符串 |

**日志级别：**

| 值 | 说明 |
|------|------|
| LOG_ERR | 错误 |
| LOG_WARNING | 警告 |
| LOG_INFO | 信息 |
| LOG_DEBUG | 调试 |

---

## 四、线程

线程操作需要链接 `-lpthread` 或 `-pthread`。

### 1.线程创建与退出

#### pthread_create - 创建线程

```c
#include <pthread.h>

int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine) (void *),
    void *arg
);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| thread | pthread_t * | 存储线程ID |
| attr | const pthread_attr_t * | 线程属性（通常NULL） |
| start_routine | void *(*)(void *) | 线程入口函数 |
| arg | void * | 传给线程的参数 |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_exit - 退出线程

```c
#include <pthread.h>

void pthread_exit(void *retval);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| retval | void * | 线程退出状态 |

---

#### pthread_join - 等待线程结束

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| thread | pthread_t | 要等待的线程ID |
| retval | void ** | 存储线程退出状态（可NULL） |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_cancel - 请求取消线程

```c
#include <pthread.h>

int pthread_cancel(pthread_t thread);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| thread | pthread_t | 目标线程ID |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_testcancel - 添加取消点

```c
#include <pthread.h>

void pthread_testcancel(void);
```

---

### 2.线程清理

#### pthread_cleanup_push - 注册清理函数

```c
#include <pthread.h>

void pthread_cleanup_push(
    void (*routine)(void *),
    void *arg
);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| routine | void (*)(void *) | 清理函数 |
| arg | void * | 传给清理函数的参数 |

---

#### pthread_cleanup_pop - 弹出清理函数

```c
#include <pthread.h>

void pthread_cleanup_pop(int execute);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| execute | int | 0不执行，非0执行 |

---

### 3.互斥锁

#### pthread_mutex_init - 初始化互斥锁

```c
#include <pthread.h>

int pthread_mutex_init(
    pthread_mutex_t *mutex,
    const pthread_mutexattr_t *attr
);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mutex | pthread_mutex_t * | 互斥锁 |
| attr | const pthread_mutexattr_t * | 属性（通常NULL） |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_mutex_destroy - 销毁互斥锁

```c
#include <pthread.h>

int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mutex | pthread_mutex_t * | 互斥锁 |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_mutex_lock - 加锁

```c
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mutex | pthread_mutex_t * | 互斥锁 |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_mutex_unlock - 解锁

```c
#include <pthread.h>

int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mutex | pthread_mutex_t * | 互斥锁 |

**返回值：** 成功返回0，失败返回错误码

---

#### pthread_mutex_trylock - 尝试加锁

```c
#include <pthread.h>

int pthread_mutex_trylock(pthread_mutex_t *mutex);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mutex | pthread_mutex_t * | 互斥锁 |

**返回值：** 获取锁返回0，失败返回EBUSY

---

### 4.互斥锁属性

#### pthread_mutexattr_init - 初始化互斥锁属性

```c
#include <pthread.h>

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
```

---

#### pthread_mutexattr_settype - 设置互斥锁类型

```c
#include <pthread.h>

int pthread_mutexattr_settype(
    pthread_mutexattr_t *attr,
    int type
);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| attr | pthread_mutexattr_t * | 属性对象 |
| type | int | 锁类型 |

**type取值：**

| 值 | 说明 |
|------|------|
| PTHREAD_MUTEX_NORMAL | 普通锁（默认） |
| PTHREAD_MUTEX_ERRORCHECK | 检错锁 |
| PTHREAD_MUTEX_RECURSIVE | 递归锁 |

---

#### pthread_mutexattr_destroy - 销毁互斥锁属性

```c
#include <pthread.h>

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
```

---

### 5.自旋锁

```c
#include <pthread.h>

pthread_spinlock_t lock;
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);
int pthread_spin_destroy(pthread_spinlock_t *lock);
```

**pshared取值：**

| 值 | 说明 |
|------|------|
| PTHREAD_PROCESS_PRIVATE | 仅同进程线程 |
| PTHREAD_PROCESS_SHARED | 不同进程 |

---

### 6.读写锁

```c
#include <pthread.h>

pthread_rwlock_t rwlock;
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);  // 读锁
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);  // 写锁
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

---

### 7.线程同步条件变量

#### pthread_cond_init - 初始化条件变量

```c
#include <pthread.h>

int pthread_cond_init(
    pthread_cond_t *cond,
    const pthread_condattr_t *attr
);
```

---

#### pthread_cond_wait - 等待条件变量

```c
#include <pthread.h>

int pthread_cond_wait(
    pthread_cond_t *cond,
    pthread_mutex_t *mutex
);
```

---

#### pthread_cond_signal - 唤醒一个线程

```c
#include <pthread.h>

int pthread_cond_signal(pthread_cond_t *cond);
```

---

#### pthread_cond_broadcast - 唤醒所有线程

```c
#include <pthread.h>

int pthread_cond_broadcast(pthread_cond_t *cond);
```

---

### 8.时间函数

#### gettimeofday - 获取时间

```c
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);
```

**timeval结构体：**

```c
struct timeval {
    time_t tv_sec;      // 秒
    suseconds_t tv_usec; // 微秒
};
```

---

## 五、进程间通信

### 1.管道

#### pipe - 创建匿名管道

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pipefd | int[2] | 文件描述符数组 |

**返回值：** 成功返回0，pipefd[0]为读端，pipefd[1]为写端

---

#### mkfifo - 创建命名管道

```c
#include <sys/types.h>
#include <sys/stat.h>

int mkfifo(const char *pathname, mode_t mode);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 管道文件路径 |
| mode | mode_t | 权限 |

**返回值：** 成功返回0，失败返回-1

---

#### popen - 创建管道并执行命令

```c
#include <stdio.h>

FILE *popen(const char *command, const char *type);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| command | const char * | shell命令 |
| type | const char * | 模式（"r"读，"w"写） |

**返回值：** 成功返回文件指针，失败返回NULL

---

### 2.共享内存

#### ftok - 生成IPC键值

```c
#include <sys/types.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pathname | const char * | 存在且可访问的文件路径 |
| proj_id | int | 项目ID（1-255，不能为0） |

**返回值：** 成功返回键值，失败返回-1

---

#### shmget - 创建/获取共享内存

```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| key | key_t | 键值 |
| size | size_t | 共享内存大小 |
| shmflg | int | 权限和标志 |

**shmflg常用值：**

| 值 | 说明 |
|------|------|
| IPC_CREAT | 创建新的 |
| IPC_EXCL | 配合CREAT，已存在则失败 |
| 0600 | 权限 |

**返回值：** 成功返回共享内存ID，失败返回-1

---

#### shmat - 映射共享内存

```c
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| shmid | int | 共享内存ID |
| shmaddr | const void * | 建议地址（通常NULL） |
| shmflg | int | 标志（通常0） |

**返回值：** 成功返回映射地址，失败返回-1

---

#### shmdt - 解除共享内存映射

```c
#include <sys/types.h>
#include <sys/shm.h>

int shmdt(const void *shmaddr);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| shmaddr | const void * | 映射地址（shmat返回值） |

**返回值：** 成功返回0，失败返回-1

---

#### shmctl - 共享内存控制

```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| shmid | int | 共享内存ID |
| cmd | int | 命令 |
| buf | struct shmid_ds * | 信息结构体 |

**cmd取值：**

| 值 | 说明 |
|------|------|
| IPC_STAT | 获取状态 |
| IPC_SET | 设置属性 |
| IPC_RMID | 删除共享内存 |

---

### 3.POSIX共享内存

```c
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);
```

---

## 六、信号

### signal - 设置信号处理

```c
#include <signal.h>

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| signum | int | 信号编号 |
| handler | sighandler_t | 处理函数/SIG_IGN/SIG_DFL |

**返回值：** 成功返回之前的处理函数，失败返回SIG_ERR

---

### sigaction - 检查/修改信号处理

```c
#include <signal.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| signum | int | 信号编号 |
| act | const struct sigaction * | 新动作（可NULL） |
| oldact | struct sigaction * | 旧动作（可NULL） |

**sigaction结构体：**

```c
struct sigaction {
    void     (*sa_handler)(int);           // 处理函数
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;                   // 阻塞信号集
    int        sa_flags;                   // 选项
    void     (*sa_restorer)(void);        // 已废弃
};
```

**sa_flags常用值：**

| 值 | 说明 |
|------|------|
| SA_SIGINFO | 使用sa_sigaction |
| SA_RESETHAND | 处理后恢复默认（一次有效） |
| SA_NODEFER | 处理时不阻塞同信号 |
| SA_RESTART | 自动重启被信号打断的系统调用 |

---

### 信号集操作

```c
#include <signal.h>

int sigemptyset(sigset_t *set);        // 清空信号集
int sigfillset(sigset_t *set);          // 添加所有信号
int sigaddset(sigset_t *set, int signo); // 添加信号
int sigdelset(sigset_t *set, int signo); // 删除信号
int sigismember(const sigset_t *set, int signo); // 检查信号
```

---

### kill - 发送信号

```c
#include <sys/types.h>
#include <signal.h>

int kill(pid_t pid, int sig);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| pid | pid_t | 目标进程ID |
| sig | int | 信号编号 |

**pid取值：**

| 值 | 说明 |
|------|------|
| > 0 | 指定进程 |
| 0 | 同进程组所有进程 |
| -1 | 所有有权限发送的进程 |
| < -1 | 进程组ID等于pid绝对值 |

**返回值：** 成功返回0，失败返回-1

---

### pause - 等待信号

```c
#include <unistd.h>

int pause(void);
```

**返回值：** 总是返回-1（被信号打断时）

---

### sigpending - 检查未决信号

```c
#include <signal.h>

int sigpending(sigset_t *set);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| set | sigset_t * | 存储未决信号集 |

**返回值：** 成功返回0，失败返回-1

---

### sigprocmask - 检查/更改信号屏蔽字

```c
#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| how | int | 如何修改 |
| set | const sigset_t * | 新信号集 |
| oldset | sigset_t * | 旧信号集（可NULL） |

**how取值：**

| 值 | 说明 |
|------|------|
| SIG_BLOCK | 添加set中的信号到屏蔽字 |
| SIG_UNBLOCK | 从屏蔽字移除set中的信号 |
| SIG_SETMASK | 替换屏蔽字为set |

**返回值：** 成功返回0，失败返回-1

---

### sigsuspend - 原子修改屏蔽字并等待信号

```c
#include <signal.h>

int sigsuspend(const sigset_t *mask);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mask | const sigset_t * | 新信号屏蔽字 |

**返回值：** 总是返回-1

---

### alarm - 设置闹钟

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| seconds | unsigned int | 秒数（0取消） |

---

### setitimer - 设置定时器

```c
#include <sys/time.h>

int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int getitimer(int which, struct itimerval *curr_value);
```

**itimerval结构体：**

```c
struct itimerval {
    struct timeval it_interval; // 间隔
    struct timeval it_value;   // 初始值
};

struct timeval {
    long tv_sec;   // 秒
    long tv_usec;  // 微秒
};
```

**which取值：**

| 值 | 说明 |
|------|------|
| ITIMER_REAL | 真实时间（SIGALRM） |
| ITIMER_VIRTUAL | 用户态（SIGVTALRM） |
| ITIMER_PROF | 用户+内核态（SIGPROF） |

---

## 七、网络编程

### 字节序转换

```c
#include <arpa/inet.h>

uint32_t htonl(uint32_t hostlong);   // 主机->网络(32位)
uint16_t htons(uint16_t hostshort);  // 主机->网络(16位)
uint32_t ntohl(uint32_t netlong);     // 网络->主机(32位)
uint16_t ntohs(uint16_t netshort);    // 网络->主机(16位)
```

---

### IP地址转换

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

in_addr_t inet_addr(const char *cp);                           // 字符串->网络字节序
int inet_aton(const char *cp, struct in_addr *inp);           // 字符串->网络字节序并存入结构体
char *inet_ntoa(struct in_addr in);                           // 网络字节序->字符串
```

---

### 地址结构体

```c
// IPv4地址结构
struct sockaddr_in {
    sa_family_t    sin_family;    // AF_INET
    in_port_t      sin_port;      // 端口号（网络字节序）
    struct in_addr sin_addr;       // IP地址
};

// IP地址结构
struct in_addr {
    in_addr_t       s_addr;       // 32位IPv4地址（网络字节序）
};

// 通用地址结构
struct sockaddr {
    sa_family_t    sin_family;    // 地址类型
    char           sa_data[14];   // 地址数据
};
```

---

### DNS解析

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(
    const char *node,
    const char *service,
    const struct addrinfo *hints,
    struct addrinfo **res
);

void freeaddrinfo(struct addrinfo *res);
```

**addrinfo结构体：**

```c
struct addrinfo {
    int              ai_flags;
    int              ai_family;     // AF_INET/IPv6/AF_UNSPEC
    int              ai_socktype;    // SOCK_STREAM/SOCK_DGRAM
    int              ai_protocol;
    size_t           ai_addrlen;
    struct sockaddr  *ai_addr;       // IP地址指针
    char            *ai_canonname;
    struct addrinfo  *ai_next;       // 下一个结构指针
};
```

---

### socket - 创建套接字

```c
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| domain | int | 协议族（AF_INET/IPv4、AF_INET6/IPv6） |
| type | int | 套接字类型（SOCK_STREAM/TCP、SOCK_DGRAM/UDP） |
| protocol | int | 协议（通常0） |

**返回值：** 成功返回套接字描述符，失败返回-1

---

### bind - 绑定地址

```c
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 套接字描述符 |
| addr | const struct sockaddr * | 地址结构 |
| addrlen | socklen_t | 地址结构长度 |

**返回值：** 成功返回0，失败返回-1

---

### listen - 监听连接

```c
#include <sys/types.h>
#include <sys/socket.h>

int listen(int sockfd, int backlog);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 套接字描述符 |
| backlog | int | 最大连接队列长度 |

**返回值：** 成功返回0，失败返回-1

---

### connect - 请求连接

```c
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 套接字描述符 |
| addr | const struct sockaddr * | 服务器地址 |
| addrlen | socklen_t | 地址结构长度 |

**返回值：** 成功返回0，失败返回-1

---

### accept - 接受连接

```c
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 监听套接字描述符 |
| addr | struct sockaddr * | 客户端地址（可NULL） |
| addrlen | socklen_t * | 地址长度（可NULL） |

**返回值：** 成功返回已连接套接字描述符，失败返回-1

---

### send/recv - TCP发送/接收

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 套接字描述符 |
| buf | void * | 数据缓冲区 |
| len | size_t | 数据长度 |
| flags | int | 标志 |

**flags常用值：**

| 值 | 说明 |
|------|------|
| MSG_OOB | 发送/接收紧急数据 |
| MSG_PEEK | 查看数据但不移除 |
| MSG_DONTWAIT | 非阻塞 |
| MSG_WAITALL | 等待所有数据 |

**返回值：** 成功返回发送/接收字节数，失败返回-1

---

### sendto/recvfrom - UDP发送/接收

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| dest_addr | const struct sockaddr * | 目标地址（UDP） |
| src_addr | struct sockaddr * | 源地址（UDP） |
| addrlen | socklen_t | 地址长度 |

---

### close - 关闭套接字

```c
#include <unistd.h>

int close(int fd);
```

---

### setsockopt - 设置套接字选项

```c
#include <sys/types.h>
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| sockfd | int | 套接字描述符 |
| level | int | 协议层（SOL_SOCKET/IPPROTO_TCP） |
| optname | int | 选项名 |
| optval | const void * | 选项值 |
| optlen | socklen_t | 选项长度 |

**常用选项：**

| optname | 说明 |
|---------|------|
| SO_REUSEADDR | 允许重用地址 |
| SO_KEEPALIVE | 保持连接 |
| SO_RCVBUF | 接收缓冲区大小 |
| SO_SNDBUF | 发送缓冲区大小 |
| SO_RCVLOWAT | 接收缓冲区下限 |
| SO_SNDLOWAT | 发送缓冲区下限 |

---

## 八、IO多路复用

### 1.select

```c
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *exceptfds, struct timeval *timeout);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| nfds | int | 最大文件描述符+1 |
| readfds | fd_set * | 读监听集合 |
| writefds | fd_set * | 写监听集合 |
| exceptfds | fd_set * | 异常监听集合 |
| timeout | struct timeval * | 超时时间（NULL永久阻塞，0非阻塞） |

**返回值：** 成功返回就绪描述符数量，超时返回0，失败返回-1

**fd_set操作：**

```c
void FD_ZERO(fd_set *set);                    // 清空集合
void FD_SET(int fd, fd_set *set);            // 添加描述符
void FD_CLR(int fd, fd_set *set);            // 移除描述符
int  FD_ISSET(int fd, fd_set *set);          // 检查是否在集合中
```

---

### 2.epoll

#### epoll_create - 创建epoll文件描述符

```c
#include <sys/epoll.h>

int epoll_create(int size);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| size | int | 历史参数（>0即可） |

**返回值：** 成功返回epoll文件描述符，失败返回-1

---

#### epoll_ctl - 控制epoll兴趣列表

```c
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| epfd | int | epoll文件描述符 |
| op | int | 操作类型 |
| fd | int | 目标文件描述符 |
| event | struct epoll_event * | 事件（删除时可为NULL） |

**op取值：**

| 值 | 说明 |
|------|------|
| EPOLL_CTL_ADD | 添加 |
| EPOLL_CTL_MOD | 修改 |
| EPOLL_CTL_DEL | 删除 |

**epoll_event结构体：**

```c
struct epoll_event {
    uint32_t     events;   // 事件类型
    epoll_data_t data;     // 用户数据
};

typedef union epoll_data {
    void        *ptr;
    int          fd;       // 文件描述符
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;
```

**events取值：**

| 值 | 说明 |
|------|------|
| EPOLLIN | 可读 |
| EPOLLOUT | 可写 |
| EPOLLET | 边缘触发 |

---

#### epoll_wait - 等待事件

```c
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| epfd | int | epoll文件描述符 |
| events | struct epoll_event * | 就绪事件数组 |
| maxevents | int | 数组最大长度 |
| timeout | int | 超时时间（毫秒，-1永久阻塞） |

**返回值：** 成功返回就绪事件数量，失败返回-1

---

## 九、数据库

### MySQL C API

#### mysql_init - 初始化连接

```c
#include <mysql/mysql.h>

MYSQL *mysql_init(MYSQL *mysql);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL结构（通常NULL自动分配） |

**返回值：** 成功返回MYSQL指针，失败返回NULL

---

#### mysql_real_connect - 建立连接

```c
#include <mysql/mysql.h>

MYSQL *mysql_real_connect(
    MYSQL *mysql,
    const char *host,
    const char *user,
    const char *passwd,
    const char *db,
    unsigned int port,
    const char *unix_socket,
    unsigned long client_flag
);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL结构指针 |
| host | const char * | 主机名/IP（NULL/"localhost"本地） |
| user | const char * | 用户名 |
| passwd | const char * | 密码 |
| db | const char * | 数据库名（可NULL） |
| port | unsigned int | 端口（0默认3306） |
| unix_socket | const char * | Unix套接字路径（通常NULL） |
| client_flag | unsigned long | 标志（通常0） |

**返回值：** 成功返回MYSQL指针，失败返回NULL

---

#### mysql_query - 执行SQL

```c
#include <mysql/mysql.h>

int mysql_query(MYSQL *mysql, const char *query);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL连接指针 |
| query | const char * | SQL语句（不能以分号结尾） |

**返回值：** 成功返回0，失败返回非0

---

#### mysql_store_result - 获取结果集

```c
#include <mysql/mysql.h>

MYSQL_RES *mysql_store_result(MYSQL *mysql);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL连接指针 |

**返回值：** 成功返回MYSQL_RES指针（查询结果），失败或无结果返回NULL

---

#### mysql_free_result - 释放结果集

```c
#include <mysql/mysql.h>

void mysql_free_result(MYSQL_RES *result);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| result | MYSQL_RES * | 结果集指针 |

---

#### mysql_num_rows - 获取行数

```c
#include <mysql/mysql.h>

my_ulonglong mysql_num_rows(MYSQL_RES *result);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| result | MYSQL_RES * | 结果集指针 |

**返回值：** 结果集行数

---

#### mysql_num_fields - 获取列数

```c
#include <mysql/mysql.h>

unsigned int mysql_num_fields(MYSQL_RES *result);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| result | MYSQL_RES * | 结果集指针 |

**返回值：** 结果集列数

---

#### mysql_fetch_row - 获取一行

```c
#include <mysql/mysql.h>

MYSQL_ROW mysql_fetch_row(MYSQL_RES *result);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| result | MYSQL_RES * | 结果集指针 |

**返回值：** 成功返回行数据（字符串数组），无更多行或出错返回NULL

---

#### mysql_close - 关闭连接

```c
#include <mysql/mysql.h>

void mysql_close(MYSQL *mysql);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL连接指针 |

---

#### mysql_error - 获取错误信息

```c
#include <mysql/mysql.h>

const char *mysql_error(MYSQL *mysql);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| mysql | MYSQL * | MYSQL连接指针 |

**返回值：** 错误描述字符串

---

## 附录

### 常用类型别名

| 类型 | 实际类型 | 说明 |
|------|----------|------|
| size_t | unsigned int/64 | 无符号大小 |
| ssize_t | int/64 | 有符号大小 |
| pid_t | int | 进程ID |
| uid_t | unsigned int | 用户ID |
| gid_t | unsigned int | 组ID |
| mode_t | unsigned int | 文件权限 |
| off_t | long/64 | 文件偏移 |
| time_t | long | 时间戳 |
| socklen_t | unsigned int | 地址长度 |

### 常用头文件

| 头文件 | 用途 |
|--------|------|
| `<unistd.h>` | 系统调用（read/write/fork等） |
| `<sys/types.h>` | 系统数据类型 |
| `<sys/stat.h>` | 文件状态 |
| `<sys/wait.h>` | 进程等待 |
| `<pthread.h>` | 线程 |
| `<signal.h>` | 信号 |
| `<sys/socket.h>` | 套接字 |
| `<netinet/in.h>` | 网络地址 |
| `<arpa/inet.h>` | IP转换 |
| `<sys/epoll.h>` | epoll |
| `<fcntl.h>` | 文件控制 |
| `<dirent.h>` | 目录 |
| `<sys/mman.h>` | 内存映射 |
| `<mysql/mysql.h>` | MySQL |

---

### 常见错误码

| 错误码       | 说明               |
| ------------ | ------------------ |
| EINVAL       | 无效参数           |
| ENOENT       | 文件不存在         |
| EACCES       | 权限不足           |
| EAGAIN       | 资源不可用（重试） |
| EBUSY        | 资源忙             |
| ENOMEM       | 内存不足           |
| EMFILE       | 文件描述符耗尽     |
| ECONNREFUSED | 连接被拒绝         |
| ETIMEDOUT    | 连接超时           |