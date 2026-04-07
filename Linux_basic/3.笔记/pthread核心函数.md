# pthread 核心函数笔记

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

| 参数 | 类型 | 说明 |
|------|------|------|
| `thread` | `pthread_t *` | **输出参数**，成功后存储新线程的线程 ID（TID） |
| `attr` | `const pthread_attr_t *` | 线程属性对象，传 `NULL` 使用默认属性（joinable、默认栈大小、继承调度策略） |
| `start_routine` | `void *(*)(void *)` | 线程入口函数，签名必须为 `void *func(void *)` |
| `arg` | `void *` | 传给 `start_routine` 的参数，可传结构体指针以携带多个值；不需要时传 `NULL` |

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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `thread` | `pthread_t` | 要等待的目标线程 ID |
| `retval` | `void **` | **输出参数**，接收目标线程的退出值（即 `pthread_exit` 的参数或 `return` 的值）；不关心退出值时传 `NULL` |

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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 类别 | 函数 |
|------|------|
| pthread 函数 | `pthread_join`、`pthread_cond_wait`、`pthread_cond_timedwait`、`pthread_testcancel` |
| 文件 I/O | `read`、`write`、`open`、`close`、`select`、`poll` |
| 时间 | `sleep`、`nanosleep`、`usleep` |
| 网络 | `accept`、`connect`、`recv`、`send` |

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

| 参数 | 类型 | 说明 |
|------|------|------|
| `mutex` | `pthread_mutex_t *` | 要初始化的互斥锁对象指针 |
| `attr` | `const pthread_mutexattr_t *` | 互斥锁属性，`NULL` 使用默认属性（普通类型、进程私有） |

#### 互斥锁类型（通过 attr 设置）

| 类型宏 | 说明 |
|--------|------|
| `PTHREAD_MUTEX_NORMAL` | 默认类型，同一线程重复加锁会**死锁** |
| `PTHREAD_MUTEX_ERRORCHECK` | 检错类型，重复加锁返回错误 `EDEADLK` |
| `PTHREAD_MUTEX_RECURSIVE` | 递归类型，同一线程可重复加锁（需对应次数解锁） |
| `PTHREAD_MUTEX_DEFAULT` | 实现定义，通常等同于 `NORMAL` |

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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `cond` | `pthread_cond_t *` | 要初始化的条件变量对象指针 |
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `cond` | `pthread_cond_t *` | 要等待的条件变量 |
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `cond` | `pthread_cond_t *` | 要等待的条件变量 |
| `mutex` | `pthread_mutex_t *` | 关联的互斥锁，调用前必须已持有 |
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `cond` | `pthread_cond_t *` | 要发送通知的条件变量 |

#### 返回值

- **`0`**：成功（无论是否有线程在等待，所以会有虚假唤醒的情况）
- **`EINVAL`**：cond 未初始化

#### 使用要点

1. **唤醒"至少一个"**：POSIX 规定唤醒至少一个等待线程，实际上通常只唤醒一个，但不保证。适用于**资源只够一个线程使用**的场景（如生产一个消息通知一个消费者）。
2. **无记忆性（使用while解决虚假唤醒的问题）**：若发出 signal 时无线程在等待，信号**丢失**，后来进入 wait 的线程不会被这个旧信号唤醒。这也是为什么要用 while 循环保护条件检查。
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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 参数 | 类型 | 说明 |
|------|------|------|
| `routine` | `void (*)(void *)` | 清理函数指针，签名为 `void func(void *)` |
| `arg` | `void *` | 传递给 `routine` 的参数（如互斥锁指针、分配的内存地址等） |

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

| 参数 | 类型 | 说明 |
|------|------|------|
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

| 函数 | 作用 | 关键入参 | 返回值要点 |
|------|------|----------|------------|
| `pthread_create` | 创建新线程 | `thread`（输出TID）、`attr`（属性/NULL）、`start_routine`（入口函数）、`arg`（参数） | `0` 成功；`EAGAIN` 资源不足；`EINVAL` attr无效 |
| `pthread_exit` | 终止当前线程 | `retval`（退出值，不可指向栈变量） | 无返回（不返回） |
| `pthread_join` | 等待线程结束并回收资源 | `thread`（目标线程ID）、`retval`（输出：退出值/NULL） | `0` 成功；`EDEADLK` 死锁；`EINVAL` 不可汇合；`ESRCH` 线程不存在 |
| `pthread_cancel` | 发送取消请求 | `thread`（目标线程ID） | `0` 请求已发送；`ESRCH` 线程不存在 |

