### 前置操作

1. 先安装好WSL2 (在powershell中操作) 

   ```shell
   // 安装WSL2
   ​	wsl --install
   // 安装完成后，重启电脑
   // 检查 WSL 的版本
   ​	wsl -l -v
   // 若VERSION是2，说明是WSL2
   ```

   

2. 在WSL2中安装nvm/nodejs/npm：

   ```shell
   // 打开wsl2，第一次打开时会让你设置Linux用户名和密码
   ​	wsl
   // 安装基础工具
   ​	sudo apt update && sudo apt upgrade -y
   ​	sudo apt install -y curl git build-essential
   // 安装 nvm
   ​	curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/master/install.sh | bash
   // 退出，让 shell 重新加载配置
   ​	exit
   // 再打开wsl2
   ​	wsl
   // 安装 Node 22 和 npm
   ​	nvm install 22
   ​	nvm use 22
   // 检查 node 和 npm 是否安装成功
   ​	node -v
   ​	npm -v
   ```

   

### Codex安装相关指令

```shell
// 安装/更新：
​	npm install -g @openai/codex@latest
// 查看安装目录：
​	which codex
// 启动（在项目根目录下）：
​	codex
// 检查最新版本：
​	npm view @openai/codex version
// 检查当前已安装版本：
​	codex --version
```



### Opencode安装相关指令

```shell
// 安装/更新：
​	npm install -g opencode-ai@latest
// 查看安装目录：
​	which opencode
// 启动（在项目根目录下）：
​	opencode
// 检查最新版本：
​	npm view opencode-ai version
// 检查当前已安装版本：
​	opencode --version
```



### Claude code安装相关指令

```shell
// 安装/更新：
​	npm install -g @anthropic-ai/claude-code@latest
// 查看安装目录：
​	which claude
// 启动（在项目根目录下）：
​	claude
// 检查最新版本：
​	npm view @anthropic-ai/claude-code version
// 检查当前已安装版本：
​	claude --version
```



