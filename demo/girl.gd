extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

var motion_count = 0


func _get_button_name(button_index: int) -> String:
	match button_index:
		MOUSE_BUTTON_LEFT: return "左键"
		MOUSE_BUTTON_RIGHT: return "右键"
		MOUSE_BUTTON_MIDDLE: return "中键"
		_: return "按钮%d" % button_index

func _on_object_drag_handle_handle_mouse_event(event):
	# 统一的鼠标事件处理，类似 _unhandled_input
	if event is InputEventMouseButton:
		var button_event = event as InputEventMouseButton
		var button_name = _get_button_name(button_event.button_index)
		
		if button_event.pressed:
			print("🖱️ [%s] 鼠标按下: %s" % [name, button_name])
			
			if button_event.button_index == MOUSE_BUTTON_RIGHT:
				print("📋 [%s] 右键菜单 - 位置: (%.0f, %.0f)" % [name, button_event.global_position.x, button_event.global_position.y])
		else:
			print("🖱️ [%s] 鼠标释放: %s" % [name, button_name])
			
	elif event is InputEventMouseMotion:
		motion_count += 1
		# 每60次运动记录一次，避免日志过多
		if motion_count % 60 == 0:
			print("🖱️ [%s] 鼠标移动中... (已记录 %d 次)" % [name, motion_count])
	


func _on_object_drag_handle_handle_mouse_entered():
	print("🖱️ [%s] 鼠标进入 - 更改光标为移动样式" % name)
	Input.set_default_cursor_shape(Input.CURSOR_MOVE)


func _on_object_drag_handle_handle_mouse_exited():
	print("🖱️ [%s] 鼠标离开 - 恢复默认光标" % name)
	Input.set_default_cursor_shape(Input.CURSOR_ARROW)


func _on_object_drag_handle_drag_started():
	print("🖱️ [%s] 开始拖拽" % name)


func _on_object_drag_handle_drag_ended():
	print("🖱️ [%s] 结束拖拽" % name)
