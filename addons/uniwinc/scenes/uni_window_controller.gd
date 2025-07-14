extends Node
class_name UniWindowControllerWrapper

## 窗口控制器包装器 - 提供Inspector友好的接口
##
## 这个类封装了GDExtension的UniWindowController，提供可在Inspector中配置的属性

## 信号定义
signal files_dropped(files: PackedStringArray)
signal window_focus_changed(focused: bool)
signal window_moved(position: Vector2)
signal window_resized(size: Vector2)
signal monitor_changed(monitor_index: int)

## Inspector中显示的属性 - 严格按照Unity版本的顺序和分组

# 主要窗口设置 (对应Unity的SerializeField + EditableProperty)
@export var is_transparent: bool = false : set = _set_transparent
@export_range(0.0, 1.0, 0.01) var alpha_value: float = 1.0 : set = _set_alpha_value
@export var is_topmost: bool = false : set = _set_topmost
@export var is_bottommost: bool = false : set = _set_bottommost
@export var is_zoomed: bool = false : set = _set_zoomed
@export var should_fit_monitor: bool = false : set = _set_should_fit_monitor
@export var use_all_monitors: bool = false : set = _set_use_all_monitors
@export var allow_drop_files: bool = false : set = _set_allow_drop_files

# 鼠标事件穿透 (Unity重要属性)
@export var is_click_through: bool = false : set = _set_click_through

# Godot扩展功能 (Unity版本中没有但很有用)
@export var is_borderless: bool = false : set = _set_borderless

# 点击测试设置 (对应Unity的public属性)
@export var is_hit_test_enabled: bool = true : set = _set_hit_test_enabled
@export_enum("None", "Opacity", "Raycast") var hit_test_type: int = 1 : set = _set_hit_test_type
@export_range(0.0, 1.0, 0.01) var opacity_threshold: float = 0.1 : set = _set_opacity_threshold

@export_group("Advanced Settings")
@export var auto_switch_camera_background: bool = true : set = _set_auto_switch_camera_background
@export var force_windowed: bool = false : set = _set_force_windowed
@export var hide_until_init_finished: bool = false
@export var current_camera: Camera3D : set = _set_current_camera

@export_group("For Windows Only")
@export_enum("None", "Alpha", "ColorKey") var transparent_type: int = 1 : set = _set_transparent_type
@export var key_color: Color = Color(0.004, 0.0, 0.004, 0.0) : set = _set_key_color

@export_group("State (Read Only)")
@export var on_object: bool = true : set = _set_readonly_warning, get = _get_on_object
@export var picked_color: Color = Color.WHITE : set = _set_readonly_warning, get = _get_picked_color

@export_group("Internal Settings")
@export var monitor_to_fit: int = 0 : set = _set_monitor_to_fit
@export var auto_attach: bool = true
@export var auto_detach: bool = true

## 内部变量
var _native_controller  # 不指定类型，避免编译时依赖
var _is_window_attached: bool = false
var _setting_properties: bool = false  # 防止setter递归调用
var _hit_test_texture: ImageTexture  # 用于像素检测的纹理
var _is_checking_hit_test: bool = false  # 是否正在进行点击测试
var _internal_on_object: bool = true  # 内部状态，对应Unity的onObject
var _internal_picked_color: Color = Color.WHITE  # 内部状态，对应Unity的pickedColor
var _hidden_position: Vector2  # 用于hide_until_init_finished功能的临时隐藏位置
var _target_position: Vector2  # 目标位置，用于恢复
var _is_temporarily_hidden: bool = false  # 是否正在临时隐藏

## Inspector属性的setter函数 - 严格按照Unity版本的属性名

# 主要窗口设置
func _set_transparent(value: bool):
	if _setting_properties:
		return
	is_transparent = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_configure_godot_transparency(value)
		_native_controller.transparent = value

func _set_alpha_value(value: float):
	if _setting_properties:
		return
	alpha_value = clamp(value, 0.0, 1.0)
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.alpha_value = alpha_value

func _set_topmost(value: bool):
	if _setting_properties:
		return
	is_topmost = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.topmost = value

