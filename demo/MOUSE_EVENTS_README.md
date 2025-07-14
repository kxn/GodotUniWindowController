# ObjectDragHandle 鼠标事件功能测试

## 功能概述

为 ObjectDragHandle 系统添加了完整的鼠标事件信号支持，包括：

### 新增信号
- `handle_mouse_event(event: InputEvent)` - 统一的鼠标事件信号，包含所有鼠标按钮和移动事件
- `handle_mouse_entered()` - 鼠标进入对象区域
- `handle_mouse_exited()` - 鼠标离开对象区域

### 新增配置选项
- `enable_mouse_events: bool` - 启用/禁用鼠标按钮和移动事件
- `enable_hover_events: bool` - 启用/禁用鼠标悬停事件

### 新增API方法
- `is_mouse_over() -> bool` - 查询鼠标是否悬停在对象上
- `get_last_mouse_position() -> Vector2` - 获取最后记录的鼠标位置
- `get_current_hover_handle() -> ObjectDragHandle` - 获取当前悬停的对象（Manager方法）

## 测试方式

### 方式1：使用现有Demo场景
```bash
# 运行修改后的demo场景
# 文件: demo/demo.tscn
```

在 Godot 编辑器中打开 `demo/demo.tscn` 并运行。控制台会显示详细的事件日志。

**测试步骤：**
1. 移动鼠标到女孩图像上 → 应该看到 `handle_mouse_entered` 事件和光标变化
2. 在图像上移动鼠标 → 应该看到 `handle_mouse_event` (MouseMotion) 事件
3. 点击左键 → 应该看到 `handle_mouse_event` (MouseButton) 事件，并开始拖拽
4. 点击右键 → 应该看到右键菜单事件
5. 移开鼠标 → 应该看到 `handle_mouse_exited` 事件和光标恢复

### 方式2：使用完整测试场景
```bash
# 运行完整测试场景
# 文件: demo/mouse_events_test.tscn
```

这个场景提供了：
- 多个测试对象（图标和按钮）
- 实时事件日志显示
- 状态监控面板
- 调试控制按钮

### 方式3：快速验证脚本
```bash
# 运行快速验证
# 文件: demo/quick_test.gd
```

将此脚本附加到任何包含 ObjectDragHandle 的场景，它会自动检查所有功能是否正确实现。

## 实现细节

### 事件触发条件
- **精确的透明检测**：只有鼠标在对象的不透明像素区域才会触发事件
- **与拖拽一致**：鼠标事件的触发区域与拖拽触发区域完全相同
- **优先级处理**：多个对象重叠时，Z-index 较高的对象优先接收事件

### 性能优化
- **复用现有逻辑**：透明检测和坐标转换复用现有代码
- **事件分发集中化**：ObjectDragManager 统一处理所有事件分发
- **避免重复计算**：同一帧内的坐标转换和透明检测结果会被复用

### 兼容性
- **向后兼容**：现有的拖拽功能完全不受影响
- **可选功能**：鼠标事件可以通过配置选项独立启用/禁用
- **无破坏性改动**：所有现有API和行为保持不变

## 使用示例

### 基础鼠标事件处理
```gdscript
extends Node

func _ready():
    var drag_handle = $ObjectDragHandle
    
    # 连接鼠标事件 - 使用统一的事件信号
    drag_handle.handle_mouse_entered.connect(_on_mouse_entered)
    drag_handle.handle_mouse_exited.connect(_on_mouse_exited)
    drag_handle.handle_mouse_event.connect(_on_mouse_event)

func _on_mouse_entered():
    # 鼠标进入时改变光标
    Input.set_default_cursor_shape(Input.CURSOR_MOVE)

func _on_mouse_exited():
    # 鼠标离开时恢复光标
    Input.set_default_cursor_shape(Input.CURSOR_ARROW)

func _on_mouse_event(event: InputEvent):
    # 统一处理所有鼠标事件，类似 _unhandled_input
    if event is InputEventMouseButton:
        var button_event = event as InputEventMouseButton
        if button_event.button_index == MOUSE_BUTTON_RIGHT and button_event.pressed:
            # 显示右键菜单
            show_context_menu(button_event.global_position)
    elif event is InputEventMouseMotion:
        # 处理鼠标移动
        update_hover_effect(event.position)
```

### 悬停效果
```gdscript
extends Node

@onready var sprite = $Sprite2D
var original_modulate: Color

func _ready():
    original_modulate = sprite.modulate
    var drag_handle = $ObjectDragHandle
    
    drag_handle.handle_mouse_entered.connect(_on_hover_start)
    drag_handle.handle_mouse_exited.connect(_on_hover_end)

func _on_hover_start():
    # 悬停时高亮显示
    sprite.modulate = Color.WHITE * 1.2

func _on_hover_end():
    # 离开时恢复原色
    sprite.modulate = original_modulate
```

