# ObjectDragHandle 重构指南

## 重构概述

ObjectDragHandle 已经完成重构，现在作用目标从自身改为父节点，提高了扩展性。

## 重构前后对比

### 重构前的使用方式

```gdscript
# 旧的场景结构
ObjectDragHandle (Control + object_drag_handle.gd)
├── Sprite2D (实际显示内容)
├── AnimatedSprite2D
└── CollisionShape2D

# 行为：拖拽移动 ObjectDragHandle 自身
```

### 重构后的使用方式

```gdscript
# 新的场景结构
GameObject (Node2D)  # 实际要被拖拽的对象
├── Sprite2D         # 实际显示内容
├── AnimatedSprite2D # 更多显示内容
├── CollisionShape2D # 碰撞体
└── DragHandle (Control + ObjectDragHandle脚本)  # 拖拽控制器

# 行为：拖拽移动 GameObject（父节点）
```

## 核心变化

### 1. 拖拽移动目标
- **重构前**：`self.position += delta`
- **重构后**：`get_parent().position += delta`

### 2. 自适应大小计算
- **重构前**：基于自己的子节点（Sprite2D等）
- **重构后**：基于父节点的其他子节点（排除自己）

### 3. 透明检测
- **重构前**：检测自己的子节点透明度
- **重构后**：检测父节点其他子节点的透明度

## 使用指南

### 1. 基本设置

1. 创建一个父节点（通常是 Node2D）作为要拖拽的对象
2. 添加显示内容作为子节点（Sprite2D、AnimatedSprite2D等）
3. 添加一个 Control 节点作为最后的子节点
4. 将 ObjectDragHandle 脚本附加到 Control 节点

### 2. 示例代码

参考 `object_drag_example.tscn` 和 `object_drag_example.gd` 中的完整示例。

### 3. 重要注意事项

1. **ObjectDragHandle 必须是父节点的子节点**，而不是独立的节点
2. **父节点必须有其他子节点**，否则自适应大小功能无法正常工作
3. **坐标系转换**：DragHandle 会自动处理坐标转换，无需手动处理

## 兼容性

- 保持了原有的 API 接口
- ObjectDragManager 无需修改，自动适配新逻辑
- 所有信号和属性保持不变

## 测试方法

运行 `object_drag_example.tscn` 场景来测试重构后的功能：
1. 拖拽红色和绿色对象
2. 观察控制台输出的调试信息
3. 验证拖拽的是整个对象，而不是拖拽句柄本身

## 故障排除

### 问题：拖拽不工作
- 确认 ObjectDragHandle 有父节点
- 确认 ObjectDragManager 存在并正确初始化

### 问题：自适应大小不正确
- 确认父节点有其他子节点（排除 DragHandle 自身）
- 检查子节点是否可见

### 问题：透明检测不准确
- 确认启用了透明检测 (`enable_transparency_detection = true`)
- 检查父节点的其他子节点是否有纹理内容

---

# Object Drag Handle 使用指南（原文档保留作为参考）

## 概述

Object Drag Handle 是一个对象拖拽系统，允许在Godot场景中拖拽UI对象本身，而不是移动整个窗口。它与window drag handle互为补充，解决了不同的使用场景。

## 核心特性

- **对象级拖拽**：移动场景中的Control对象，而不是窗口
- **透明检测**：复用主窗口控制器的透明检测机制
- **重叠处理**：智能处理多个可拖拽对象的重叠区域
- **自适应大小**：根据子节点内容自动调整Control大小
- **冲突检测**：运行时检测与window drag handle的冲突

## 快速开始

### 1. 基础场景结构

```
MainScene (Node2D)
├── UniWindowController (窗口控制器)
├── ObjectDragManager (Control + object_drag_manager.gd)
│   ├── DraggableIcon (ObjectDragHandle + object_drag_handle.gd)
│   │   └── Sprite2D (实际显示的图标)
│   ├── DraggableButton (ObjectDragHandle)
│   │   └── Button (可拖拽的按钮)
│   └── DraggableCard (ObjectDragHandle)
│       ├── Control (卡片背景)
│       └── Label (卡片文字)
└── 其他游戏对象...
```

### 2. 添加到现有项目

#### 步骤1：添加ObjectDragManager

1. 将`object_drag_manager.tscn`拖拽到你的场景中
2. 确保ObjectDragManager是全屏的（anchors设置为0,0,1,1）
3. 确保UniWindowController存在于场景中

#### 步骤2：创建可拖拽对象

1. 将`object_drag_handle.tscn`实例化为ObjectDragManager的子节点
2. 在ObjectDragHandle下添加实际显示的内容（Sprite2D、Control等）
3. ObjectDragHandle会自动调整大小来适应子节点

### 3. 使用示例

```gdscript
# 主场景脚本
extends Control

func _ready():
    var drag_manager = $ObjectDragManager
    
    # 连接拖拽信号
    for handle in drag_manager.object_drag_handles:
        handle.drag_started.connect(_on_object_drag_started)
        handle.dragging.connect(_on_object_dragging)
        handle.drag_ended.connect(_on_object_drag_ended)

func _on_object_drag_started(handle: ObjectDragHandle):
    print("开始拖拽: ", handle.name)

func _on_object_dragging(handle: ObjectDragHandle, pos: Vector2):
    # 拖拽过程中的逻辑
    pass

func _on_object_drag_ended(handle: ObjectDragHandle):
    print("拖拽结束: ", handle.name, " 位置: ", pos)
```

