extends Control
class_name WindowDragHandle

## 窗口拖拽处理器
##
## 这个类提供了与Unity版本UniWindowMoveHandle完全一致的窗口拖拽移动功能。
## 通过检测鼠标拖拽操作来移动整个应用窗口。
## 实现逻辑严格遵循Unity版本，使用增量拖拽方式。
## 修复：使用主控制器的透明检测结果，避免冲突。

## 配置选项 - 这些会在Inspector中显示
@export_group("Drag Settings")
@export var disable_on_zoomed: bool = true ## 当窗口最大化时禁用拖拽
@export var show_debug_info: bool = false ## 显示调试信息

@export_group("Transparency Detection")
@export var enable_transparency_detection: bool = true ## 启用透明度检测（与Unity一致）

## 内部状态 - 基于坐标系转换的差值算法
var _is_dragging: bool = false
var _mouse_offset: Vector2  # 鼠标相对于窗口的偏移量（在Native坐标系中）
var _screen_height: int  # 主显示器高度，用于Y轴转换
var _window_controller  # 不指定类型，避免编译时依赖
var _is_hit_test_enabled: bool = true  # 记录拖拽前的hit test状态

## 信号
signal drag_started()
signal drag_ended()
signal dragging(position: Vector2)

func _ready():
	# 确保控件能接收鼠标事件
	mouse_filter = Control.MOUSE_FILTER_STOP
	
	# 在编辑器中不执行运行时逻辑
	if Engine.is_editor_hint():
		return
	
	_find_window_controller()

## 获取主控制器的透明检测结果
func _get_on_opaque_pixel_from_controller() -> bool:
	# 使用主控制器的透明检测结果，避免重复检测
	# 这些是getter方法，不是native方法，所以只需要直接访问属性
	if _window_controller:
		return _window_controller._internal_on_object
	
	# 默认返回true（在不透明区域）
	return true

func _find_window_controller():
	# 首先在父节点中查找
	var parent = get_parent()
	while parent:
		if parent.has_method("get_native_controller"):  # 这个检查保留，因为这是查找正确类型的节点
			_window_controller = parent
			return
		parent = parent.get_parent()
	
	# 如果没找到，在整个场景树中查找
	var nodes = get_tree().get_nodes_in_group("uni_window_controller")
	if nodes.size() > 0 and nodes[0].has_method("get_native_controller"):
		_window_controller = nodes[0]
		return
	
	# 最后尝试查找任何有get_native_controller方法的节点
	_window_controller = _find_wrapper_node(get_tree().current_scene)

func _find_wrapper_node(node: Node):  # 不指定返回类型
	if node.has_method("get_native_controller"):
		return node
	
	for child in node.get_children():
		var result = _find_wrapper_node(child)
		if result:
			return result
	
	return null

## 检查是否可以拖拽 - 与Unity版本的IsEnabled属性一致
func _can_drag() -> bool:
	if not _window_controller:
		return false
	
	# 修复Bug3：完全复制Unity版本的IsZoomed逻辑
	# Unity版本：IsZoomed = shouldFitMonitor || isZoomed
	if disable_on_zoomed:
		var should_fit = false
		var is_zoomed = false
		
		# 检查shouldFitMonitor（Unity版本的重要逻辑）
		should_fit = _window_controller.get_should_fit_monitor()
			
		# 检查isZoomed状态
		is_zoomed = _window_controller.get_zoomed()
		
		# Unity版本的关键逻辑：should_fit OR is_zoomed 都会禁用拖拽
		if should_fit or is_zoomed:
			return false
	
	return true

## 处理GUI输入事件
func _gui_input(event: InputEvent):
	if Engine.is_editor_hint():
		return
	
	# 使用主控制器的透明度检测结果：只有在不透明区域才处理拖拽
	if enable_transparency_detection and event is InputEventMouseButton and event.pressed:
		var on_opaque_pixel = _get_on_opaque_pixel_from_controller()
		if not on_opaque_pixel:
			return
		
	if event is InputEventMouseButton:
		_handle_mouse_button(event)
	elif event is InputEventMouseMotion:
		_handle_mouse_motion(event)

func _handle_mouse_button(event: InputEventMouseButton):
	if event.button_index == MOUSE_BUTTON_LEFT:
		if event.pressed:
			_start_drag(event.global_position)
		else:
			_end_drag()

func _handle_mouse_motion(event: InputEventMouseMotion):
	if _is_dragging:
		_update_drag(event.global_position)

## 将Godot窗口内坐标转换为系统屏幕坐标
func _convert_to_screen_coordinates(godot_position: Vector2) -> Vector2:
	# 获取当前窗口在屏幕上的位置
	var window_pos_i = DisplayServer.window_get_position()
	var window_pos = Vector2(window_pos_i.x, window_pos_i.y)  # 转换为Vector2
	
	# Godot坐标 + 窗口在屏幕上的偏移 = 系统屏幕坐标
	var screen_pos = godot_position + window_pos
	
	return screen_pos

## 坐标系转换函数
## 将系统屏幕坐标(左上角原点)转换为Native坐标系(左下角原点)
func _screen_to_native_coords(screen_pos: Vector2) -> Vector2:
	return Vector2(screen_pos.x, _screen_height - screen_pos.y)

## 将Native坐标系(左下角原点)转换为系统屏幕坐标(左上角原点)  
func _native_to_screen_coords(native_pos: Vector2) -> Vector2:
	return Vector2(native_pos.x, _screen_height - native_pos.y)