# Godot扩展功能 - borderless
func _set_borderless(value: bool):
	if _setting_properties:
		return
	is_borderless = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_configure_godot_borderless(value)
		_native_controller.borderless = value

# 鼠标事件穿透 (Unity重要属性)
func _set_click_through(value: bool):
	if _setting_properties:
		return
	is_click_through = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_click_through(value)

func _set_bottommost(value: bool):
	if _setting_properties:
		return
	is_bottommost = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_bottommost(value)

func _set_zoomed(value: bool):
	if _setting_properties:
		return
	is_zoomed = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_zoomed(value)

func _set_should_fit_monitor(value: bool):
	if _setting_properties:
		return
	should_fit_monitor = value
	# 优先级处理：use_all_monitors > should_fit_monitor
	if value and use_all_monitors:
		print("use_all_monitors已启用，should_fit_monitor设置被忽略")
		return
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_should_fit_monitor(value)
		# 修复Bug2：如果启用should_fit_monitor，立即应用
		if value:
			_native_controller.fit_to_monitor(monitor_to_fit)

func _set_use_all_monitors(value: bool):
	if _setting_properties:
		return
	use_all_monitors = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		if value:
			# 启用use_all_monitors时，自动禁用should_fit_monitor和is_zoomed
			if should_fit_monitor:
				print("启用use_all_monitors，自动禁用should_fit_monitor")
				should_fit_monitor = false
				_native_controller.set_should_fit_monitor(false)
			if is_zoomed:
				print("启用use_all_monitors，自动禁用is_zoomed")  
				is_zoomed = false
				_native_controller.set_zoomed(false)
			# 立即应用跨所有显示器的设置
			_fit_to_all_monitors()

func _set_allow_drop_files(value: bool):
	if _setting_properties:
		return
	allow_drop_files = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		print("Setting allow_drop_files to: ", value)
		_native_controller.allow_drop_files = value
		print("Allow drop files applied successfully")

# 点击测试设置  
func _set_hit_test_enabled(value: bool):
	if _setting_properties:
		return
	is_hit_test_enabled = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_hit_test_enabled(value)

func _set_hit_test_type(value: int):
	if _setting_properties:
		return
	hit_test_type = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_hit_test_type(value)

func _set_opacity_threshold(value: float):
	if _setting_properties:
		return
	opacity_threshold = clamp(value, 0.0, 1.0)
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_opacity_threshold(opacity_threshold)

# 高级设置
func _set_auto_switch_camera_background(value: bool):
	if _setting_properties:
		return
	auto_switch_camera_background = value
	# 这个功能是Godot特有的，暂时不实现

func _set_force_windowed(value: bool):
	if _setting_properties:
		return
	force_windowed = value
	# 这个功能需要在初始化时处理

func _set_current_camera(value: Camera3D):
	if _setting_properties:
		return
	current_camera = value

# Windows专用设置
func _set_transparent_type(value: int):
	if _setting_properties:
		return
	transparent_type = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_transparent_type(value)

func _set_key_color(value: Color):
	if _setting_properties:
		return
	key_color = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_key_color(value)

# 只读状态属性的setter警告
func _set_readonly_warning(value):
	push_warning("这是只读属性，不能在Inspector中修改")

# 只读状态属性的getter函数
func _get_on_object() -> bool:
	return _internal_on_object

func _get_picked_color() -> Color:
	return _internal_picked_color

# 内部设置
func _set_monitor_to_fit(value: int):
	if _setting_properties:
		return
	monitor_to_fit = value
	if _native_controller and _is_window_attached and not Engine.is_editor_hint():
		# 直接调用native方法，防止递归
		_native_controller.set_monitor_to_fit(value)

## 初始化
func _ready():
	# 在编辑器中不执行运行时逻辑
	if Engine.is_editor_hint():
		return
	
	# hide_until_init_finished 功能：立即隐藏窗口
	if hide_until_init_finished:
		# 创建临时的native controller来快速隐藏窗口
		if ClassDB.class_exists("UniWindowController"):
			var temp_controller = ClassDB.instantiate("UniWindowController")
			if temp_controller and temp_controller.attach_window():
				_hide_window_temporarily_early(temp_controller)
				temp_controller.detach_window()
		
	# 延迟初始化，确保GDExtension已加载
	call_deferred("_initialize_controller")

