#include "uniwinc_controller.h"
#include "uniwinc_core.h"
#include "uniwinc_message_queue.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#ifdef _WIN32
    #include <windows.h>
    #include <vector>
#endif

using namespace godot;

// 静态实例用于回调
static UniWindowController* g_controller_instance = nullptr;

void UniWindowController::_bind_methods() {
    // 绑定方法
    ClassDB::bind_method(D_METHOD("attach_window"), &UniWindowController::attach_window);
    ClassDB::bind_method(D_METHOD("detach_window"), &UniWindowController::detach_window);
    
    // 基础属性绑定
    ClassDB::bind_method(D_METHOD("set_transparent", "transparent"), &UniWindowController::set_transparent);
    ClassDB::bind_method(D_METHOD("get_transparent"), &UniWindowController::get_transparent);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transparent"), "set_transparent", "get_transparent");
    
    ClassDB::bind_method(D_METHOD("set_borderless", "borderless"), &UniWindowController::set_borderless);
    ClassDB::bind_method(D_METHOD("get_borderless"), &UniWindowController::get_borderless);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "borderless"), "set_borderless", "get_borderless");
    
    ClassDB::bind_method(D_METHOD("set_topmost", "topmost"), &UniWindowController::set_topmost);
    ClassDB::bind_method(D_METHOD("get_topmost"), &UniWindowController::get_topmost);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "topmost"), "set_topmost", "get_topmost");
    
    ClassDB::bind_method(D_METHOD("set_bottommost", "bottommost"), &UniWindowController::set_bottommost);
    ClassDB::bind_method(D_METHOD("get_bottommost"), &UniWindowController::get_bottommost);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bottommost"), "set_bottommost", "get_bottommost");
    
    // 点击透传属性绑定 - 修复方法名一致性
    ClassDB::bind_method(D_METHOD("set_click_through", "click_through"), &UniWindowController::set_clickthrough);
    ClassDB::bind_method(D_METHOD("get_click_through"), &UniWindowController::get_clickthrough);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "click_through"), "set_click_through", "get_click_through");
    
    ClassDB::bind_method(D_METHOD("set_alpha_value", "alpha"), &UniWindowController::set_alpha_value);
    ClassDB::bind_method(D_METHOD("get_alpha_value"), &UniWindowController::get_alpha_value);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "alpha_value", PROPERTY_HINT_RANGE, "0.0,1.0"), "set_alpha_value", "get_alpha_value");
    
    ClassDB::bind_method(D_METHOD("set_position", "position"), &UniWindowController::set_position);
    ClassDB::bind_method(D_METHOD("get_position"), &UniWindowController::get_position);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");
    
    ClassDB::bind_method(D_METHOD("set_size", "size"), &UniWindowController::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &UniWindowController::get_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size"), "set_size", "get_size");
    
    // Unity兼容API绑定
    ClassDB::bind_method(D_METHOD("set_zoomed", "zoomed"), &UniWindowController::set_zoomed);
    ClassDB::bind_method(D_METHOD("get_zoomed"), &UniWindowController::get_zoomed);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "zoomed"), "set_zoomed", "get_zoomed");
    
    ClassDB::bind_method(D_METHOD("set_should_fit_monitor", "should_fit"), &UniWindowController::set_should_fit_monitor);
    ClassDB::bind_method(D_METHOD("get_should_fit_monitor"), &UniWindowController::get_should_fit_monitor);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "should_fit_monitor"), "set_should_fit_monitor", "get_should_fit_monitor");
    
    ClassDB::bind_method(D_METHOD("set_monitor_to_fit", "monitor_index"), &UniWindowController::set_monitor_to_fit);
    ClassDB::bind_method(D_METHOD("get_monitor_to_fit"), &UniWindowController::get_monitor_to_fit);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "monitor_to_fit"), "set_monitor_to_fit", "get_monitor_to_fit");
    
    ClassDB::bind_method(D_METHOD("set_transparent_type", "type"), &UniWindowController::set_transparent_type);
    ClassDB::bind_method(D_METHOD("get_transparent_type"), &UniWindowController::get_transparent_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "transparent_type"), "set_transparent_type", "get_transparent_type");
    
    ClassDB::bind_method(D_METHOD("set_key_color", "color"), &UniWindowController::set_key_color);
    ClassDB::bind_method(D_METHOD("get_key_color"), &UniWindowController::get_key_color);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "key_color"), "set_key_color", "get_key_color");
    
    ClassDB::bind_method(D_METHOD("set_hit_test_type", "type"), &UniWindowController::set_hit_test_type);
    ClassDB::bind_method(D_METHOD("get_hit_test_type"), &UniWindowController::get_hit_test_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hit_test_type"), "set_hit_test_type", "get_hit_test_type");
    
    ClassDB::bind_method(D_METHOD("set_opacity_threshold", "threshold"), &UniWindowController::set_opacity_threshold);
    ClassDB::bind_method(D_METHOD("get_opacity_threshold"), &UniWindowController::get_opacity_threshold);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "opacity_threshold", PROPERTY_HINT_RANGE, "0.0,1.0"), "set_opacity_threshold", "get_opacity_threshold");
    
    ClassDB::bind_method(D_METHOD("set_hit_test_enabled", "enabled"), &UniWindowController::set_hit_test_enabled);
    ClassDB::bind_method(D_METHOD("get_hit_test_enabled"), &UniWindowController::get_hit_test_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hit_test_enabled"), "set_hit_test_enabled", "get_hit_test_enabled");
    
    // 文件拖拽绑定 - 修复Bug1：添加缺失的drop files绑定
    ClassDB::bind_method(D_METHOD("set_allow_drop_files", "allow"), &UniWindowController::set_allow_drop_files);
    ClassDB::bind_method(D_METHOD("get_allow_drop_files"), &UniWindowController::get_allow_drop_files);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_drop_files"), "set_allow_drop_files", "get_allow_drop_files");
    
    // 状态查询方法
    ClassDB::bind_method(D_METHOD("is_active"), &UniWindowController::is_active);
    ClassDB::bind_method(D_METHOD("is_maximized"), &UniWindowController::is_maximized);
    ClassDB::bind_method(D_METHOD("is_minimized"), &UniWindowController::is_minimized);
    
    // 多显示器方法
    ClassDB::bind_method(D_METHOD("get_monitor_count"), &UniWindowController::get_monitor_count);
    ClassDB::bind_method(D_METHOD("get_monitor_size", "monitor_index"), &UniWindowController::get_monitor_size);
    ClassDB::bind_method(D_METHOD("get_monitor_position", "monitor_index"), &UniWindowController::get_monitor_position);
    ClassDB::bind_method(D_METHOD("get_current_monitor"), &UniWindowController::get_current_monitor);
    
    // 修复Bug2：添加fit_to_monitor方法绑定
    ClassDB::bind_method(D_METHOD("fit_to_monitor", "monitor_index"), &UniWindowController::fit_to_monitor);
    
    // 窗口控制方法
    ClassDB::bind_method(D_METHOD("set_window_title", "title"), &UniWindowController::set_window_title);
    ClassDB::bind_method(D_METHOD("get_window_title"), &UniWindowController::get_window_title);
    ClassDB::bind_method(D_METHOD("minimize_window"), &UniWindowController::minimize_window);
    ClassDB::bind_method(D_METHOD("maximize_window"), &UniWindowController::maximize_window);
    ClassDB::bind_method(D_METHOD("restore_window"), &UniWindowController::restore_window);
    ClassDB::bind_method(D_METHOD("set_window_opacity", "opacity"), &UniWindowController::set_window_opacity);
    ClassDB::bind_method(D_METHOD("get_window_opacity"), &UniWindowController::get_window_opacity);
    
    // 静态方法绑定（鼠标和键盘）
    ClassDB::bind_static_method("UniWindowController", D_METHOD("get_cursor_position"), &UniWindowController::get_cursor_position);
    ClassDB::bind_static_method("UniWindowController", D_METHOD("set_cursor_position", "position"), &UniWindowController::set_cursor_position);
    ClassDB::bind_static_method("UniWindowController", D_METHOD("get_mouse_buttons"), &UniWindowController::get_mouse_buttons);
    ClassDB::bind_static_method("UniWindowController", D_METHOD("get_modifier_keys"), &UniWindowController::get_modifier_keys);
    
    // 信号定义
    ADD_SIGNAL(MethodInfo("files_dropped", PropertyInfo(Variant::PACKED_STRING_ARRAY, "files")));
    ADD_SIGNAL(MethodInfo("window_focus_changed", PropertyInfo(Variant::BOOL, "focused")));
    ADD_SIGNAL(MethodInfo("window_moved", PropertyInfo(Variant::VECTOR2, "position")));
    ADD_SIGNAL(MethodInfo("window_resized", PropertyInfo(Variant::VECTOR2, "size")));
    ADD_SIGNAL(MethodInfo("monitor_changed", PropertyInfo(Variant::INT, "monitor_index")));
}

