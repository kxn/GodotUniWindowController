extends Control
class_name ObjectDragHandle

## 对象拖拽处理器 - 只负责拖拽逻辑和自适应大小
##
## 这个类实现了与window_drag_handle不同的对象拖拽机制。
## 它移动的是父节点，而不是自己本身。
## 透明检测和大小计算基于父节点的其他子节点（排除自己）。
## 事件处理由ObjectDragManager集中管理。

# 配置选项
@export_group("Drag Settings")
@export var enable_transparency_detection: bool = true
@export var show_debug_info: bool = false
@export_range(0.0, 1.0, 0.01) var opacity_threshold: float = 0.1

@export_group("Mouse Events")
@export var enable_mouse_events: bool = true
@export var enable_hover_events: bool = true

@export_group("Size Settings")
@export var auto_size_enabled: bool = true
@export var size_update_interval: float = 0.1  # 自动更新大小的间隔（秒）
@export var manual_size: Vector2 = Vector2.ZERO  # 手动设置的大小，为零时使用自动大小

# 内部状态
var _is_dragging: bool = false
var _drag_start_position: Vector2
var _drag_start_mouse_pos: Vector2
var _manager: ObjectDragManager  # 管理器引用
var _size_update_timer: Timer

# 鼠标事件状态
var _mouse_is_over: bool = false
var _last_mouse_position: Vector2 = Vector2.ZERO

# 信号定义
signal drag_started()
signal dragging(position: Vector2)
signal drag_ended()
signal size_changed(new_size: Vector2)

# 鼠标事件信号
signal handle_mouse_event(event: InputEvent)
signal handle_mouse_entered()
signal handle_mouse_exited()

func _ready():
	# 确保不处理鼠标事件（由管理器处理）
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	
	if Engine.is_editor_hint():
		return
	
	# 设置初始大小
	if auto_size_enabled:
		_update_size_to_fit_children()
	elif manual_size != Vector2.ZERO:
		size = manual_size
	
	# 监听子节点变化
	child_entered_tree.connect(_on_child_changed)
	child_exiting_tree.connect(_on_child_changed)
	
	# 启动定期更新（处理动画等动态变化）
	if auto_size_enabled:
		_start_size_update_timer()
	
	
## 管理器调用的拖拽开始方法
func start_drag_from_manager(global_pos: Vector2):
	if not _can_start_drag():
		return
		
	_is_dragging = true
	var parent = get_parent()
	if parent:
		_drag_start_position = parent.position
	else:
		_drag_start_position = Vector2.ZERO
		push_warning("ObjectDragHandle: 没有父节点，使用默认位置")
	_drag_start_mouse_pos = global_pos
	
	drag_started.emit()
	
	if show_debug_info:
		var debug_parent = get_parent()
		var parent_pos = debug_parent.position if debug_parent else Vector2.ZERO
		print("ObjectDragHandle: 开始拖拽 ", name, " 父节点位置 ", parent_pos)

## 管理器调用的拖拽更新方法
func update_drag_from_manager(global_pos: Vector2):
	if not _is_dragging:
		return
		
	# 简单的位移计算：当前鼠标位置 - 开始位置 = 位移量
	var mouse_delta = global_pos - _drag_start_mouse_pos
	var new_position = _drag_start_position + mouse_delta
	
	# 更新父节点位置
	var parent = get_parent()
	if parent:
		parent.position = new_position
		dragging.emit(parent.position)
	else:
		push_warning("ObjectDragHandle: 没有父节点，无法拖拽")
	
	if show_debug_info:
		var debug_parent = get_parent()
		var parent_pos = debug_parent.position if debug_parent else Vector2.ZERO
		print("ObjectDragHandle: 拖拽中 ", name, " 父节点到位置 ", parent_pos)

## 管理器调用的拖拽结束方法
func end_drag_from_manager():
	if not _is_dragging:
		return
		
	_is_dragging = false
	drag_ended.emit()
	
	if show_debug_info:
		var debug_parent = get_parent()
		var parent_pos = debug_parent.position if debug_parent else Vector2.ZERO
		print("ObjectDragHandle: 结束拖拽 ", name, " 父节点最终位置 ", parent_pos)

## 鼠标事件处理方法（由管理器调用）
func handle_mouse_event_from_manager(event: InputEvent, local_pos: Vector2):
	if not enable_mouse_events:
		return
		
	_last_mouse_position = local_pos
	
	# 统一发出鼠标事件信号，让使用者自己处理不同事件类型
	handle_mouse_event.emit(event)
	
	if show_debug_info:
		var event_type = ""
		if event is InputEventMouseButton:
			var button_event = event as InputEventMouseButton
			var action = "按下" if button_event.pressed else "释放"
			event_type = "鼠标%s(%d)" % [action, button_event.button_index]
		elif event is InputEventMouseMotion:
			if Engine.get_process_frames() % 60 == 0:  # 限制输出频率
				event_type = "鼠标移动"
		
		if event_type != "":
			print("ObjectDragHandle: %s %s 位置:%s" % [event_type, name, local_pos])