## 每帧更新 - 对应Unity版本的Update()方法
func _process(delta):
	# 在编辑器中不执行
	if Engine.is_editor_hint():
		return
	
	if not _native_controller or not _is_window_attached:
		return
		
	# 关键：每帧更新点击透传状态，完全对应Unity的UpdateClickThrough()
	_update_click_through()

func _initialize_controller():
	# 检查GDExtension是否可用
	if not ClassDB.class_exists("UniWindowController"):
		push_error("UniWinC GDExtension 未加载！请确保插件已正确安装。")
		return
	
	# 创建原生控制器
	_native_controller = ClassDB.instantiate("UniWindowController")  # 使用ClassDB.instantiate避免编译时依赖
	if not _native_controller:
		push_error("无法创建 UniWindowController 实例")
		return
		
	
	# 连接信号
	_connect_signals()
	
	# 等待更长时间确保窗口完全准备好，然后应用设置
	if auto_attach:
		_apply_settings()

func _connect_signals():
	if not _native_controller:
		return
	
	# 直接连接信号 - 如果信号不存在会直接报错，这样更容易发现问题
	_native_controller.files_dropped.connect(_on_files_dropped)
	_native_controller.window_focus_changed.connect(_on_window_focus_changed)
	_native_controller.window_moved.connect(_on_window_moved)
	_native_controller.window_resized.connect(_on_window_resized)
	_native_controller.monitor_changed.connect(_on_monitor_changed)
	
	print("All signals connected successfully")

func _apply_settings():
	if not _native_controller:
		return
		
	
	# 设置标志防止setter递归调用
	_setting_properties = true
	
	# 附加到当前窗口
	var attach_result = _native_controller.attach_window()
	
	if attach_result:
		_is_window_attached = true
		
	
		# 首先重置Godot窗口属性以避免冲突
		_reset_godot_window_properties()
		
		
		
		# 如果force_windowed开启且当前是全屏，则切换到窗口模式
		if force_windowed and DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN:
			DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
		
		# 设置透明度
		_configure_godot_transparency(is_transparent)
		_native_controller.transparent = is_transparent
		# 强制刷新透明效果
		if is_transparent:
			# 尝试多种方式确保透明度生效
			_force_transparency_refresh()
		
		# 设置无边框 (Godot扩展功能)
		_configure_godot_borderless(is_borderless)
		_native_controller.borderless = is_borderless
		# 强制刷新边框效果
		if is_borderless:
			_force_borderless_refresh()
			
		# 设置置顶
		_native_controller.topmost = is_topmost
		# 强制刷新置顶效果
		if is_topmost:
			_force_topmost_refresh()
			
		# 设置透明度值
		_native_controller.alpha_value = alpha_value
		
		# 修复Bug1：确保allow_drop_files在初始化时正确设置
		if allow_drop_files:
			print("Applying allow_drop_files setting during initialization: ", allow_drop_files)
			_native_controller.allow_drop_files = allow_drop_files
			print("Allow drop files initialized successfully")
		
		# 修复Bug2：应用监视器适配 - 复制Unity版本Start()中的关键逻辑
		# 在窗口附加成功后立即执行监视器适配（Unity版本的UpdateMonitorFitting()逻辑）
		# 优先级：use_all_monitors > should_fit_monitor > is_zoomed
		if use_all_monitors:
			print("应用启动时跨显示器适配")
			_fit_to_all_monitors()
		elif should_fit_monitor:
			var monitor_count = _native_controller.get_monitor_count()
			var target_monitor_index = monitor_to_fit
			
			if target_monitor_index < 0:
				target_monitor_index = 0
			if monitor_count <= target_monitor_index:
				target_monitor_index = monitor_count - 1
			
			if target_monitor_index >= 0:
				_native_controller.fit_to_monitor(target_monitor_index)
				print("应用启动时监视器适配完成: monitor " + str(target_monitor_index))
		
		# hide_until_init_finished 功能：恢复窗口位置
		if hide_until_init_finished:
			_restore_window_position()
		
	else:
		push_warning("窗口附加失败（可能在编辑器中运行）")
	
	# 清除标志
	_setting_properties = false
	
	# 启动像素检测协程（类似Unity版本的HitTestCoroutine）
	if is_hit_test_enabled and hit_test_type == 1:  # Opacity测试
		_start_hit_test_coroutine()