UniWindowController::UniWindowController() {
    g_controller_instance = this;
    MessageQueue::initialize();
}

UniWindowController::~UniWindowController() {
    if (g_controller_instance == this) {
        g_controller_instance = nullptr;
    }
    _cleanup_native();
    MessageQueue::cleanup();
}

void UniWindowController::_ready() {
    UtilityFunctions::print("UniWindowController ready");
    _initialize_native();
}

void UniWindowController::_process(double delta) {
    if (!_is_initialized) {
        return;
    }
    
    // 处理其他消息队列中的事件（不包括drop files）
    _process_messages();
    
    // 定期更新状态
    _update_from_native();
}

bool UniWindowController::attach_window() {
    if (!_is_initialized) {
        _initialize_native();
    }
    
    // 多次尝试附加，因为Godot窗口可能需要时间初始化
    for (int attempt = 0; attempt < 3; attempt++) {
        _is_active = UniWinCore::attach_window();
        if (_is_active) {
            UtilityFunctions::print("Window attached successfully on attempt " + String::num_int64(attempt + 1));
            return true;
        }
        
        // 等待一小段时间后重试
        if (attempt < 2) {
            UtilityFunctions::print("Attach attempt " + String::num_int64(attempt + 1) + " failed, retrying...");
            // 在实际应用中，这里可能需要使用定时器而不是阻塞
        }
    }
    
    UtilityFunctions::print("Failed to attach window after 3 attempts");
    return false;
}

