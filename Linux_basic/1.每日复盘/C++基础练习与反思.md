# 基础练习与反思

这份笔记的目标不是背概念，而是把每个基础点都回答成三个问题：

1. 它解决什么问题？
2. 它在代码里怎么写？
3. 初学时最容易错在哪里？

## 第一章：C与C++

### 1. 一个基本 C++ 源文件包含哪些内容？

一个最小的 C++ 程序通常包含：

- 头文件引入：告诉编译器要使用哪些库。
- 命名空间处理：避免每次都写很长的名字。
- 函数或类的声明与定义：写具体功能。
- `main` 函数：程序入口。

```cpp
#include <iostream>

int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(3, 4);
    std::cout << result << std::endl;
    return 0;
}
```

编译执行：

```bash
g++ main.cpp -o main
./main
```

如果拆成头文件和源文件，通常这样组织：

```text
project/
  main.cpp       // 程序入口
  student.h      // 类、函数的声明
  student.cpp    // 类、函数的实现
```

常见原则：

- `.h` 或 `.hpp` 放声明，说明“有什么”。
- `.cpp` 放实现，说明“怎么做”。
- 可执行程序必须有且只有一个 `main` 函数入口。

### 2. 命名空间 namespace

命名空间的主要作用是避免名字冲突。

假设两个库里都有一个叫 `print` 的函数，如果没有命名空间，编译器不知道你想调用哪一个。

```cpp
namespace A {
    void print() {
        std::cout << "A::print" << std::endl;
    }
}

namespace B {
    void print() {
        std::cout << "B::print" << std::endl;
    }
}

int main() {
    A::print();
    B::print();
    return 0;
}
```

命名空间的使用方式常见有三种。

第一种：每次都写完整名字，最清晰。

```cpp
std::cout << "hello" << std::endl;
```

第二种：引入某一个名字。

```cpp
using std::cout;
using std::endl;

cout << "hello" << endl;
```

第三种：引入整个命名空间。

```cpp
using namespace std;

cout << "hello" << endl;
```

初学阶段可以在小练习里用 `using namespace std;`，但在真实项目或头文件中不推荐这样写，因为它容易把很多名字一次性暴露出来，引发冲突。

同一个作用域中可以多次定义同名命名空间，它们会自动合并。

```cpp
namespace Test {
    int a = 10;
}

namespace Test {
    int b = 20;
}

int main() {
    std::cout << Test::a << " " << Test::b << std::endl;
}
```

### 3. const 关键字

`const` 表示“只读”。被 `const` 修饰的对象不能通过这个名字被修改。

```cpp
const int n = 10;
// n = 20; // 错误
```

对于普通内置类型，`const` 的特点是：

- 必须初始化。
- 初始化后不能修改。
- 编译器可以利用它做安全检查和优化。

```cpp
const double pi = 3.14159;
```

#### const 修饰指针

理解 `const` 指针时，可以先看 `const` 离谁近。

`const int* p`：指向常量的指针。

```cpp
int a = 10;
int b = 20;

const int* p = &a;
// *p = 30; // 错误，不能通过 p 修改 a
p = &b;    // 正确，p 可以指向别的变量
```

`int const* p` 和 `const int* p` 含义相同。

```cpp
int const* p = &a;
```

`int* const p`：常量指针。

```cpp
int a = 10;
int b = 20;

int* const p = &a;
*p = 30;   // 正确，可以修改指向的值
// p = &b; // 错误，p 自己不能再指向别人
```

`const int* const p`：指向常量的常量指针。

```cpp
int a = 10;

const int* const p = &a;
// *p = 20; // 错误，不能改值
// p = nullptr; // 错误，不能改指向
```

记忆方式：

- `const` 在 `*` 左边：指向的值不能改。
- `const` 在 `*` 右边：指针变量本身不能改。

### 4. new/delete 与 malloc/free

`malloc/free` 是 C 语言的内存管理方式，`new/delete` 是 C++ 的内存管理方式。

```cpp
int* p1 = (int*)malloc(sizeof(int));
free(p1);

int* p2 = new int(10);
delete p2;
```

主要区别：