## 核心点击透传更新逻辑 - 完全对应Unity版本的UpdateClickThrough()
func _update_click_through():
	# 自動ヒットテスト無しならば終了
	if not is_hit_test_enabled or hit_test_type == 0:  # HitTestType.None
		return
	
	# マウスカーソル非表示状態ならば透明画素上と同扱い
	var hit = _internal_on_object
	
	if is_click_through:
		# ここまでクリックスルー状態だったら、ヒットしたときだけ戻す
		if hit:
			_set_click_through_native(false)
	else:
		# ここまでクリックスルーでなければ、透明かつヒットしなかったときだけクリックスルーとする
		if is_transparent and not hit:
			_set_click_through_native(true)

func _set_click_through_native(value: bool):
	"""直接调用native方法设置点击透传，对应Unity的SetClickThrough"""
	if _native_controller and _is_window_attached:
		_native_controller.set_click_through(value)
		is_click_through = value

## 公共API方法 - 用于运行时动态设置属性 (与Unity版本一致)
func set_transparent(transparent: bool):
	is_transparent = transparent
	if _native_controller and _is_window_attached:
		_configure_godot_transparency(transparent)
		_native_controller.transparent = transparent

# Godot扩展功能 - borderless
func set_borderless(borderless: bool):
	is_borderless = borderless
	if _native_controller and _is_window_attached:
		_configure_godot_borderless(borderless)
		_native_controller.borderless = borderless

# 鼠标事件穿透 (Unity重要属性)
func set_click_through(click_through: bool):
	is_click_through = click_through
	if _native_controller and _is_window_attached:
		_native_controller.set_click_through(click_through)

func get_click_through() -> bool:
	return is_click_through

func set_alpha_value(alpha: float):
	alpha_value = clamp(alpha, 0.0, 1.0)
	if _native_controller and _is_window_attached:
		_native_controller.alpha_value = alpha_value

func set_topmost(topmost: bool):
	is_topmost = topmost
	if _native_controller and _is_window_attached:
		_native_controller.topmost = topmost

# Unity兼容的新API方法
func set_bottommost(bottommost: bool):
	is_bottommost = bottommost
	if _native_controller and _is_window_attached:
		_native_controller.set_bottommost(bottommost)

func get_bottommost() -> bool:
	return is_bottommost

func set_zoomed(zoomed: bool):
	is_zoomed = zoomed
	if _native_controller and _is_window_attached:
		_native_controller.set_zoomed(zoomed)

func get_zoomed() -> bool:
	if _native_controller and _is_window_attached:
		return _native_controller.get_zoomed()
	return is_zoomed

func set_should_fit_monitor(should_fit: bool):
	should_fit_monitor = should_fit
	if _native_controller and _is_window_attached:
		_native_controller.set_should_fit_monitor(should_fit)

func get_should_fit_monitor() -> bool:
	return should_fit_monitor

func set_monitor_to_fit(monitor_index: int):
	monitor_to_fit = monitor_index
	if _native_controller and _is_window_attached:
		_native_controller.set_monitor_to_fit(monitor_index)

func get_monitor_to_fit() -> int:
	return monitor_to_fit

func set_transparent_type(type: int):
	transparent_type = type
	if _native_controller and _is_window_attached:
		_native_controller.set_transparent_type(type)

func get_transparent_type() -> int:
	return transparent_type

func set_key_color(color: Color):
	key_color = color
	if _native_controller and _is_window_attached:
		# 将Color转换为Vector2（r,g,b,a -> x,y,x,y）或适当的格式
		_native_controller.set_key_color(color)

func get_key_color() -> Color:
	return key_color

func set_hit_test_type(type: int):
	hit_test_type = type
	if _native_controller and _is_window_attached:
		_native_controller.set_hit_test_type(type)