func handle_mouse_enter_from_manager():
	if not enable_hover_events or _mouse_is_over:
		return
		
	_mouse_is_over = true
	handle_mouse_entered.emit()
	
	if show_debug_info:
		print("ObjectDragHandle: 鼠标进入 ", name)

func handle_mouse_exit_from_manager():
	if not enable_hover_events or not _mouse_is_over:
		return
		
	_mouse_is_over = false
	handle_mouse_exited.emit()
	
	if show_debug_info:
		print("ObjectDragHandle: 鼠标离开 ", name)

## 检查是否可以开始拖拽
func _can_start_drag() -> bool:
	# 检测与window drag handle的冲突
	if _detect_window_drag_handle_conflict():
		push_warning("ObjectDragHandle: 检测到WindowDragHandle存在，可能导致冲突")
		if show_debug_info:
			print("ObjectDragHandle: 拖拽被阻止 - WindowDragHandle冲突")
		return false
	
	return true

## 自适应大小实现 - 基于父节点的其他子节点
func _update_size_to_fit_children():
	if not auto_size_enabled:
		return
	
	var parent = get_parent()
	if not parent:
		push_warning("ObjectDragHandle: 没有父节点，无法计算自适应大小")
		return
		
	var min_x = INF
	var min_y = INF
	var max_x = -INF
	var max_y = -INF
	var has_visible_children = false
	
	# 遍历父节点的所有子节点，但排除自己
	for child in parent.get_children():
		if child == self:  # 跳过自己，防止循环引用
			continue
		if child == _size_update_timer:
			continue
		if not child.has_method("is_visible_in_tree") or not child.visible:
			continue
			
		var child_rect = _get_child_display_rect(child)
		if child_rect.size.x > 0 and child_rect.size.y > 0:
			has_visible_children = true
			min_x = min(min_x, child_rect.position.x)
			min_y = min(min_y, child_rect.position.y)
			max_x = max(max_x, child_rect.position.x + child_rect.size.x)
			max_y = max(max_y, child_rect.position.y + child_rect.size.y)
	
	if has_visible_children:
		# 计算新的大小和位置
		var new_size = Vector2(max_x - min_x, max_y - min_y)
		var new_local_pos = Vector2(min_x, min_y)
		
		# 更新自己的位置和大小（相对于父节点）
		position = new_local_pos
		if size != new_size:
			size = new_size
			size_changed.emit(new_size)
			
		if show_debug_info and Engine.get_process_frames() % 600 == 0:  # 每10秒打印一次，而不是每次都打印
			print("ObjectDragHandle ", name, " 自适应大小: ", new_size, " 位置: ", new_local_pos)
	else:
		# 没有可见子节点，设置最小大小
		var new_size = Vector2(10, 10)
		if size != new_size:
			size = new_size
			size_changed.emit(new_size)

## 获取子节点的显示区域
func _get_child_display_rect(child: Node) -> Rect2:
	if child is Sprite2D:
		return _get_sprite2d_rect(child)
	elif child is AnimatedSprite2D:
		return _get_animated_sprite2d_rect(child)
	elif child is CollisionShape2D:
		return _get_collision_shape_rect(child)
	elif child is Area2D or child is RigidBody2D or child is CharacterBody2D:
		return _get_physics_body_rect(child)
	else:
		return Rect2()

func _get_sprite2d_rect(sprite: Sprite2D) -> Rect2:
	if not sprite.texture:
		return Rect2()
		
	var texture_size = sprite.texture.get_size()
	var sprite_size = texture_size * sprite.scale
	
	# Sprite2D 默认是 centered=true，意味着位置是图片的中心点
	var sprite_pos: Vector2
	if sprite.centered:
		# 如果是居中锚点，需要减去一半大小来得到左上角位置
		sprite_pos = sprite.position - sprite_size / 2
	else:
		# 如果不是居中，position 就是左上角
		sprite_pos = sprite.position
	
	# 考虑 offset（这是额外的偏移）
	sprite_pos -= sprite_size * sprite.offset
	
	return Rect2(sprite_pos, sprite_size)