| 对比点 | `new/delete` | `malloc/free` |
| --- | --- | --- |
| 所属语言 | C++ | C |
| 返回类型 | 返回具体类型指针 | 返回 `void*`，常需要强转 |
| 构造函数 | 会调用构造函数 | 不会 |
| 析构函数 | `delete` 会调用析构函数 | 不会 |
| 失败行为 | 默认抛出异常 | 返回 `NULL` |

创建数组时要配对使用 `new[]` 和 `delete[]`：

```cpp
int* arr = new int[10];
delete[] arr;
```

不能混用：

```cpp
int* p = new int;
// free(p); // 错误

int* q = (int*)malloc(sizeof(int));
// delete q; // 错误
```

### 5. 引用

引用可以理解为变量的别名。

```cpp
int a = 10;
int& r = a;

r = 20;
std::cout << a << std::endl; // 20
```

引用的特点：

- 定义时必须初始化。
- 初始化后不能再绑定到其他变量。
- 对引用的操作就是对原变量的操作。

引用的本质可以理解为：编译器帮我们隐藏了一层指针，但使用形式更像普通变量。

对引用取地址，得到的是原变量的地址。

```cpp
int a = 10;
int& r = a;

std::cout << &a << std::endl;
std::cout << &r << std::endl; // 与 &a 相同
```

### 6. 引用和指针的区别

| 对比点 | 引用 | 指针 |
| --- | --- | --- |
| 是否必须初始化 | 必须 | 不必须 |
| 是否可以为空 | 正常引用不能为空 | 可以是 `nullptr` |
| 是否可以改变绑定目标 | 不可以 | 可以 |
| 使用方式 | 像普通变量 | 需要 `*` 解引用 |
| 常见用途 | 函数参数、返回值 | 动态内存、链表、数组、底层操作 |

引用常用于函数传参，避免拷贝，也能让函数修改外部变量。

```cpp
void swapValue(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}
```

如果不想函数修改参数，可以使用常引用。

```cpp
void printName(const std::string& name) {
    std::cout << name << std::endl;
}
```

### 7. 函数重载

函数重载是指：同一个作用域中，函数名相同，但参数列表不同。

```cpp
int add(int a, int b) {
    return a + b;
}

double add(double a, double b) {
    return a + b;
}

int add(int a, int b, int c) {
    return a + b + c;
}
```

形参列表不同可以表现为：

- 参数个数不同。
- 参数类型不同。
- 参数顺序不同。

```cpp
void test(int a, double b) {}
void test(double a, int b) {}
```

只有返回值不同不能构成重载。

```cpp
// int func(int a) {}
// double func(int a) {} // 错误
```

函数重载的原理是：C++ 编译器会对函数名进行改编，把参数类型也编码到符号名中。因此同名函数在底层其实会变成不同的符号。

### 8. 内联函数 inline

内联函数的想法是：把函数调用处直接替换成函数体，减少函数调用开销。

```cpp
inline int square(int x) {
    return x * x;
}
```

适合使用内联函数的场景：

- 函数体很短。
- 调用非常频繁。
- 逻辑简单，例如 getter、setter、小计算函数。

不适合使用内联函数的场景：

- 函数体很长。
- 有复杂循环或递归。
- 函数逻辑经常变化，导致编译依赖扩大。

注意：`inline` 是给编译器的建议，编译器可以选择不内联。

### 9. C++ 异常处理

C++ 用 `throw`、`try`、`catch` 处理异常。

```cpp
int divide(int a, int b) {
    if (b == 0) {
        throw std::runtime_error("除数不能为 0");
    }
    return a / b;
}

int main() {
    try {
        std::cout << divide(10, 0) << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
```

三个关键字的作用：

- `throw`：抛出异常。
- `try`：包住可能出问题的代码。
- `catch`：捕获并处理异常。

如果异常没有被处理，程序通常会终止。

如果异常被合适的 `catch` 捕获，程序可以继续执行。

初学时要注意：异常适合处理“异常情况”，不应该用来代替普通的 `if/else` 流程控制。

### 10. C++ 程序的内存布局

一个程序运行时，内存通常可以粗略分为这些区域：

| 区域 | 存放内容 | 特点 |
| --- | --- | --- |
| 代码区 | 程序指令 | 通常只读 |
| 全局/静态区 | 全局变量、静态变量 | 程序开始时创建，程序结束时释放 |
| 常量区 | 字符串常量、部分常量数据 | 通常不能修改 |
| 栈区 | 局部变量、函数参数 | 自动创建和释放 |
| 堆区 | `new`、`malloc` 申请的内存 | 需要手动释放，或交给智能指针管理 |