func get_hit_test_type() -> int:
	return hit_test_type

func set_opacity_threshold(threshold: float):
	opacity_threshold = clamp(threshold, 0.0, 1.0)
	if _native_controller and _is_window_attached:
		_native_controller.set_opacity_threshold(opacity_threshold)

func get_opacity_threshold() -> float:
	return opacity_threshold

func set_hit_test_enabled(enabled: bool):
	is_hit_test_enabled = enabled
	if _native_controller and _is_window_attached:
		_native_controller.set_hit_test_enabled(enabled)

func get_hit_test_enabled() -> bool:
	return is_hit_test_enabled

# Unity版本中的重要方法
func set_camera(camera: Camera3D):
	current_camera = camera
	# 这里可以添加相机背景切换逻辑

func set_transparent_type_enum(type: int):
	# Unity版本用枚举参数，这里兼容
	set_transparent_type(type)

func focus():
	if _native_controller and _is_window_attached:
		_native_controller.focus()

# 新功能的公共API
func set_use_all_monitors(enabled: bool):
	"""设置是否使用跨所有显示器模式"""
	use_all_monitors = enabled
	if _native_controller and _is_window_attached:
		if enabled:
			_fit_to_all_monitors()

func get_use_all_monitors() -> bool:
	"""获取当前是否启用跨所有显示器模式"""
	return use_all_monitors

func fit_to_all_monitors_now():
	"""立即将窗口适配到所有显示器"""
	if _native_controller and _is_window_attached:
		_fit_to_all_monitors()
	else:
		print("无法执行跨显示器适配：窗口未附加")

## Unity版本兼容的属性访问器
# 窗口位置和大小 (对应Unity的windowPosition和windowSize)
var window_position: Vector2:
	get:
		if _native_controller and _is_window_attached:
			return _native_controller.position
		return Vector2.ZERO
	set(value):
		if _native_controller and _is_window_attached:
			_native_controller.position = value

var window_size: Vector2:
	get:
		if _native_controller and _is_window_attached:
			return _native_controller.size
		return Vector2.ZERO
	set(value):
		if _native_controller and _is_window_attached:
			_native_controller.size = value

# 客户端区域大小 (对应Unity的clientSize)
var client_size: Vector2:
	get:
		if _native_controller and _is_window_attached:
			return _native_controller.get_client_size()
		return Vector2.ZERO

# 鼠标光标位置 (对应Unity的cursorPosition)
var cursor_position: Vector2:
	get:
		return get_cursor_position()
	set(value):
		set_cursor_position(value)
## 静态方法 - 与Unity版本一致
func get_cursor_position() -> Vector2:
	if _native_controller:
		return _native_controller.get_cursor_position()
	return Vector2.ZERO

func set_cursor_position(position: Vector2):
	if _native_controller:
		_native_controller.set_cursor_position(position)

func get_mouse_buttons() -> int:
	if _native_controller:
		return _native_controller.get_mouse_buttons()
	return 0

func get_modifier_keys() -> int:
	if _native_controller:
		return _native_controller.get_modifier_keys()
	return 0

# Unity版本中的静态方法 - 监视器相关
func get_monitor_count() -> int:
	if _native_controller:
		return _native_controller.get_monitor_count()
	return 1

func get_monitor_rect(index: int) -> Rect2:
	if _native_controller:
		# Unity返回Rect，Godot使用Rect2
		var monitor_pos = _native_controller.get_monitor_position(index)
		var monitor_size = _native_controller.get_monitor_size(index)
		return Rect2(monitor_pos, monitor_size)
	return Rect2()

## 文件对话框方法（FilePanel替代）
func open_file_dialog(title: String = "Open File", filters: PackedStringArray = [], initial_path: String = "") -> PackedStringArray:
	# 这里需要调用新的FileDialog扩展
	# 暂时返回空数组，实际实现需要添加对应的GDExtension类
	return PackedStringArray()

func save_file_dialog(title: String = "Save File", filters: PackedStringArray = [], initial_path: String = "") -> String:
	# 这里需要调用新的FileDialog扩展
	# 暂时返回空字符串，实际实现需要添加对应的GDExtension类
	return ""

