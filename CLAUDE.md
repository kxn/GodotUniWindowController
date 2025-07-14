# 项目说明

## 当前需求

## 注意事项

- 修改 bug 的时候请注意，找到一个原因以后，请进一步找出所有可能的原因，不要迅速下结论
- 如果确认修正了一个问题，那么请再仔细查找一下项目中是否还有类似的问题，一并修复
- GDScript 没有 try...except 语法！ 不要老是把它当 python
- 在实现阶段不要防御性编程，对于系统性错误应该让其直接暴露，这会掩盖大量的问题，导致调试异常困难。只有和用户输入相关的地方才需要防御性编程，其他的异常情况请直接暴露，包括但不限于调用 gdscript 方法之前检查方法是否存在，在修改需求的时候将原逻辑作为 fallback 逻辑保留，等等。请绝对不要进行这样的防御性编程
- 请注意阅读一定要仔细，不要有漏掉或者搞错的地方
- 所有的沟通请使用中文
- 对于自己不是很懂的，请积极上网搜索解决方法，不要自己瞎想

## 编译说明

### GDExtension C++ 部分编译方法

项目使用 SCons 构建系统，需要使用项目中的虚拟环境：

#### 基本编译命令

```bash
# 激活虚拟环境
source venv/bin/activate

# Linux 版本编译
scons platform=linux target=template_debug -j4

# Windows 版本交叉编译（使用 MinGW）
scons platform=windows use_mingw=yes target=template_debug arch=x86_64 -j4

# macOS 版本编译
scons platform=macos target=template_debug -j4

# 清理编译产物
scons -c
```

#### 重要参数说明

1. **platform**: 目标平台 (linux|windows|macos)
2. **use_mingw=yes**: Windows 平台必须使用，启用 MinGW 交叉编译器
3. **target**: 编译目标 (template_debug|template_release|editor)
4. **arch**: 架构 (x86_64|arm64)
5. **-j**: 并行编译线程数

#### 编译产物位置

- Linux: `addons/uniwinc/bin/linux/libuniwincgd.so`
- Windows: `addons/uniwinc/bin/windows/libuniwincgd.dll`
- macOS: `addons/uniwinc/bin/macos/libuniwincgd.dylib`

#### 依赖检查

确保系统已安装必要的交叉编译工具：

- Linux 本地编译: gcc/g++
- Windows 交叉编译: mingw-w64 (`x86_64-w64-mingw32-g++`)
- macOS 交叉编译: Xcode Command Line Tools (在 macOS 上)

#### 常见问题

1. **编译超时**: godot-cpp 首次编译时间较长，需要足够的时间完成
2. **MinGW 缺失**: Windows 交叉编译需要安装 mingw-w64 包
3. **venv 环境**: 必须激活项目虚拟环境才能找到 scons