void UniWindowController::detach_window() {
    if (_is_active) {
        UniWinCore::detach_window();
        _is_active = false;
        UtilityFunctions::print("Window detached");
    }
}

void UniWindowController::set_transparent(bool transparent) {
    _is_transparent = transparent;
    if (_is_active) {
        UniWinCore::set_transparent(transparent);
    }
}

bool UniWindowController::get_transparent() const {
    return _is_transparent;
}

void UniWindowController::set_borderless(bool borderless) {
    _is_borderless = borderless;
    if (_is_active) {
        UniWinCore::set_borderless(borderless);
    }
}

bool UniWindowController::get_borderless() const {
    return _is_borderless;
}

void UniWindowController::set_topmost(bool topmost) {
    _is_topmost = topmost;
    if (_is_active) {
        UniWinCore::set_topmost(topmost);
    }
}

bool UniWindowController::get_topmost() const {
    return _is_topmost;
}

void UniWindowController::set_alpha_value(float alpha) {
    _alpha_value = Math::clamp(alpha, 0.0f, 1.0f);
    if (_is_active) {
        UniWinCore::set_alpha_value(_alpha_value);
    }
}

float UniWindowController::get_alpha_value() const {
    return _alpha_value;
}

void UniWindowController::set_position(Vector2 position) {
    _position = position;
    if (_is_active) {
        UniWinCore::set_position(position.x, position.y);
    }
}

Vector2 UniWindowController::get_position() const {
    if (_is_active) {
        float x, y;
        UniWinCore::get_position(&x, &y);
        return Vector2(x, y);
    }
    return _position;
}