## 公共API方法 - 直接委托给原生控制器
func attach_window() -> bool:
	if not _native_controller:
		return false
	var result = _native_controller.attach_window()
	_is_window_attached = result
	return result

func detach_window():
	if _native_controller:
		_native_controller.detach_window()
	_is_window_attached = false

## 状态查询方法
func is_active() -> bool:
	if _native_controller:
		return _native_controller.is_active()
	return false

func is_maximized() -> bool:
	if _native_controller:
		return _native_controller.is_maximized()
	return false

func is_minimized() -> bool:
	if _native_controller:
		return _native_controller.is_minimized()
	return false

## 窗口控制方法
func minimize_window():
	if _native_controller:
		_native_controller.minimize_window()

func maximize_window():
	if _native_controller:
		_native_controller.maximize_window()

func restore_window():
	if _native_controller:
		_native_controller.restore_window()

func get_monitor_size(monitor_index: int) -> Vector2:
	if _native_controller:
		return _native_controller.get_monitor_size(monitor_index)
	return Vector2.ZERO

func get_current_monitor() -> int:
	if _native_controller:
		return _native_controller.get_current_monitor()
	return 0

## 获取原生控制器引用（供其他脚本使用）
func get_native_controller():  # 不指定返回类型，避免编译时依赖
	return _native_controller

## 信号回调
func _on_files_dropped(files: PackedStringArray):
	print("*** GDScript received files_dropped signal ***")
	print("Files count: ", files.size())
	for i in range(files.size()):
		print("  File ", i, ": ", files[i])
	files_dropped.emit(files)

func _on_window_focus_changed(focused: bool):
	window_focus_changed.emit(focused)

func _on_window_moved(position: Vector2):
	window_moved.emit(position)

func _on_window_resized(size: Vector2):
	window_resized.emit(size)

func _on_monitor_changed(monitor_index: int):
	monitor_changed.emit(monitor_index)
	# 修复：监视器变化时重新应用适配（Unity版本的重要逻辑）
	if should_fit_monitor:
		_force_zoomed_delayed()

## Unity版本的ForceZoomed协程等价实现
func _force_zoomed_delayed():
	# 延迟0.5秒后强制最大化（复制Unity版本的逻辑）
	await get_tree().create_timer(0.5).timeout
	if should_fit_monitor and _native_controller and _is_window_attached:
		var is_zoomed = _native_controller.get_zoomed()
		if not is_zoomed:
			_native_controller.set_zoomed(true)

## 重置Godot窗口属性以避免冲突
func _reset_godot_window_properties():
	var main_window = get_window()
	if not main_window:
		return
		
	# 重置到默认状态，然后根据组件设置来配置
	main_window.transparent = false
	main_window.borderless = false
	get_viewport().transparent_bg = false

## 配置Godot窗口设置以避免冲突
func _configure_godot_window():
	# 获取主窗口
	var main_window = get_window()
	if not main_window:
		return
		
	# 设置Godot窗口属性以配合native扩展
	if is_borderless:
		_configure_godot_borderless(true)
		
	if is_transparent:
		_configure_godot_transparency(true)

func _configure_godot_borderless(borderless: bool):
	var main_window = get_window()
	if main_window:
		main_window.borderless = borderless

func _configure_godot_transparency(transparent: bool):
	var main_window = get_window()
	if main_window:
		# 重要修复：不要设置Godot的透明属性，避免与native extension冲突
		# main_window.transparent = transparent  # 让native extension完全控制窗口透明
		# 只设置渲染背景透明
		get_viewport().transparent_bg = transparent
		
		# 确保Godot不干扰鼠标事件处理
		if transparent:
			main_window.gui_embed_subwindows = false

## 强制刷新效果的辅助方法
func _force_transparency_refresh():
	# 尝试多种方式强制透明度生效
	var main_window = get_window()
	if main_window:
		# 方法1: 切换透明状态
		main_window.transparent = false
		main_window.transparent = true
		get_viewport().transparent_bg = true
		
		# 方法2: 强制重新渲染
		RenderingServer.force_sync()
		
		# 方法3: 通过alpha值触发更新
		var current_alpha = _native_controller.alpha_value
		_native_controller.alpha_value = 0.99
		_native_controller.alpha_value = current_alpha
		

