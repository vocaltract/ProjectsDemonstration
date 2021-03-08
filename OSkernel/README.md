# 项目说明
项目基于`abstract-machine`框架，`kernel`目录下为本人完成的内容。关于本人代码实现方法的进一步说明可以见`$PATH_TO_PROJECT/kernel/report.pdf`。

本项目同时兼容32位与64位。

## AbstractMachine
AbstractMachine作为框架代码，提供以下十三个API以及一段可直接访问的原始内存，本人代码在这些API上完成。因此需要基于`_atomic_xchg`实现互斥/自旋锁，信号量等。
```c
// ====================== Turing Machine (TRM) =======================

extern _Area _heap;
void _putc(char ch);
void _halt(int code) __attribute__((__noreturn__));

// ======================= I/O Extension (IOE) =======================

int    _ioe_init();
size_t _io_read (uint32_t dev, uintptr_t reg, void *buf, size_t size);
size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size);

// ====================== Context Extension (CTE) ====================

int  _cte_init(_Context *(*handler)(_Event ev, _Context *ctx));
void _yield();
int  _intr_read();
void _intr_write(int enable);
_Context* _kcontext(_Area kstack, void (*entry)(void *), void *arg);

// ================= Virtual Memory Extension (VME) ==================

int  _vme_init(void *(*pgalloc)(size_t size), void (*pgfree)(void *));
void _protect(_AddressSpace *as);
void _unprotect(_AddressSpace *as);
void _map(_AddressSpace *as, void *va, void *pa, int prot);
_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry);

// ================= Multi-Processor Extension (MPE) =================

int _mpe_init(void (*entry)());
int _ncpu();
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);
```

## 正确性与实现
由于项目的正确性由老师一些没有公开极强benchmark与测试样例保证。
### 内存分配
#### 实现方法
借鉴Google的`slab`的思想，利用链表串起的位图实现，具体见`$PATH_TO_PROJECT/kernel/report.pdf`。
对于内存分配，包括以下四个benchmark和正确性检查。
* 频繁4KB内存分配
* 频繁1B-64B内存分配
* 混合内存分配
* 一个真实内存分配的benchmark
#### 正确性检查
`double alloc`,`double free`,`memory leaking`等，由OJ上的一个并发数据结构进行检查。
### kernel线程调度
#### 实现方法
为了应对多处理器的多个线程调度器的数据竞争，虽然是简单的ROUND ROBIN，但是需要应对`stack race`问题，见`$PATH_TO_PROJECT/kernel/report.pdf`。
#### 正确性检查
由OJ进行检查，包括是否发生各类异常，以及一个特殊要求：在充分长的一段时间后，某个线程应该被调度到每个处理器上至少一次。


## 编译与运行
由于没有老师的测试样例源码，运行后仅有一个简单的打印匹配嵌套`K`层括号的展示。
### 依赖
* `gcc-7`或更**低**版本。

由于框架`abstract-machine`使用了一个`C`的奇技淫巧（见下文），它不被高版本`gcc`支持，因此请使用`gcc-7`或更低版本来编译该项目。

    函数名与已初始化的全局变量被称为强符号，未初始化的全局变量被称为弱符号。对于出现在不同文件中的同名全局变量，如果有一个是初始化的，而其他是未初始化的，根据C标准ld的行为应该是选取已初始化的全局变量，舍弃其他的。但是在高版本的gcc中，其认为出现同名的全局变量是一种错误，不论其是否为强符号，从而导致编译失败。
* `QEMU`

在`ubuntu`上可以直接`sudo apt install qemu-system`即可。

* 环境变量

在编译前，在`bash`中输入`export AM_HOME=<abstract-machine目录>`创建需要的环境变量。

例如，如果`abstract-machine`位于`/usr/home/abstract-machine`，则`export AM_HOME=/usr/home/abstract-machine`。

### 编译与运行方式
首先，在`$PATH_TO_PROJECT/kernel`下`make`编译`abstract-machine`框架代码与操作系统核心代码并生成`qemu`所需的镜像文件。

再在`$PATH_TO_PROJECT/kernel`下输入`make run64`运行，此时`QEMU`设置4个CPU，从串口输出，运行64位程序。可以在bash上看到高频律的嵌套括号输出。