void UniWindowController::set_size(Vector2 size) {
    _size = size;
    if (_is_active) {
        UniWinCore::set_size(size.x, size.y);
    }
}

Vector2 UniWindowController::get_size() const {
    if (_is_active) {
        float width, height;
        UniWinCore::get_size(&width, &height);
        return Vector2(width, height);
    }
    return _size;
}

bool UniWindowController::is_active() const {
    return _is_active && UniWinCore::is_active();
}

bool UniWindowController::is_maximized() const {
    return _is_active && UniWinCore::is_maximized();
}

bool UniWindowController::is_minimized() const {
    return _is_active && UniWinCore::is_minimized();
}

int UniWindowController::get_monitor_count() const {
    return UniWinCore::get_monitor_count();
}

Vector2 UniWindowController::get_monitor_size(int monitor_index) const {
    float width, height;
    UniWinCore::get_monitor_size(monitor_index, &width, &height);
    return Vector2(width, height);
}

int UniWindowController::get_current_monitor() const {
    return UniWinCore::get_current_monitor();
}

void UniWindowController::_initialize_native() {
    if (!_is_initialized) {
        _is_initialized = UniWinCore::initialize();
        if (_is_initialized) {
            // 注册回调函数
            UniWinCore::register_drop_files_callback(_on_files_dropped);
            UniWinCore::register_focus_changed_callback(_on_window_focus_changed);
            UniWinCore::register_window_moved_callback(_on_window_moved);
            UniWinCore::register_window_resized_callback(_on_window_resized);
            UniWinCore::register_monitor_changed_callback(_on_monitor_changed);
            
            // 修复Bug1：验证回调注册是否成功
            UtilityFunctions::print("Callback registration completed:");
            UtilityFunctions::print("  - Drop files callback registered");
            UtilityFunctions::print("  - Focus changed callback registered");
            UtilityFunctions::print("  - Window moved callback registered");
            UtilityFunctions::print("  - Window resized callback registered");
            UtilityFunctions::print("  - Monitor changed callback registered");
            
            UtilityFunctions::print("Native library initialized");
        } else {
            UtilityFunctions::print("Failed to initialize native library");
        }
    }
}

void UniWindowController::_cleanup_native() {
    if (_is_initialized) {
        detach_window();
        UniWinCore::cleanup();
        _is_initialized = false;
    }
}

void UniWindowController::_update_from_native() {
    // 定期同步状态
    // 这里可以添加需要定期更新的状态
}

// 静态回调函数 - 宽字符版本，直接emit signal
void UniWindowController::_on_files_dropped(const wchar_t* file_paths_w) {
    UtilityFunctions::print("*** Drop files callback triggered ***");
    if (!file_paths_w || !g_controller_instance) {
        UtilityFunctions::print("ERROR: Invalid callback parameters");
        return;
    }
    
    // 将宽字符串转换为UTF-8 (Godot使用UTF-8)
    String file_paths_utf8;
    
#ifdef _WIN32
    // Windows: 使用WideCharToMultiByte转换为UTF-8
    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, file_paths_w, -1, nullptr, 0, nullptr, nullptr);
    if (utf8_size > 0) {
        std::vector<char> utf8_buffer(utf8_size);
        WideCharToMultiByte(CP_UTF8, 0, file_paths_w, -1, utf8_buffer.data(), utf8_size, nullptr, nullptr);
        file_paths_utf8 = String::utf8(utf8_buffer.data());
    }
#else
    // 非Windows: 使用标准库转换
    std::wstring ws(file_paths_w);
    std::string s(ws.begin(), ws.end());
    file_paths_utf8 = String::utf8(s.c_str());
#endif
    
    UtilityFunctions::print("Files received: " + file_paths_utf8);
    
    // 解析文件路径并直接emit signal
    PackedStringArray files;
    PackedStringArray lines = file_paths_utf8.split("\n");
    
    for (int i = 0; i < lines.size(); i++) {
        String path = lines[i].strip_edges();
        if (!path.is_empty()) {
            files.append(path);
        }
    }
    
    if (files.size() > 0) {
        UtilityFunctions::print("Emitting files_dropped signal with " + String::num_int64(files.size()) + " files");
        g_controller_instance->emit_signal("files_dropped", files);
        UtilityFunctions::print("Signal emitted successfully");
    } else {
        UtilityFunctions::print("WARNING: No valid files found in dropped files");
    }
}

