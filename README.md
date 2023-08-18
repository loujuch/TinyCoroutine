# TinyCoroutine

## 抽象结构

### 协程对象(Coroutine)

核心对象，代表一个协程对象。

要求一个有效协程至少具有以下资源：

1. 可执行函数

2. 栈空间

3. 上下文

```
必须API：

1. void resume(): 恢复该线程对象。

2. void yield(): 挂起该线程对象。

3. void exit(): 结束该线程对象的执行。

4. Status status() const: 获取线程当前状态。
```

### 可执行函数(Functor)

采用 C++ 11 的函数对象`std::function<void()>`。

### 栈空间(StackSpace)

```
co::SharedStack stk(1024*1024*1024);
co::Attribute attr((StackSpace &)stk); // 保存为 
co::Coroutine co0;
co::Coroutine co1((StackSpace &)attr, func);
if(attr.vaild()) {
	co::Coroutine co0(co::Coroutine(attr, func));
}
```

代表一段内存空间，可以在栈上，也可以在堆上。

```
必须API：

1. char* top() const: 返回堆栈的栈顶。

2. std::ptrdiff_t capacity() const: 返回堆栈的容量。

3. bool shared() const: 返回该栈的类型。

4. std::unique_ptr<co::StackSpace> data(): 返回一个指向于该栈具有相同内容的的指针，不要求可重入。
```

`StackSpace`为一个抽象类，该库实现了其对应的两个子类`SharedStack`、`UniqueStack`，这两个类都是在堆中分配内存。

问题在于，共享栈的赋值应当是深拷贝(拷贝)，而独占栈的赋值应当是浅拷贝(移动)。

#### 共享栈(SharedStack)

用于空间共享，在协程切换时，若发生冲突，则将其中内容复制，在切回时恢复。

#### 独占栈(UniqueStack)

为独占空间，在

## 特殊情况处理

约定以下术语：

1. 主协程：为协程环境的一部分。当一个线程构建协程环境时，当前的上下文自动被创建为主协程。

2. 父协程：协程间的调用关系。当一个协程调用另一个协程时，该协程就是被调用协程的父协程，被调用协程即为该协程的子协程。

3. 子协程：详见`父协程`。子协程退出后只可以返回其对应的父协程。

4. 当前协程：当前正在运行的协程，只在协程环境创建后存在意义。

5. 死协程：当前函数执行完成后的协程。

6. 空协程：不含有执行函数的协程。

### 协程的线程间转移或共享

该协程库不允许协程在线程间共享。

只允许协程在被绑定前(第一次调用`resume`)前进行线程间的移动。

### 图状协程调用

正常情况下，所有协程对象将能够由调用和被调用关系构成一个调用树。但使用移动、引用等方法，可能使得子协程能够调用自己的祖先，使得调用关系变为图结构。

图状协程调用可能会产生一些较严重的问题：

1. 耗尽问题：一个协程可能在作为自身的子协程时完成内部函数的运行，于是在子协程返回时运行死协程。

2. 上下文问题：要求协程对象有足够的空间整理返回对象。

该库使用(外部协程调用栈+链表)的方式解决上下文问题，使用返回检测的方式解决调用问题。

### 协程环境创建时机

有以下选择：

1. 线程创建时自动创建。

2. 用户手动创建。

3. 第一次调用需协程环境函数时自动创建。

4. 第一次创建有效协程对象时自动创建。

该库选择在`第一次创建有效协程对象`时刻创建协程环境(因为至少需要有两个协程才有管理的必要)。

### 协程环境撤消时机

有以下选择：

1. 线程退出自动撤销。

2. 用户手动撤销。

3. 全部非主协程生命周期结束后自动撤销。

该库选择在`全部非主协程生命周期结束`时刻撤消协程环境。

### 异常抛出情况

以下情况会抛出`CoroutineException`异常：

1. 恢复当前正运行协程。

2. 挂起已挂起协程。

3. 挂起/退出主协程。

4. 恢复空协程或死协程。

5. 在无协程环境时调用协程函数。