示例：

```cpp
int globalValue = 10; // 全局/静态区

int main() {
    int localValue = 20;        // 栈区
    int* heapValue = new int;   // heapValue 变量在栈区，指向的 int 在堆区
    const char* text = "hello"; // 字符串字面量通常在常量区

    delete heapValue;
    return 0;
}
```

初学时最重要的是区分：

- 栈上的变量离开作用域会自动释放。
- 堆上的内存不会自动释放，忘记释放会导致内存泄漏。

## 第二章：类与对象

### 1. 类和对象

类是对一类事物的抽象描述，说明它有什么数据和行为。

对象是根据类创建出来的具体实例。

```cpp
#include <iostream>
#include <string>

class Student {
public:
    std::string name;
    int age;

    void study() {
        std::cout << name << " 正在学习" << std::endl;
    }
};

int main() {
    Student s;
    s.name = "张三";
    s.age = 18;
    s.study();

    return 0;
}
```

可以这样理解：

```text
类：学生这个概念
对象：张三、李四这些具体学生
```

### 2. 权限修饰符 public 和 private

权限修饰符控制类成员能不能被外部访问。

`public`：类外可以访问。

```cpp
class Student {
public:
    std::string name;
};

int main() {
    Student s;
    s.name = "张三";
}
```

`private`：只能在类内部访问。

```cpp
class Student {
private:
    int age;

public:
    void setAge(int value) {
        if (value >= 0) {
            age = value;
        }
    }

    int getAge() {
        return age;
    }
};
```

外部不能直接访问：

```cpp
Student s;
// s.age = 18; // 错误
s.setAge(18);
```

`private` 的意义不是“麻烦”，而是保护对象状态，避免外部随意修改导致对象不合法。

### 3. struct 和 class 的区别

在 C++ 中，`struct` 和 `class` 都可以定义成员变量和成员函数。

主要区别是默认访问权限不同：

- `struct` 默认是 `public`。
- `class` 默认是 `private`。

```cpp
struct A {
    int x; // 默认 public
};

class B {
    int x; // 默认 private
};
```

使用习惯：

- `struct` 常用于简单数据聚合。
- `class` 常用于有封装、有行为、有不变量的对象。

### 4. 构造函数

构造函数在创建对象时自动调用，用来初始化对象。

特点：

- 函数名和类名相同。
- 没有返回值，连 `void` 也不能写。
- 创建对象时自动调用。

```cpp
class Student {
private:
    std::string name;
    int age;

public:
    Student() {
        name = "未知";
        age = 0;
    }

    Student(const std::string& n, int a) {
        name = n;
        age = a;
    }
};
```

更推荐使用初始化列表：

```cpp
class Student {
private:
    std::string name;
    int age;

public:
    Student(const std::string& n, int a)
        : name(n), age(a) {
    }
};
```

什么时候会调用构造函数：

```cpp
Student s1;              // 调用无参构造
Student s2("张三", 18);  // 调用有参构造
```

### 5. 拷贝构造函数

拷贝构造函数用已有对象创建新对象。

```cpp
class Student {
public:
    std::string name;
    int age;

    Student(const std::string& n, int a) : name(n), age(a) {}

    Student(const Student& other) {
        name = other.name;
        age = other.age;
    }
};

int main() {
    Student s1("张三", 18);
    Student s2 = s1; // 调用拷贝构造函数
}
```

拷贝构造函数的常见形式：

```cpp
ClassName(const ClassName& other);
```

为什么参数要用引用？

如果不用引用，传参时又要拷贝一次对象，会继续调用拷贝构造函数，导致无限递归。

什么时候会调用拷贝构造：

- 用一个对象初始化另一个新对象。
- 对象按值传参。
- 函数按值返回对象时，某些情况下可能发生拷贝或移动。

### 6. 浅拷贝和深拷贝

浅拷贝只复制成员变量的值。如果成员变量里有指针，就只复制地址，两个对象会指向同一块内存。

```cpp
class Array {
public:
    int* data;

    Array(int value) {
        data = new int(value);
    }

    ~Array() {
        delete data;
    }
};
```

