# 说明
该项目同样基于`abstract-machine`。
## 编译运行
首先设置环境变量`export NEMU_HOME=<nemu的目录>`，例如`export NEMU_HOME=\usr\home\Emulator\nemu`

依赖`readline`库，可以在`ubuntu`下以下指令按照
`sudo apt install libreadline-dev` 安装

在`nemu`目录下`make run`即可启动该模拟器。默认情况下没有装载镜像，仅有一个8个指令的`dummy`内容。