func _force_borderless_refresh():
	# 强制边框状态生效
	var main_window = get_window()
	if main_window:
		# 切换边框状态来强制更新
		main_window.borderless = false
		main_window.borderless = true
		
		# 强制窗口重新绘制
		main_window.move_to_center()
		

func _force_topmost_refresh():
	# 强制置顶状态生效
	if _native_controller:
		# 先设置为非置顶，再设置为置顶
		_native_controller.topmost = false
		_native_controller.topmost = true
		
		# 也可以尝试改变窗口位置来触发更新
		var current_pos = _native_controller.position
		_native_controller.position = current_pos + Vector2(1, 1)
		_native_controller.position = current_pos
		

func get_window() -> Window:
	# 获取主窗口的方法
	if get_viewport():
		return get_viewport().get_window()
	return null

## Unity版本兼容的像素检测系统 - 只负责检测像素，不处理点击透传
func _start_hit_test_coroutine():
	# 启动类似Unity版本的HitTestCoroutine，只检测像素状态
	if _is_checking_hit_test:
		return
		
	_is_checking_hit_test = true
	_hit_test_texture = ImageTexture.new()
	
	# 启动检测协程
	_hit_test_coroutine()

func _hit_test_coroutine():
	# 持续检测鼠标下像素的透明度，类似Unity版本的HitTestCoroutine
	# 但只更新onObject状态，不直接处理点击透传
	while _is_checking_hit_test and is_hit_test_enabled:
		await get_tree().process_frame
		
		if hit_test_type == 1:  # HitTestType.Opacity
			_hit_test_by_opaque_pixel()
		elif hit_test_type == 2:  # HitTestType.Raycast  
			_hit_test_by_raycast()
		else:
			# ヒットテスト無しの場合は常にtrue
			_internal_on_object = true

func _hit_test_by_opaque_pixel():
	# 检测鼠标下是否有不透明像素，对应Unity版本的HitTestByOpaquePixel
	var mouse_pos = _get_client_cursor_position()
	var screen_size = get_viewport().get_visible_rect().size
	
	# 检查鼠标是否在屏幕范围内
	if mouse_pos.x < 0 or mouse_pos.x >= screen_size.x or mouse_pos.y < 0 or mouse_pos.y >= screen_size.y:
		_internal_on_object = false
		return
		
	# 如果不是透明状态，范围内就算不透明
	if not is_transparent:
		_internal_on_object = true
		return
		
	# 对应Unity版本中透明度为ColorKey时的处理
	if transparent_type == 2:  # ColorKey
		_internal_on_object = true
		return
		
	# 获取当前渲染的图像进行像素检测
	var viewport_texture = get_viewport().get_texture()
	if not viewport_texture:
		_internal_on_object = false
		return
		
	var image = viewport_texture.get_image()
	if not image:
		_internal_on_object = false
		return
		
	# 检查指定位置的像素
	var pixel_color = image.get_pixel(int(mouse_pos.x), int(mouse_pos.y))
	_internal_picked_color = pixel_color  # 更新picked_color状态
	
	# 对应Unity版本：return (color.a >= opacityThreshold)
	_internal_on_object = (pixel_color.a >= opacity_threshold)

func _hit_test_by_raycast():
	# 实现Raycast检测，对应Unity版本的HitTestByRaycast
	# 暂时简化实现
	_internal_on_object = true

func _get_client_cursor_position() -> Vector2:
	# 获取客户端相对鼠标位置，类似Unity版本的GetClientCursorPosition
	var global_mouse_pos = DisplayServer.mouse_get_position()
	var window = get_window()
	if not window:
		return Vector2.ZERO
		
	var window_pos = window.position
	return global_mouse_pos - window_pos