### 5.2 互斥锁函数

| 函数 | 作用 | 关键入参 | 返回值要点 |
|------|------|----------|------------|
| `pthread_mutex_init` | 初始化互斥锁 | `mutex`（锁指针）、`attr`（属性/NULL） | `0` 成功；`EINVAL` attr无效；`ENOMEM` 内存不足 |
| `pthread_mutex_lock` | 加锁（阻塞等待） | `mutex`（锁指针） | `0` 成功获锁；`EINVAL` 未初始化；`EDEADLK` 重复加锁（检错模式） |
| `pthread_mutex_trylock` | 尝试加锁（非阻塞） | `mutex`（锁指针） | `0` 成功获锁；`EBUSY` 锁已被占用（正常，不是错误） |
| `pthread_mutex_unlock` | 解锁 | `mutex`（锁指针） | `0` 成功；`EINVAL` 未初始化；`EPERM` 非锁持有者解锁（检错模式） |
| `pthread_mutex_destroy` | 销毁互斥锁 | `mutex`（锁指针） | `0` 成功；`EBUSY` 锁仍被持有或有线程等待；`EINVAL` 无效 |

### 5.3 条件变量函数

| 函数 | 作用 | 关键入参 | 返回值要点 |
|------|------|----------|------------|
| `pthread_cond_init` | 初始化条件变量 | `cond`（cond指针）、`attr`（属性/NULL） | `0` 成功；`EINVAL` attr无效；`ENOMEM` 内存不足 |
| `pthread_cond_wait` | 原子释放锁并等待通知（无超时） | `cond`（cond指针）、`mutex`（必须预先持有） | `0` 被唤醒（需 while 再检查条件）；`EINVAL` 参数无效 |
| `pthread_cond_timedwait` | 带超时的条件等待 | `cond`、`mutex`、`abstime`（**绝对**超时时间戳） | `0` 被唤醒；`ETIMEDOUT` 超时；`EINVAL` 参数无效 |
| `pthread_cond_signal` | 唤醒至少一个等待线程 | `cond`（cond指针） | `0` 成功（即使无线程在等待也返回0）；`EINVAL` 未初始化 |
| `pthread_cond_broadcast` | 唤醒所有等待线程 | `cond`（cond指针） | `0` 成功（即使无线程在等待也返回0）；`EINVAL` 未初始化 |
| `pthread_cond_destroy` | 销毁条件变量 | `cond`（cond指针） | `0` 成功；`EBUSY` 有线程在等待；`EINVAL` 无效 |

### 5.4 线程清理处理程序

| 函数/宏 | 作用 | 关键入参 | 返回值要点 |
|---------|------|----------|------------|
| `pthread_cleanup_push` | 向清理栈压入清理函数 | `routine`（清理函数）、`arg`（传给routine的参数） | 无（宏，无返回值） |
| `pthread_cleanup_pop` | 弹出（并可执行）栈顶清理函数 | `execute`（非零=执行后弹出，零=仅弹出） | 无（宏，无返回值） |

### 5.5 关键规则速记

| 场景 | 正确做法 |
|------|----------|
| 条件变量等待 | **while 循环**检查条件，防虚假唤醒 |
| 线程退出值 | 不能指向线程栈局部变量，用堆或全局存储 |
| joinable 线程 | 必须 `pthread_join` 或 `pthread_detach`，否则资源泄漏 |
| 多锁加锁顺序 | 所有线程保持相同加锁顺序，防死锁 |
| `pthread_cond_timedwait` 超时 | `abstime` 是绝对时间戳，用 `clock_gettime` + 偏移量计算 |
| `pthread_cancel` 清理 | 配合 `pthread_cleanup_push` 注册解锁/释放函数 |
| 静态初始化 | 全局/静态 mutex/cond 用 `_INITIALIZER` 宏，无需 `init`/`destroy` |
| `pthread_cleanup_push/pop` | 必须在同一词法作用域内成对出现（宏展开含括号） |