## 开始拖拽 - 基于坐标系转换的差值算法 (修复：不禁用主控制器的hit test)
func _start_drag(mouse_godot_position: Vector2):
	if not _can_drag():
		return
	
	# 获取屏幕高度用于坐标转换
	var screen_rect = DisplayServer.screen_get_usable_rect()
	_screen_height = screen_rect.size.y
	
	# 计算鼠标的系统屏幕坐标(左上角原点)
	var godot_window_pos = DisplayServer.window_get_position()
	var mouse_screen_coords = mouse_godot_position + Vector2(godot_window_pos.x, godot_window_pos.y)
	
	# 转换为Native坐标系(左下角原点)
	var mouse_native_coords = _screen_to_native_coords(mouse_screen_coords)
	
	# 获取Native窗口位置(已经是左下角原点坐标系)
	var native_window_pos = _get_native_window_position()
	
	# 计算鼠标相对于窗口的偏移量(在Native坐标系中)
	_mouse_offset = mouse_native_coords - native_window_pos
	
	# 修复：拖拽时不禁用主控制器的hit test，保持透明检测正常工作
	# 只在拖拽期间临时关闭点击透传，防止拖拽时鼠标事件透传
	if not _is_dragging:
		_window_controller.set_click_through(false)
	
	_is_dragging = true
	drag_started.emit()

## 获取Native窗口位置
func _get_native_window_position() -> Vector2:
	if _window_controller:
		var native_controller = _window_controller.get_native_controller()
		if native_controller:
			return native_controller.position
	return Vector2.ZERO

## 更新拖拽 - 基于坐标系转换的差值算法
func _update_drag(mouse_godot_position: Vector2):
	if not _window_controller or not _is_dragging:
		return
	
	# 检查各种拖拽条件
	if not _can_drag():
		_end_drag()
		return
	if _is_modifier_pressed():
		return
	if not _is_left_mouse_pressed():
		_end_drag()
		return
	if _is_fullscreen():
		_end_drag()
		return
	
	# 计算当前鼠标的系统屏幕坐标(左上角原点)
	var godot_window_pos = DisplayServer.window_get_position()
	var mouse_screen_coords = mouse_godot_position + Vector2(godot_window_pos.x, godot_window_pos.y)
	
	# 转换为Native坐标系(左下角原点)
	var mouse_native_coords = _screen_to_native_coords(mouse_screen_coords)
	
	# 差值算法：新窗口位置 = 当前鼠标位置 - 偏移量
	var new_native_window_position = mouse_native_coords - _mouse_offset
	
	# 设置窗口位置
	_set_native_window_position(new_native_window_position)
	
	# 发出信号
	dragging.emit(new_native_window_position)

## 设置Native窗口位置
func _set_native_window_position(position: Vector2):
	if _window_controller:
		var native_controller = _window_controller.get_native_controller()
		if native_controller:
			native_controller.position = position

## 结束拖拽 - 修复：不恢复hit test设置，让主控制器继续管理
func _end_drag():
	if not _is_dragging:
		return
	
	# 修复：不恢复hit test设置，让主控制器自己管理透明检测和点击透传
	# 拖拽结束后，主控制器会根据当前鼠标位置自动设置点击透传状态
	
	_is_dragging = false
	
	# 发出信号
	drag_ended.emit()

## 检查是否有修饰键按下 - 与Unity版本一致
func _is_modifier_pressed() -> bool:
	# 使用Native库的修饰键检测（与Unity版本一致）
	if _window_controller:
		var modifier_keys = _window_controller.get_modifier_keys()
		# 检查是否有修饰键按下（参考Unity版本的ModifierKey枚举）
		# None = 0, Alt = 1, Control = 2, Shift = 4, Command = 8
		if modifier_keys != 0:
			return true
	
	return false

## 检查鼠标左键是否按下 - Unity版本的安全检查
func _is_left_mouse_pressed() -> bool:
	# 使用Native库的鼠标按键检测（与Unity版本一致）
	if _window_controller:
		var mouse_buttons = _window_controller.get_mouse_buttons()
		# 检查左键是否按下（Unity版本逻辑）
		# None = 0, Left = 1, Right = 2, Middle = 4
		return (mouse_buttons & 1) != 0  # Left = 1
	
	# Fallback到Godot的检测
	return Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT)

## 检查是否全屏 - Unity版本逻辑
func _is_fullscreen() -> bool:
	# Unity版本：if (Screen.fullScreen)
	return DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN

## 获取窗口位置 - 使用系统屏幕坐标
func _get_window_position() -> Vector2:
	# 使用Godot的窗口位置（这是系统屏幕坐标）
	var godot_window_pos = DisplayServer.window_get_position()
	var screen_pos = Vector2(godot_window_pos.x, godot_window_pos.y)
	
	# 也获取native controller的位置用于对比
	if _window_controller:
		var native_controller = _window_controller.get_native_controller()
		if native_controller:
			var native_pos = native_controller.position
	
	return screen_pos

## 设置窗口位置 - 暂时使用Godot的窗口位置进行测试
func _set_window_position(position: Vector2):
	# 先测试用Godot自己的窗口位置设置
	var pos_i = Vector2i(int(position.x), int(position.y))
	DisplayServer.window_set_position(pos_i)
	
	# 也可以同时设置native controller位置用于对比
	if _window_controller:
		var native_controller = _window_controller.get_native_controller()
		if native_controller:
			# 使用与wrapper一致的方式：直接赋值position属性
			native_controller.position = position

## 处理窗口失去焦点 - Unity版本中很重要
func _notification(what: int):
	if Engine.is_editor_hint():
		return
		
	if what == NOTIFICATION_WM_WINDOW_FOCUS_OUT:
		if _is_dragging:
			_end_drag()

## 属性访问器
func is_dragging() -> bool:
	return _is_dragging

func set_window_controller(controller):  # 不指定参数类型
	_window_controller = controller