## 新功能实现
func _fit_to_all_monitors():
	"""将窗口扩展到适配所有显示器的区域"""
	if not _native_controller or not _is_window_attached:
		print("无法应用跨显示器设置：窗口未附加")
		return
		
	var monitor_count = _native_controller.get_monitor_count()
	if monitor_count <= 0:
		print("无法获取显示器信息")
		return
		
	print("开始计算跨所有显示器的窗口区域，显示器数量：", monitor_count)
	
	# 计算包含所有显示器的最小边界矩形
	var min_x = 99999.0
	var min_y = 99999.0
	var max_x = -99999.0
	var max_y = -99999.0
	
	for i in range(monitor_count):
		var monitor_pos = _native_controller.get_monitor_position(i)
		var monitor_size = _native_controller.get_monitor_size(i)
		
		print("显示器 ", i, " - 位置：", monitor_pos, " 大小：", monitor_size)
		
		# 更新边界
		min_x = min(min_x, monitor_pos.x)
		min_y = min(min_y, monitor_pos.y)
		max_x = max(max_x, monitor_pos.x + monitor_size.x)
		max_y = max(max_y, monitor_pos.y + monitor_size.y)
	
	# 计算包含所有显示器的窗口位置和大小
	var all_monitors_pos = Vector2(min_x, min_y)
	var all_monitors_size = Vector2(max_x - min_x, max_y - min_y)
	
	print("跨显示器窗口区域 - 位置：", all_monitors_pos, " 大小：", all_monitors_size)
	
	# 设置窗口位置和大小
	_native_controller.position = all_monitors_pos
	_native_controller.size = all_monitors_size
	
	print("跨显示器窗口设置完成")

func _hide_window_temporarily():
	"""临时隐藏窗口到所有显示器区域外"""
	if not _native_controller:
		return
		
	# 保存当前位置作为目标位置
	_target_position = _native_controller.position
	
	# 计算所有显示器的边界
	var monitor_count = _native_controller.get_monitor_count()
	var max_x = -99999.0
	
	for i in range(monitor_count):
		var monitor_pos = _native_controller.get_monitor_position(i)
		var monitor_size = _native_controller.get_monitor_size(i)
		max_x = max(max_x, monitor_pos.x + monitor_size.x)
	
	# 将窗口移动到所有显示器右侧外部区域
	_hidden_position = Vector2(max_x + 1000, _target_position.y)
	_native_controller.position = _hidden_position
	_is_temporarily_hidden = true
	
	print("窗口已临时隐藏到位置：", _hidden_position)

func _hide_window_temporarily_early(temp_controller):
	"""在初始化阶段使用临时控制器快速隐藏窗口"""
	if not temp_controller:
		return
		
	# 保存当前位置作为目标位置（使用Godot的当前窗口位置）
	var main_window = get_window()
	if main_window:
		_target_position = Vector2(main_window.position.x, main_window.position.y)
	else:
		_target_position = Vector2(100, 100)  # 默认位置
	
	# 计算所有显示器的边界
	var monitor_count = temp_controller.get_monitor_count()
	var max_x = -99999.0
	
	for i in range(monitor_count):
		var monitor_pos = temp_controller.get_monitor_position(i)
		var monitor_size = temp_controller.get_monitor_size(i)
		max_x = max(max_x, monitor_pos.x + monitor_size.x)
	
	# 将窗口移动到所有显示器右侧外部区域
	_hidden_position = Vector2(max_x + 1000, _target_position.y)
	temp_controller.position = _hidden_position
	_is_temporarily_hidden = true
	
	print("窗口已在初始化阶段临时隐藏到位置：", _hidden_position)

func _restore_window_position():
	"""恢复窗口到目标位置"""
	if not _is_temporarily_hidden or not _native_controller:
		return
		
	# 如果启用了use_all_monitors，目标位置应该是跨显示器区域
	if use_all_monitors:
		_fit_to_all_monitors()
	else:
		_native_controller.position = _target_position
	
	_is_temporarily_hidden = false
	print("窗口位置已恢复")


## 清理
func _exit_tree():
	# 停止像素检测协程
	_is_checking_hit_test = false
	
	if not Engine.is_editor_hint() and _native_controller and auto_detach:
		detach_window()
		_native_controller = null