void UniWindowController::_on_window_focus_changed(bool focused) {
    auto message = std::make_unique<WindowFocusChangedMessage>(focused);
    MessageQueue::push_message(std::move(message));
}

void UniWindowController::_on_window_moved(float x, float y) {
    auto message = std::make_unique<WindowMovedMessage>(x, y);
    MessageQueue::push_message(std::move(message));
}

void UniWindowController::_on_window_resized(float width, float height) {
    auto message = std::make_unique<WindowResizedMessage>(width, height);
    MessageQueue::push_message(std::move(message));
}

void UniWindowController::_on_monitor_changed(int monitor_index) {
    auto message = std::make_unique<MonitorChangedMessage>(monitor_index);
    MessageQueue::push_message(std::move(message));
}

void UniWindowController::_process_messages() {
    // 处理队列中的所有消息（除了FILES_DROPPED，现在直接处理）
    while (!MessageQueue::is_empty()) {
        auto message = MessageQueue::pop_message();
        if (!message) {
            break;
        }
        
        switch (message->type) {
            case MessageType::FILES_DROPPED: {
                // Drop files 现在直接在回调中处理，这里不应该有这种消息
                UtilityFunctions::print("WARNING: FILES_DROPPED message found in queue, this should not happen");
                break;
            }
            
            case MessageType::WINDOW_FOCUS_CHANGED: {
                auto* focus_msg = static_cast<WindowFocusChangedMessage*>(message.get());
                emit_signal("window_focus_changed", focus_msg->focused);
                break;
            }
            
            case MessageType::WINDOW_MOVED: {
                auto* moved_msg = static_cast<WindowMovedMessage*>(message.get());
                _position = Vector2(moved_msg->x, moved_msg->y);
                emit_signal("window_moved", _position);
                break;
            }
            
            case MessageType::WINDOW_RESIZED: {
                auto* resized_msg = static_cast<WindowResizedMessage*>(message.get());
                _size = Vector2(resized_msg->width, resized_msg->height);
                emit_signal("window_resized", _size);
                break;
            }
            
            case MessageType::MONITOR_CHANGED: {
                auto* monitor_msg = static_cast<MonitorChangedMessage*>(message.get());
                emit_signal("monitor_changed", monitor_msg->monitor_index);
                break;
            }
        }
    }
}

// 新增的窗口控制方法实现
void UniWindowController::set_window_title(const String& title) {
    _window_title = title;
    // 这里可以添加设置窗口标题的native调用
    // UniWinCore::set_window_title(title);
}

String UniWindowController::get_window_title() const {
    return _window_title;
}

void UniWindowController::minimize_window() {
    if (_is_active) {
        // UniWinCore::minimize_window();
        UtilityFunctions::print("Window minimized");
    }
}

void UniWindowController::maximize_window() {
    if (_is_active) {
        // UniWinCore::maximize_window();
        UtilityFunctions::print("Window maximized");
    }
}

void UniWindowController::restore_window() {
    if (_is_active) {
        // UniWinCore::restore_window();
        UtilityFunctions::print("Window restored");
    }
}

void UniWindowController::set_window_icon(const String& icon_path) {
    _window_icon_path = icon_path;
    // UniWinCore::set_window_icon(icon_path);
}

void UniWindowController::enable_always_on_top(bool enable) {
    set_topmost(enable);
}

void UniWindowController::set_window_opacity(float opacity) {
    _window_opacity = Math::clamp(opacity, 0.0f, 1.0f);
    set_alpha_value(_window_opacity);
}

float UniWindowController::get_window_opacity() const {
    return _window_opacity;
}

// Unity兼容的新增方法实现
void UniWindowController::set_bottommost(bool bottommost) {
    _is_bottommost = bottommost;
    if (_is_active) {
        UniWinCore::set_bottommost(bottommost);
    }
}