func _get_animated_sprite2d_rect(anim_sprite: AnimatedSprite2D) -> Rect2:
	if not anim_sprite.sprite_frames:
		return Rect2()
		
	var current_texture = null
	if anim_sprite.sprite_frames.has_animation(anim_sprite.animation):
		var frame_count = anim_sprite.sprite_frames.get_frame_count(anim_sprite.animation)
		if frame_count > 0 and anim_sprite.frame < frame_count:
			current_texture = anim_sprite.sprite_frames.get_frame_texture(anim_sprite.animation, anim_sprite.frame)
	
	if not current_texture:
		return Rect2()
		
	var texture_size = current_texture.get_size()
	var sprite_size = texture_size * anim_sprite.scale

	# Sprite2D 默认是 centered=true，意味着位置是图片的中心点
	var sprite_pos: Vector2
	if anim_sprite.centered:
		# 如果是居中锚点，需要减去一半大小来得到左上角位置
		sprite_pos = anim_sprite.position - sprite_size / 2
	else:
		# 如果不是居中，position 就是左上角
		sprite_pos = anim_sprite.position
	
	# 考虑 offset（这是额外的偏移）
	sprite_pos -= sprite_size * anim_sprite.offset

	return Rect2(sprite_pos, sprite_size)

func _get_control_rect(control: Control) -> Rect2:
	# Control子节点使用其设置的rect
	return Rect2(control.position, control.size)

func _get_collision_shape_rect(collision: CollisionShape2D) -> Rect2:
	if not collision.shape:
		return Rect2()
		
	# 根据不同的形状类型计算边界
	if collision.shape is RectangleShape2D:
		var rect_shape = collision.shape as RectangleShape2D
		var half_size = rect_shape.size / 2
		return Rect2(collision.position - half_size, rect_shape.size)
	elif collision.shape is CircleShape2D:
		var circle_shape = collision.shape as CircleShape2D
		var radius = circle_shape.radius
		return Rect2(collision.position - Vector2(radius, radius), Vector2(radius * 2, radius * 2))
	
	return Rect2()

func _get_physics_body_rect(body: Node2D) -> Rect2:
	# 遍历物理体的所有CollisionShape2D子节点
	var combined_rect = Rect2()
	var has_shapes = false
	
	for child in body.get_children():
		if child is CollisionShape2D:
			var shape_rect = _get_collision_shape_rect(child)
			if shape_rect.size.x > 0 and shape_rect.size.y > 0:
				if not has_shapes:
					combined_rect = shape_rect
					has_shapes = true
				else:
					combined_rect = combined_rect.merge(shape_rect)
	
	return combined_rect

## 局部透明检测（供管理器调用） - 检测父节点的其他子节点
func is_self_opaque_at_position(local_pos: Vector2) -> bool:
	if not enable_transparency_detection:
		return true
	
	var parent = get_parent()
	if not parent:
		return false
		
	# 遍历父节点的所有子节点，但排除自己
	for child in parent.get_children():
		if child == self:  # 跳过自己，防止循环引用
			continue
		if child == _size_update_timer:
			continue
		if not child.has_method("is_visible_in_tree") or not child.visible:
			continue
			
		# 将 DragHandle 的本地坐标转换为兄弟节点的本地坐标
		# 正确的转换：DragHandle本地坐标 -> 父节点坐标 -> 兄弟节点本地坐标
		var parent_pos = local_pos + position  # DragHandle本地坐标转为父节点坐标
		var sibling_local_pos = parent_pos - child.position  # 父节点坐标转为兄弟节点本地坐标
		
		if _is_child_opaque_at_position(child, sibling_local_pos):
			return true
	
	return false

func _is_child_opaque_at_position(child: Node, local_pos: Vector2) -> bool:
	if child is Sprite2D:
		return _check_sprite2d_opacity(child, local_pos)
	elif child is AnimatedSprite2D:
		return _check_animated_sprite2d_opacity(child, local_pos)
	elif child is CollisionShape2D:
		return _check_collision_shape_contains(child, local_pos)
	
	return false

func _check_sprite2d_opacity(sprite: Sprite2D, local_pos: Vector2) -> bool:
	if not sprite.texture:
		return false
	
	# local_pos 已经是相对于 Sprite2D 的坐标（重构后无需再转换）
	var texture_size = sprite.texture.get_size()
	var sprite_local_pos = local_pos
	
	# 如果 Sprite2D 是居中的，需要调整坐标
	if sprite.centered:
		sprite_local_pos += texture_size * sprite.scale / 2
	
	# 考虑 offset
	sprite_local_pos += texture_size * sprite.scale * sprite.offset
	
	# 考虑缩放：将坐标转换回原始纹理坐标
	var texture_pos = sprite_local_pos / sprite.scale
	
	# 检查是否在纹理范围内
	if texture_pos.x < 0 or texture_pos.x >= texture_size.x or texture_pos.y < 0 or texture_pos.y >= texture_size.y:
		return false
	
	# 获取像素并检查透明度
	var image = sprite.texture.get_image()
	if image:
		var pixel_color = image.get_pixel(int(texture_pos.x), int(texture_pos.y))
		var is_opaque = pixel_color.a > opacity_threshold
		return is_opaque
	
	return false