如果直接使用编译器生成的默认拷贝：

```cpp
Array a(10);
Array b = a; // 浅拷贝，a.data 和 b.data 指向同一块内存
```

问题：两个对象析构时会 `delete` 同一块内存，导致错误。

深拷贝会重新申请一块内存，再复制内容。

```cpp
class Array {
public:
    int* data;

    Array(int value) {
        data = new int(value);
    }

    Array(const Array& other) {
        data = new int(*other.data);
    }

    ~Array() {
        delete data;
    }
};
```

只要类中管理堆内存，就要认真考虑拷贝构造、赋值运算符和析构函数。

### 7. 析构函数

析构函数在对象销毁时自动调用，用来释放资源。

特点：

- 函数名是 `~类名`。
- 没有返回值。
- 没有参数。
- 一个类只能有一个析构函数。

```cpp
class File {
public:
    File() {
        std::cout << "打开资源" << std::endl;
    }

    ~File() {
        std::cout << "释放资源" << std::endl;
    }
};
```

析构函数一般用来：

- 释放 `new` 出来的内存。
- 关闭文件。
- 关闭网络连接。
- 释放锁或其他系统资源。

### 8. this 指针

`this` 是成员函数内部自动存在的指针，指向当前对象。

```cpp
class Student {
private:
    std::string name;

public:
    void setName(const std::string& name) {
        this->name = name;
    }
};
```

这里左边的 `this->name` 是成员变量，右边的 `name` 是参数。

`this` 的本质：

- 每个非静态成员函数都有一个隐藏的 `this` 指针参数。
- `this` 指向调用该成员函数的对象。
- 静态成员函数没有 `this` 指针。

### 9. 赋值运算符重载

赋值运算符用于“已有对象给已有对象赋值”。

```cpp
Student s1("张三", 18);
Student s2("李四", 20);
s2 = s1; // 调用赋值运算符
```

它和拷贝构造不同：

```cpp
Student s2 = s1; // 创建新对象，调用拷贝构造
s2 = s1;         // 两个对象都已经存在，调用赋值运算符
```

常见写法：

```cpp
class Array {
private:
    int* data;

public:
    Array(int value) {
        data = new int(value);
    }

    Array(const Array& other) {
        data = new int(*other.data);
    }

    Array& operator=(const Array& other) {
        if (this == &other) {
            return *this;
        }

        delete data;
        data = new int(*other.data);
        return *this;
    }

    ~Array() {
        delete data;
    }
};
```

需要注意：

- 返回 `Array&`，方便连续赋值：`a = b = c;`
- 要处理自赋值：`a = a;`
- 旧资源释放后再拷贝新资源。

### 10. 特殊成员的初始化

有些成员必须在初始化列表中初始化。

例如 `const` 成员：

```cpp
class Student {
private:
    const int id;

public:
    Student(int value) : id(value) {
    }
};
```

引用成员也必须在初始化列表中初始化：

```cpp
class Holder {
private:
    int& ref;

public:
    Holder(int& value) : ref(value) {
    }
};
```

对象成员如果没有默认构造函数，也需要在初始化列表中初始化。

```cpp
class Address {
public:
    Address(const std::string& city) {
    }
};

class Student {
private:
    Address address;

public:
    Student() : address("北京") {
    }
};
```

总结：能用初始化列表时，优先使用初始化列表。

### 11. static 成员

静态成员属于类，不属于某一个对象。

```cpp
class Student {
public:
    static int count;

    Student() {
        count++;
    }
};

int Student::count = 0;
```

访问静态成员：

```cpp
std::cout << Student::count << std::endl;
```

静态成员函数也属于类：

```cpp
class Student {
private:
    static int count;

public:
    static int getCount() {
        return count;
    }
};
```

静态成员函数能访问：

- 静态成员变量。
- 其他静态成员函数。

静态成员函数不能直接访问：

- 非静态成员变量。
- 非静态成员函数。

原因是静态成员函数没有 `this` 指针，不知道你想访问哪个对象的普通成员。

### 12. const 成员函数

`const` 成员函数表示：这个函数不会修改当前对象的普通成员变量。

```cpp
class Student {
private:
    std::string name;

public:
    std::string getName() const {
        return name;
    }
};
```

`const` 写在函数参数列表后面：

```cpp
返回类型 函数名(参数列表) const;
```

