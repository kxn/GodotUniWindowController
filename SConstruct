#!/usr/bin/env python3

import os
import sys

EnsureSConsVersion(4, 0)

# 简单地使用godot-cpp的构建系统
env = SConscript("godot-cpp/SConstruct")

# 添加我们的源文件路径
env.Append(CPPPATH=["src/"])

# 获取平台信息
platform = env["platform"]
arch = env["arch"]
target = env["target"]

print(f"Building for platform: {platform}, arch: {arch}, target: {target}")

# 平台特定配置
if platform == "windows":
    target_extension = ".dll"
    library_name = "libuniwincgd"
    env.Append(CPPDEFINES=["WIN32_LEAN_AND_MEAN"])
elif platform == "macos":
    target_extension = ".dylib" 
    library_name = "libuniwincgd"
elif platform == "linux":
    target_extension = ".so"
    library_name = "libuniwincgd"
else:
    print(f"Unsupported platform: {platform}")
    Exit(1)

# 我们的源文件
sources = Glob("src/*.cpp")

# 输出目录和文件
output_dir = f"addons/uniwinc/bin/{platform}"
os.makedirs(output_dir, exist_ok=True)

# 构建共享库
library = env.SharedLibrary(f"{output_dir}/{library_name}", sources)

Default(library)