func _check_animated_sprite2d_opacity(anim_sprite: AnimatedSprite2D, local_pos: Vector2) -> bool:
	if not anim_sprite.sprite_frames:
		return false
		
	var current_texture = null
	if anim_sprite.sprite_frames.has_animation(anim_sprite.animation):
		var frame_count = anim_sprite.sprite_frames.get_frame_count(anim_sprite.animation)
		if frame_count > 0 and anim_sprite.frame < frame_count:
			current_texture = anim_sprite.sprite_frames.get_frame_texture(anim_sprite.animation, anim_sprite.frame)
	
	if not current_texture:
		return false
	
	# local_pos 已经是相对于 AnimatedSprite2D 的坐标（重构后无需再转换）
	var texture_size = current_texture.get_size()
	var sprite_local_pos = local_pos
	
	# 考虑 offset
	sprite_local_pos += texture_size * anim_sprite.offset
	
	# 考虑缩放
	var texture_pos = sprite_local_pos / anim_sprite.scale
	
	if texture_pos.x < 0 or texture_pos.x >= texture_size.x or texture_pos.y < 0 or texture_pos.y >= texture_size.y:
		return false
	
	var image = current_texture.get_image()
	if image:
		var pixel_color = image.get_pixel(int(texture_pos.x), int(texture_pos.y))
		return pixel_color.a > opacity_threshold
	
	return false

func _check_collision_shape_contains(collision: CollisionShape2D, local_pos: Vector2) -> bool:
	if not collision.shape:
		return false
	
	# local_pos 已经是相对于 CollisionShape2D 的坐标（重构后无需再转换）
	var shape_local_pos = local_pos
	
	# 根据形状类型进行检测
	if collision.shape is RectangleShape2D:
		var rect_shape = collision.shape as RectangleShape2D
		var half_size = rect_shape.size / 2
		return abs(shape_local_pos.x) <= half_size.x and abs(shape_local_pos.y) <= half_size.y
	elif collision.shape is CircleShape2D:
		var circle_shape = collision.shape as CircleShape2D
		return shape_local_pos.length() <= circle_shape.radius
	
	return false

## 定期更新大小（处理动画等动态变化）
func _start_size_update_timer():
	if _size_update_timer:
		return
		
	_size_update_timer = Timer.new()
	_size_update_timer.wait_time = size_update_interval
	_size_update_timer.timeout.connect(_on_size_update_timer)
	_size_update_timer.autostart = true
	add_child(_size_update_timer)

func _on_size_update_timer():
	if not _is_dragging and auto_size_enabled:  # 拖拽时不更新大小，避免干扰
		_update_size_to_fit_children()

func _on_child_changed(node: Node = null):
	# 子节点变化时立即更新（node参数可能是新加入或离开的节点）
	if not _is_dragging and auto_size_enabled:
		call_deferred("_update_size_to_fit_children")
	

## 冲突检测
func _detect_window_drag_handle_conflict() -> bool:
	var window_drag_handles = []
	_recursive_find_window_drag_handles(get_tree().current_scene, window_drag_handles)
	return window_drag_handles.size() > 0

func _recursive_find_window_drag_handles(node: Node, results: Array):
	if node.get_script() and node.get_script().get_global_name() == "WindowDragHandle":
		results.append(node)
	
	for child in node.get_children():
		_recursive_find_window_drag_handles(child, results)

## 工具方法
func set_manager(manager: ObjectDragManager):
	_manager = manager

func is_dragging() -> bool:
	return _is_dragging

func get_manager() -> ObjectDragManager:
	return _manager

## 鼠标状态查询方法
func is_mouse_over() -> bool:
	return _mouse_is_over

func get_last_mouse_position() -> Vector2:
	return _last_mouse_position

## 公共API方法
func start_drag_manually(mouse_pos: Vector2) -> bool:
	"""手动开始拖拽"""
	if not _manager:
		push_warning("ObjectDragHandle: 没有管理器，无法手动开始拖拽")
		return false
	
	start_drag_from_manager(mouse_pos)
	return _is_dragging

func end_drag_manually():
	"""手动结束拖拽"""
	if _is_dragging:
		end_drag_from_manager()

func set_auto_size_enabled(enabled: bool):
	"""设置是否启用自动大小"""
	auto_size_enabled = enabled
	if enabled:
		_update_size_to_fit_children()
		if not _size_update_timer:
			_start_size_update_timer()
	elif _size_update_timer:
		_size_update_timer.queue_free()
		_size_update_timer = null

func set_manual_size(new_size: Vector2):
	"""设置手动大小"""
	manual_size = new_size
	if not auto_size_enabled:
		size = new_size
		size_changed.emit(new_size)

func refresh_size():
	"""手动刷新大小"""
	if auto_size_enabled:
		_update_size_to_fit_children()

## 清理
func _exit_tree():
	if _size_update_timer:
		_size_update_timer.queue_free()
		_size_update_timer = null