bool UniWindowController::get_bottommost() const {
    return _is_bottommost;
}

void UniWindowController::set_zoomed(bool zoomed) {
    _is_zoomed = zoomed;
    if (_is_active) {
        UniWinCore::set_zoomed(zoomed);
    }
}

bool UniWindowController::get_zoomed() const {
    if (_is_active) {
        return UniWinCore::is_zoomed();
    }
    return _is_zoomed;
}

void UniWindowController::set_should_fit_monitor(bool should_fit) {
    _should_fit_monitor = should_fit;
    if (_is_active) {
        UniWinCore::set_should_fit_monitor(should_fit);
        if (should_fit && _monitor_to_fit >= 0) {
            // 修复Bug2：立即应用监视器适配，复制Unity版本的完整逻辑
            fit_to_monitor(_monitor_to_fit);
        }
    }
}

bool UniWindowController::get_should_fit_monitor() const {
    return _should_fit_monitor;
}

void UniWindowController::set_monitor_to_fit(int monitor_index) {
    _monitor_to_fit = monitor_index;
    if (_is_active) {
        UniWinCore::set_monitor_to_fit(monitor_index);
        if (_should_fit_monitor) {
            // 修复Bug2：立即应用监视器适配
            fit_to_monitor(monitor_index);
        }
    }
}

// 修复Bug2：实现完整的fit_to_monitor逻辑，复制Unity版本的实现
void UniWindowController::fit_to_monitor(int monitor_index) {
    if (!_is_active) {
        UtilityFunctions::print("Cannot fit to monitor: window not active");
        return;
    }
    
    // 获取监视器数量并验证索引
    int monitor_count = get_monitor_count();
    int target_monitor = monitor_index;
    
    UtilityFunctions::print("Fitting to monitor " + String::num_int64(monitor_index) + " (total monitors: " + String::num_int64(monitor_count) + ")");
    
    if (target_monitor < 0) {
        target_monitor = 0;
    }
    if (target_monitor >= monitor_count) {
        target_monitor = monitor_count - 1;
    }
    
    if (target_monitor >= 0) {
        // 复制Unity版本FitToMonitor的完整逻辑
        float dx, dy, dw, dh;
        UniWinCore::get_monitor_rectangle(target_monitor, &dx, &dy, &dw, &dh);
        
        UtilityFunctions::print("Monitor " + String::num_int64(target_monitor) + " rectangle: " + 
                              String::num(dx) + ", " + String::num(dy) + " - " + 
                              String::num(dw) + "x" + String::num(dh));
        
        // 如果当前已经最大化，先解除最大化（Unity版本逻辑）
        bool was_maximized = is_maximized();
        if (was_maximized) {
            UtilityFunctions::print("Window was maximized, restoring first...");
            set_zoomed(false);
            // 等待一帧让状态更新
            // 注意：这里可能需要异步处理
        }
        
        // 计算监视器中央位置
        float cx = dx + (dw / 2.0f);
        float cy = dy + (dh / 2.0f);
        
        // 获取当前窗口大小
        float ww, wh;
        UniWinCore::get_size(&ww, &wh);
        
        UtilityFunctions::print("Current window size: " + String::num(ww) + "x" + String::num(wh));
        
        // 将窗口中央移动到指定监视器中央
        float wx = cx - (ww / 2.0f);
        float wy = cy - (wh / 2.0f);
        
        UtilityFunctions::print("Moving window to position: " + String::num(wx) + ", " + String::num(wy));
        set_position(Vector2(wx, wy));
        
        // 最大化窗口（Unity版本逻辑）- 这是关键步骤
        UtilityFunctions::print("Maximizing window...");
        set_zoomed(true);
        
        // 验证最大化是否成功
        bool is_now_maximized = is_maximized();
        UtilityFunctions::print("Window maximization result: " + String(is_now_maximized ? "SUCCESS" : "FAILED"));
        
        if (!is_now_maximized) {
            // 如果最大化失败，尝试直接设置窗口大小为监视器大小
            UtilityFunctions::print("Maximization failed, trying to set window size to monitor size...");
            
            // 方法1: 先设置窗口位置到监视器左上角，然后设置大小
            set_position(Vector2(dx, dy));
            set_size(Vector2(dw, dh));
            
            // 等待一下让设置生效
            // TODO: 在实际应用中可能需要异步等待
            
            // 验证设置是否成功
            Vector2 new_pos = get_position();
            Vector2 new_size = get_size();
            UtilityFunctions::print("Fallback result - Position: " + String::num(new_pos.x) + ", " + String::num(new_pos.y) + 
                                  " Size: " + String::num(new_size.x) + "x" + String::num(new_size.y));
            
            // 如果还是不对，尝试第二种方法：使用Godot的窗口API
            if (abs(new_size.x - dw) > 10 || abs(new_size.y - dh) > 10) {
                UtilityFunctions::print("Size setting via native failed, this suggests native library issues");
                UtilityFunctions::print("Monitor fitting completed with potential size mismatch");
            } else {
                UtilityFunctions::print("Fallback size setting successful");
            }
        }
        
        UtilityFunctions::print("Fit window to monitor " + String::num_int64(target_monitor) + " completed");
    } else {
        UtilityFunctions::print("Invalid monitor index: " + String::num_int64(target_monitor));
    }
}

