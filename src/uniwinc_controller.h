#ifndef UNIWINC_CONTROLLER_H
#define UNIWINC_CONTROLLER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/color.hpp>
#include "uniwinc_message_queue.h"

using namespace godot;

class UniWindowController : public Node {
    GDCLASS(UniWindowController, Node)

private:
    // 窗口状态
    bool _is_transparent = false;
    bool _is_borderless = false;
    bool _is_topmost = false;
    bool _is_bottommost = false;
    bool _is_clickthrough = false;
    bool _allow_drop_files = false;
    bool _is_zoomed = false;
    bool _should_fit_monitor = false;
    bool _hit_test_enabled = true;
    float _alpha_value = 1.0f;
    float _opacity_threshold = 0.1f;
    int _monitor_to_fit = 0;
    int _transparent_type = 1; // Alpha
    int _hit_test_type = 1;    // Opacity
    Color _key_color = Color(1.0f, 0.0f, 1.0f, 0.0f);
    Vector2 _position = Vector2();
    Vector2 _size = Vector2();
    String _window_title = "";
    String _window_icon_path = "";
    float _window_opacity = 1.0f;
    
    // 内部状态
    bool _is_active = false;
    bool _is_initialized = false;

protected:
    static void _bind_methods();

public:
    UniWindowController();
    ~UniWindowController();
    
    // 生命周期
    void _ready() override;
    void _process(double delta) override;
    
    // 窗口控制方法
    bool attach_window();
    void detach_window();
    
    // 基础属性访问器
    void set_transparent(bool transparent);
    bool get_transparent() const;
    
    void set_borderless(bool borderless);
    bool get_borderless() const;
    
    void set_topmost(bool topmost);
    bool get_topmost() const;
    
    void set_bottommost(bool bottommost);
    bool get_bottommost() const;
    
    void set_clickthrough(bool clickthrough);
    bool get_clickthrough() const;
    
    void set_alpha_value(float alpha);
    float get_alpha_value() const;
    
    void set_position(Vector2 position);
    Vector2 get_position() const;
    
    void set_size(Vector2 size);
    Vector2 get_size() const;
    
    void set_allow_drop_files(bool allow);
    bool get_allow_drop_files() const;
    
    // 新增的Unity兼容API
    void set_zoomed(bool zoomed);
    bool get_zoomed() const;
    
    void set_should_fit_monitor(bool should_fit);
    bool get_should_fit_monitor() const;
    
    void set_monitor_to_fit(int monitor_index);
    int get_monitor_to_fit() const;
    
    // 修复Bug2：添加fit_to_monitor方法声明
    void fit_to_monitor(int monitor_index);
    
    void set_transparent_type(int type);
    int get_transparent_type() const;
    
    void set_key_color(const Color& color);
    Color get_key_color() const;
    
    void set_hit_test_type(int type);
    int get_hit_test_type() const;
    
    void set_opacity_threshold(float threshold);
    float get_opacity_threshold() const;
    
    void set_hit_test_enabled(bool enabled);
    bool get_hit_test_enabled() const;
    
    // 窗口控制功能
    void set_window_title(const String& title);
    String get_window_title() const;
    
    void minimize_window();
    void maximize_window();
    void restore_window();
    
    void set_window_icon(const String& icon_path);
    
    // 高级功能
    void enable_always_on_top(bool enable);
    void set_window_opacity(float opacity);
    float get_window_opacity() const;
    
    // 状态查询
    bool is_active() const;
    bool is_maximized() const;
    bool is_minimized() const;
    
    // 多显示器支持
    int get_monitor_count() const;
    Vector2 get_monitor_size(int monitor_index) const;
    Vector2 get_monitor_position(int monitor_index) const;
    int get_current_monitor() const;
    
    // 鼠标和键盘 - 静态方法
    static Vector2 get_cursor_position();
    static void set_cursor_position(Vector2 position);
    static int get_mouse_buttons();
    static int get_modifier_keys();

private:
    // Native 库接口
    void _initialize_native();
    void _cleanup_native();
    void _update_from_native();
    void _process_messages();
    
    // 回调处理
    static void _on_files_dropped(const wchar_t* file_paths_w);  // 宽字符版本，转换为UTF-8
    static void _on_window_focus_changed(bool focused);
    static void _on_window_moved(float x, float y);
    static void _on_window_resized(float width, float height);
    static void _on_monitor_changed(int monitor_index);
};

#endif // UNIWINC_CONTROLLER_H