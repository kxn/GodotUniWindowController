extends Control
class_name ObjectDragManager

## 全屏事件管理器，负责分发鼠标事件给正确的ObjectDragHandle
##
## 这个类解决了Godot中Control兄弟节点无法直接传递鼠标事件的限制。
## 它接收全屏的鼠标事件，然后根据优先级和透明检测决定拖拽哪个ObjectDragHandle。
## 实现逻辑基于window_drag_handle.gd的透明检测机制复用。

var object_drag_handles: Array[ObjectDragHandle] = []
var _main_controller  # 主窗口控制器引用
var _current_dragging_handle: ObjectDragHandle = null

## 初始化
func _ready():
	# 设置为全屏接收所有鼠标事件
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	mouse_filter = Control.MOUSE_FILTER_STOP
	
	# 在编辑器中不执行运行时逻辑
	if Engine.is_editor_hint():
		return
	
	# 查找主控制器（复用window_drag_handle.gd的逻辑）
	_find_main_window_controller()
	
	# 延迟收集所有ObjectDragHandle，确保场景完全加载
	call_deferred("_collect_drag_handles")

## 查找主窗口控制器 - 复用window_drag_handle.gd的实现
func _find_main_window_controller():
	# 首先在父节点中查找
	var parent = get_parent()
	while parent:
		if parent.has_method("get_native_controller"):
			_main_controller = parent
			return
		parent = parent.get_parent()
	
	# 如果没找到，在整个场景树中查找组成员
	var nodes = get_tree().get_nodes_in_group("uni_window_controller")
	if nodes.size() > 0 and nodes[0].has_method("get_native_controller"):
		_main_controller = nodes[0]
		return
	
	# 最后尝试查找任何有get_native_controller方法的节点
	_main_controller = _find_wrapper_node(get_tree().current_scene)

func _find_wrapper_node(node: Node):
	if node.has_method("get_native_controller"):
		return node
	
	for child in node.get_children():
		var result = _find_wrapper_node(child)
		if result:
			return result
	
	return null

## 收集所有ObjectDragHandle并设置它们
func _collect_drag_handles():
	object_drag_handles.clear()
	_recursive_find_drag_handles(self)
	
	# 按优先级排序：Z-index高的优先，场景树中后添加的优先
	object_drag_handles.sort_custom(_compare_drag_handles)
	
	print("ObjectDragManager: 发现 ", object_drag_handles.size(), " 个ObjectDragHandle")

func _recursive_find_drag_handles(node: Node):
	if node is ObjectDragHandle:
		object_drag_handles.append(node)
		# 禁用ObjectDragHandle的事件处理
		node.mouse_filter = Control.MOUSE_FILTER_IGNORE
		# 设置管理器引用
		node.set_manager(self)
		# 移除频繁的调试信息
	
	for child in node.get_children():
		_recursive_find_drag_handles(child)

func _compare_drag_handles(a: ObjectDragHandle, b: ObjectDragHandle) -> bool:
	# Z-index高的优先
	if a.z_index != b.z_index:
		return a.z_index > b.z_index
	# 场景树中后添加的优先
	return a.get_index() > b.get_index()

## 核心事件处理逻辑
func _gui_input(event: InputEvent):
	if Engine.is_editor_hint():
		return
		
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
		if event.pressed:
			_handle_mouse_press(event.global_position)
		else:
			_handle_mouse_release()
	elif event is InputEventMouseMotion:
		_handle_mouse_motion(event.global_position)

func _handle_mouse_press(global_pos: Vector2):
	# 依次检查每个ObjectDragHandle（按优先级）
	for i in range(object_drag_handles.size()):
		var handle = object_drag_handles[i]
		if _should_handle_drag(handle, global_pos):
			_current_dragging_handle = handle
			handle.start_drag_from_manager(global_pos)
			accept_event()
			return

func _handle_mouse_motion(global_pos: Vector2):
	if _current_dragging_handle and _current_dragging_handle.is_dragging():
		_current_dragging_handle.update_drag_from_manager(global_pos)

func _handle_mouse_release():
	if _current_dragging_handle and _current_dragging_handle.is_dragging():
		_current_dragging_handle.end_drag_from_manager()
		_current_dragging_handle = null

## 判断是否应该由指定的handle处理拖拽
func _should_handle_drag(handle: ObjectDragHandle, global_pos: Vector2) -> bool:
	# 将全局坐标转换为handle的本地坐标
	# Control没有to_local方法，需要手动计算
	var handle_global_pos = handle.global_position
	var local_pos = global_pos - handle_global_pos
	
	# 检查是否在handle的自适应区域内
	var rect = Rect2(Vector2.ZERO, handle.size)
	var in_rect = rect.has_point(local_pos)
	
	if not in_rect:
		return false
	
	# 检查handle在这个位置是否有不透明内容
	var opaque = handle.is_self_opaque_at_position(local_pos)
	return opaque

## 获取主控制器的透明检测结果
func _get_on_opaque_pixel_from_controller() -> bool:
	if _main_controller:
		return _main_controller._internal_on_object
	return true

## 公共API方法
func get_main_controller():
	return _main_controller

func refresh_drag_handles():
	"""手动刷新ObjectDragHandle列表"""
	_collect_drag_handles()

func get_drag_handles_count() -> int:
	return object_drag_handles.size()

func get_current_dragging_handle() -> ObjectDragHandle:
	return _current_dragging_handle

## 调试信息
func _input(event: InputEvent):
	# 处理全局输入，确保能接收到松开事件
	if event is InputEventMouseButton and not event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if _current_dragging_handle and _current_dragging_handle.is_dragging():
			_handle_mouse_release()