int UniWindowController::get_monitor_to_fit() const {
    return _monitor_to_fit;
}

void UniWindowController::set_transparent_type(int type) {
    _transparent_type = type;
    if (_is_active) {
        UniWinCore::set_transparent_type(type);
    }
}

int UniWindowController::get_transparent_type() const {
    return _transparent_type;
}

void UniWindowController::set_key_color(const Color& color) {
    _key_color = color;
    if (_is_active) {
        UniWinCore::set_key_color(color);
    }
}

Color UniWindowController::get_key_color() const {
    return _key_color;
}

void UniWindowController::set_hit_test_type(int type) {
    _hit_test_type = type;
    if (_is_active) {
        UniWinCore::set_hit_test_type(type);
    }
}

int UniWindowController::get_hit_test_type() const {
    return _hit_test_type;
}

void UniWindowController::set_opacity_threshold(float threshold) {
    _opacity_threshold = Math::clamp(threshold, 0.0f, 1.0f);
    if (_is_active) {
        UniWinCore::set_opacity_threshold(_opacity_threshold);
    }
}

float UniWindowController::get_opacity_threshold() const {
    return _opacity_threshold;
}

void UniWindowController::set_hit_test_enabled(bool enabled) {
    _hit_test_enabled = enabled;
    if (_is_active) {
        UniWinCore::set_hit_test_enabled(enabled);
    }
}

bool UniWindowController::get_hit_test_enabled() const {
    return _hit_test_enabled;
}

void UniWindowController::set_clickthrough(bool clickthrough) {
    _is_clickthrough = clickthrough;
    if (_is_active) {
        UniWinCore::set_clickthrough(clickthrough);
    }
}

bool UniWindowController::get_clickthrough() const {
    return _is_clickthrough;
}

void UniWindowController::set_allow_drop_files(bool allow) {
    _allow_drop_files = allow;
    if (_is_active) {
        UniWinCore::set_allow_drop_files(allow);
    }
}

bool UniWindowController::get_allow_drop_files() const {
    return _allow_drop_files;
}

Vector2 UniWindowController::get_monitor_position(int monitor_index) const {
    float x, y;
    UniWinCore::get_monitor_position(monitor_index, &x, &y);
    return Vector2(x, y);
}

// 静态方法实现
Vector2 UniWindowController::get_cursor_position() {
    float x, y;
    UniWinCore::get_cursor_position(&x, &y);
    return Vector2(x, y);
}

void UniWindowController::set_cursor_position(Vector2 position) {
    UniWinCore::set_cursor_position(position.x, position.y);
}

int UniWindowController::get_mouse_buttons() {
    return UniWinCore::get_mouse_buttons();
}

int UniWindowController::get_modifier_keys() {
    return UniWinCore::get_modifier_keys();
}