## 配置选项

### ObjectDragManager

无特殊配置，自动管理所有ObjectDragHandle。

### ObjectDragHandle

#### Drag Settings
- **enable_transparency_detection**: 是否启用透明检测（默认true）
- **show_debug_info**: 显示调试信息（默认false）
- **opacity_threshold**: 透明度阈值，0.0-1.0（默认0.1）

#### Size Settings
- **auto_size_enabled**: 启用自动大小调整（默认true）
- **size_update_interval**: 大小更新间隔秒数（默认0.1）
- **manual_size**: 手动设置的大小，为零时使用自动大小

## API参考

### ObjectDragManager

#### 属性
- `object_drag_handles: Array[ObjectDragHandle]` - 所有已注册的拖拽对象

#### 方法
- `refresh_drag_handles()` - 手动刷新ObjectDragHandle列表
- `get_drag_handles_count() -> int` - 获取拖拽对象数量
- `get_current_dragging_handle() -> ObjectDragHandle` - 获取当前正在拖拽的对象
- `get_main_controller()` - 获取主窗口控制器引用

### ObjectDragHandle

#### 信号
- `drag_started()` - 开始拖拽时发出
- `dragging(position: Vector2)` - 拖拽过程中发出
- `drag_ended()` - 拖拽结束时发出
- `size_changed(new_size: Vector2)` - 大小变化时发出

#### 方法
- `start_drag_manually(mouse_pos: Vector2) -> bool` - 手动开始拖拽
- `end_drag_manually()` - 手动结束拖拽
- `is_dragging() -> bool` - 查询是否正在拖拽
- `set_auto_size_enabled(enabled: bool)` - 设置是否启用自动大小
- `set_manual_size(new_size: Vector2)` - 设置手动大小
- `refresh_size()` - 手动刷新大小
- `is_self_opaque_at_position(local_pos: Vector2) -> bool` - 检测指定位置是否不透明

## 重叠处理机制

当多个ObjectDragHandle有重叠区域时，系统按以下优先级处理：

1. **Z-index优先**：z_index值更高的对象优先
2. **场景树顺序**：后添加到场景树的对象优先
3. **透明检测**：只有在不透明区域才响应拖拽

```gdscript
# 设置拖拽优先级
$ObjectDragHandle1.z_index = 1  # 较低优先级
$ObjectDragHandle2.z_index = 2  # 较高优先级
```

## 自适应大小

ObjectDragHandle会自动根据子节点内容调整大小：

### 支持的子节点类型
- **Sprite2D**: 根据纹理大小和缩放计算
- **AnimatedSprite2D**: 根据当前帧纹理计算
- **Control**: 使用Control的rect大小
- **CollisionShape2D**: 根据形状类型计算边界
- **物理体**: 根据其下的CollisionShape2D计算

### 自适应行为
- 计算所有可见子节点的边界矩形
- 调整ObjectDragHandle的位置和大小来包含所有内容
- 自动调整子节点的相对位置
- 定期更新以处理动画变化

## 透明检测

Object Drag Handle实现了两级透明检测：

### 1. 全局透明检测
复用主窗口控制器的透明检测结果，与window drag handle保持一致。

### 2. 局部透明检测
检测ObjectDragHandle自己的子节点在指定位置是否透明：

```gdscript
# 检测逻辑
func is_self_opaque_at_position(local_pos: Vector2) -> bool:
    for child in get_children():
        if _is_child_opaque_at_position(child, local_pos):
            return true
    return false
```

## 与Window Drag Handle的区别

| 特性 | Window Drag Handle | Object Drag Handle |
|------|-------------------|-------------------|
| 移动目标 | 整个应用窗口 | 场景中的Control对象 |
| 坐标系统 | Native/屏幕坐标 | Godot场景坐标 |
| 事件处理 | 直接处理_gui_input | 管理器集中处理 |
| 重叠处理 | 单一对象 | 优先级机制 |
| 使用场景 | 窗口管理 | UI对象交互 |

## 注意事项

1. **冲突避免**：不要在同一场景中同时使用window drag handle和object drag handle
2. **性能考虑**：透明检测可能有性能开销，可以通过调整`opacity_threshold`优化
3. **层级管理**：确保ObjectDragManager在正确的层级，能够接收到鼠标事件
4. **自动大小**：如果子节点频繁变化，考虑调整`size_update_interval`或禁用自动大小

## 调试

### 启用调试信息
```gdscript
# 对所有ObjectDragHandle启用调试
for handle in drag_manager.object_drag_handles:
    handle.show_debug_info = true
```

### 调试快捷键（在示例场景中）
- **R键**：刷新拖拽对象列表
- **D键**：切换调试信息显示
- **S键**：显示当前状态信息

### 常见问题排查

1. **无法拖拽**：
   - 检查ObjectDragManager是否全屏
   - 确认ObjectDragHandle的mouse_filter设置为IGNORE
   - 验证透明检测是否正常工作

2. **大小不正确**：
   - 检查auto_size_enabled设置
   - 确认子节点的可见性
   - 手动调用refresh_size()

3. **优先级问题**：
   - 调整z_index值
   - 检查场景树中的节点顺序

## 示例文件

- `object_drag_example.tscn` - 完整的使用示例
- `object_drag_manager.tscn` - ObjectDragManager模板
- `object_drag_handle.tscn` - ObjectDragHandle模板