### 复用现有输入处理代码
```gdscript
extends Node

func _ready():
    var drag_handle = $ObjectDragHandle
    # 直接将鼠标事件转发给现有的输入处理方法
    drag_handle.handle_mouse_event.connect(_handle_input)

func _handle_input(event: InputEvent):
    # 可以直接复用现有的 _input 或 _unhandled_input 代码
    if event is InputEventMouseButton:
        var button_event = event as InputEventMouseButton
        if button_event.pressed:
            match button_event.button_index:
                MOUSE_BUTTON_LEFT:
                    _handle_left_click(button_event)
                MOUSE_BUTTON_RIGHT:
                    _handle_right_click(button_event)
                MOUSE_BUTTON_MIDDLE:
                    _handle_middle_click(button_event)
    elif event is InputEventMouseMotion:
        _handle_mouse_motion(event)

# 现有的处理方法可以直接复用
func _handle_left_click(event: InputEventMouseButton):
    print("左键点击")

func _handle_right_click(event: InputEventMouseButton):
    print("右键点击")

func _handle_mouse_motion(event: InputEventMouseMotion):
    print("鼠标移动")

func _on_hover_start():
    # 悬停时高亮显示
    sprite.modulate = Color.WHITE * 1.2

func _on_hover_end():
    # 离开时恢复原色
    sprite.modulate = original_modulate
```

### 状态查询
```gdscript
extends Node

func _process(_delta):
    var drag_handle = $ObjectDragHandle
    
    # 查询鼠标状态
    if drag_handle.is_mouse_over():
        var mouse_pos = drag_handle.get_last_mouse_position()
        print("鼠标在对象上，位置: ", mouse_pos)
    
    # 查询管理器状态
    var manager = $ObjectDragManager
    var hover_handle = manager.get_current_hover_handle()
    if hover_handle:
        print("当前悬停对象: ", hover_handle.name)
```

## 故障排除

### 事件不触发
1. 检查 `enable_mouse_events` 和 `enable_hover_events` 是否启用
2. 确认 ObjectDragManager 正确设置并包含 ObjectDragHandle
3. 验证透明检测设置（`enable_transparency_detection`）

### 事件重复或丢失
1. 检查是否有多个 ObjectDragManager 实例
2. 确认 mouse_filter 设置正确（Handle应该是IGNORE，Manager应该是STOP）

### 性能问题
1. 减少 `mouse_motion` 事件的处理频率
2. 在不需要时禁用透明检测
3. 检查是否有过多的调试输出

## 开发说明

此功能的实现严格遵循了现有的架构模式：
- ObjectDragHandle 负责具体的事件处理和信号发出
- ObjectDragManager 负责事件捕获、优先级判断和分发
- 完全复用现有的透明检测和坐标转换逻辑
- 保持与现有拖拽功能的完美协调

这确保了功能的稳定性和一致性，同时为用户提供了强大的鼠标交互能力。

## 🔄 设计优势 - 统一事件信号

### 为什么使用 `handle_mouse_event(event: InputEvent)`？

1. **代码复用性**：可以直接复用现有的 `_input` 或 `_unhandled_input` 处理代码
2. **简洁性**：只需连接一个信号而不是多个分散的信号
3. **扩展性**：未来添加新的输入事件类型（如触摸、手柄）时无需修改接口
4. **熟悉性**：与 Godot 原生的 `_unhandled_input` 模式一致，开发者更容易理解

### 对比其他设计方案

#### ❌ 分散信号方案
```gdscript
# 需要连接多个信号
handle.mouse_button_down.connect(_on_button_down)
handle.mouse_button_up.connect(_on_button_up) 
handle.mouse_motion.connect(_on_motion)
# 代码分散在多个处理方法中
```

#### ✅ 统一信号方案
```gdscript
# 只需连接一个信号
handle.handle_mouse_event.connect(_on_mouse_event)
# 统一处理，代码集中，便于复用
```

### 实际收益

1. **减少样板代码**：不需要为每种事件类型写单独的处理方法
2. **便于迁移**：现有的输入处理逻辑可以直接移植过来
3. **统一的事件处理模式**：与 Godot 的设计哲学保持一致
4. **更好的可维护性**：事件处理逻辑集中在一个地方

这种设计既保持了功能的完整性，又提供了更好的开发体验。