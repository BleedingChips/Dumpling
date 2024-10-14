# Dumpling

> 好吃不过饺子！

图形接口的封装，开发中。目前只完成了windowx下的窗口封装和dx12的部分接口封装。支持ImGui。

## 功能

### ImGui

## 安装

从 Github 上将以下库Clone到本地，并使其保持在同一个路径下：

	```
	https://github.com/BleedingChips/Dumpling.git
	https://github.com/BleedingChips/Potato.git
	```

在包含该项目的 xmake.lua 上，添加如下代码即可：

	```lua
	includes("../Potato/")
	includes("../Dumpling/")

	target(xxx)
		...
		add_deps("Dumpling")
	target_end()
	```

运行 `xmake_install.ps1` 安装 `xmake`，运行`xmake_generate_vs_project.ps1`将在`vsxmake2022`下产生vs的项目文件。

