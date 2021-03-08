# 说明
## 编译运行
在当前目录下输入`make`编译该动态链接库，在`./tests`目录下输入`make test`进行测试。
项目兼容32位与64位环境。

## API说明
一共提供了三个函数，签名如下所示。

```c
struct co* co_start(const char *name, void (*func)(void *), void *arg);
void co_yield();
void co_wait(struct co *co);
```

`co_start` 创建一个名称为`name`，以`arg`为实参执行函数`func`的协程，返回一个存有该协程信息的结构体。`co_start`仅为协程分配资源，但并没有执行`func`。`func`内部可以调用`co_yield`。

`co_wait` 保证`co`对应的函数在`co_wait`函数返回前执行完毕。此外，整个程序中的第一次`co_wait`真正启动协程，`co`对应的协程一定被启动，如果`co`对应的函数调用`co_yield`，则其他协程也会启动。**注意**，由于有可能存在没有被启动的协程，因为调度机制是随机而不是round robin。

`co_yield`发生一次让渡，使得当前协程暂停运行，切换到其他协程。

注意，在`main`中，每调用一次`co_start`创建一个协程必须对应一个`co_wait`回收它的资源。