特点：

- `const` 成员函数内部不能修改普通成员变量。
- `const` 对象只能调用 `const` 成员函数。
- 非 `const` 对象既可以调用 `const` 成员函数，也可以调用非 `const` 成员函数。

```cpp
class Student {
public:
    void read() const {}
    void write() {}
};

int main() {
    const Student s1;
    s1.read();
    // s1.write(); // 错误

    Student s2;
    s2.read();
    s2.write();
}
```

### 13. 类对象的大小

一个对象的大小主要取决于它的非静态成员变量。

```cpp
class A {
public:
    int x;
    char y;
};
```

对象大小还会受内存对齐影响，所以不一定等于所有成员大小的简单相加。

```cpp
std::cout << sizeof(A) << std::endl;
```

不影响普通对象大小的内容：

- 成员函数：代码放在代码区，不在每个对象里复制一份。
- 静态成员变量：属于类，不属于单个对象。
- 静态成员函数：属于类，不属于单个对象。

空类对象大小通常是 1 字节。这样不同对象才能有不同地址。

```cpp
class Empty {};

std::cout << sizeof(Empty) << std::endl; // 通常是 1
```

### 14. 类中的 new 和 delete

使用 `new` 创建对象时，会做两件事：

1. 申请内存。
2. 调用构造函数。

```cpp
Student* p = new Student("张三", 18);
```

使用 `delete` 释放对象时，也会做两件事：

1. 调用析构函数。
2. 释放内存。

```cpp
delete p;
```

数组对象要使用 `new[]` 和 `delete[]`：

```cpp
Student* arr = new Student[3];
delete[] arr;
```

初学时要牢记：申请和释放必须成对出现。

### 15. 单例模式

单例模式的目标是：一个类在整个程序中只有一个对象。

实现步骤：

1. 构造函数私有化，禁止外部随便创建对象。
2. 类内部保存一个静态对象或静态指针。
3. 提供一个公共静态函数返回这个唯一对象。
4. 禁止拷贝和赋值，避免产生第二个对象。

简单写法：

```cpp
class Singleton {
private:
    Singleton() = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
};
```

使用：

```cpp
Singleton& s = Singleton::getInstance();
```

这是一种常见且简洁的写法，局部静态对象会在第一次调用时创建。

### 16. vector

`vector` 是 C++ 标准库中的动态数组。

```cpp
#include <vector>

int main() {
    std::vector<int> nums;
    nums.push_back(10);
    nums.push_back(20);
    nums.push_back(30);
}
```

它的特点：

- 底层是一段连续内存。
- 支持下标访问。
- 可以自动扩容。
- 适合频繁尾部插入和随机访问。

底层结构可以理解为：

```text
begin 指向第一个元素
end 指向最后一个元素的下一个位置
capacity_end 指向已申请空间的末尾
```

常用函数：

```cpp
std::vector<int> v;

v.push_back(1);
v.push_back(2);

std::cout << v.size() << std::endl;     // 当前元素个数
std::cout << v.capacity() << std::endl; // 当前容量
std::cout << v[0] << std::endl;         // 不检查越界
std::cout << v.at(0) << std::endl;      // 检查越界
```

扩容机制：

- 当 `size()` 超过当前 `capacity()` 时，`vector` 会申请更大的连续空间。
- 然后把旧元素搬到新空间。
- 最后释放旧空间。

因此，扩容可能导致原来的指针、引用、迭代器失效。

```cpp
std::vector<int> v;
v.push_back(1);

int* p = &v[0];
v.push_back(2);
v.push_back(3);
// p 可能已经失效，不能继续依赖它
```

如果提前知道大概元素个数，可以使用 `reserve` 减少扩容次数。

```cpp
std::vector<int> v;
v.reserve(100);
```

### 本章反思

第一章的重点是理解 C++ 相比 C 增加了哪些更安全、更抽象的机制：命名空间解决命名冲突，引用简化间接访问，函数重载提高表达能力，`new/delete` 配合构造析构管理对象生命周期，异常提供错误传播方式。

第二章的重点是理解“对象不仅是数据的集合，也是行为和资源的管理者”。构造函数负责初始化，析构函数负责清理，拷贝构造和赋值运算符决定对象如何复制，`private` 和 `const` 帮助我们写出更可靠的类。
