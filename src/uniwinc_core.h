#ifndef UNIWINC_CORE_H
#define UNIWINC_CORE_H

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

// 回调函数类型定义
typedef void (*DropFilesCallback)(const wchar_t* file_paths);  // 更新为宽字符版本
typedef void (*FocusChangedCallback)(bool focused);
typedef void (*WindowMovedCallback)(float x, float y);
typedef void (*WindowResizedCallback)(float width, float height);
typedef void (*MonitorChangedCallback)(int monitor_index);

class UniWinCore {
public:
    // 初始化和清理
    static bool initialize();
    static void cleanup();
    
    // 窗口控制
    static bool attach_window();
    static void detach_window();
    
    // 窗口状态查询
    static bool is_active();
    static bool is_transparent();
    static bool is_borderless();
    static bool is_topmost();
    static bool is_bottommost();
    static bool is_maximized();
    static bool is_minimized();
    static bool is_zoomed();
    
    // 基础窗口属性设置
    static void set_transparent(bool transparent);
    static void set_borderless(bool borderless);
    static void set_topmost(bool topmost);
    static void set_bottommost(bool bottommost);
    static void set_alpha_value(float alpha);
    static void set_clickthrough(bool clickthrough);
    static void set_zoomed(bool zoomed);
    
    // Unity兼容的扩展属性
    static void set_transparent_type(int type);
    static int get_transparent_type();
    
    static void set_key_color(const Color& color);
    static Color get_key_color();
    
    static void set_hit_test_type(int type);
    static int get_hit_test_type();
    
    static void set_opacity_threshold(float threshold);
    static float get_opacity_threshold();
    
    static void set_hit_test_enabled(bool enabled);
    static bool get_hit_test_enabled();
    
    // 监视器适配功能
    static void set_should_fit_monitor(bool should_fit);
    static bool get_should_fit_monitor();
    
    static void set_monitor_to_fit(int monitor_index);
    static int get_monitor_to_fit();
    
    static void fit_to_monitor(int monitor_index);
    
    // 窗口位置和大小
    static void set_position(float x, float y);
    static void get_position(float* x, float* y);
    static void set_size(float width, float height);
    static void get_size(float* width, float* height);
    static void get_client_size(float* width, float* height);
    
    // 多显示器支持
    static int get_monitor_count();
    static void get_monitor_size(int monitor_index, float* width, float* height);
    static void get_monitor_position(int monitor_index, float* x, float* y);
    static void get_monitor_rectangle(int monitor_index, float* x, float* y, float* width, float* height);
    static int get_current_monitor();
    
    // 文件拖拽
    static void set_allow_drop_files(bool allow);
    
    // 窗口控制
    static void minimize_window();
    static void maximize_window();
    static void restore_window();
    
    // 鼠标和键盘
    static void get_cursor_position(float* x, float* y);
    static void set_cursor_position(float x, float y);
    static int get_mouse_buttons();
    static int get_modifier_keys();
    
    // 回调注册
    static void register_drop_files_callback(DropFilesCallback callback);
    static void register_focus_changed_callback(FocusChangedCallback callback);
    static void register_window_moved_callback(WindowMovedCallback callback);
    static void register_window_resized_callback(WindowResizedCallback callback);
    static void register_monitor_changed_callback(MonitorChangedCallback callback);
    
    // 文件对话框
    static String open_file_panel(const String& title, const String& filters, const String& initial_path);
    static String save_file_panel(const String& title, const String& filters, const String& initial_path);
    
    // 增强的文件对话框方法 (对应Unity的完整PanelSettings支持)
    static String open_file_panel_with_settings(const String& title, const String& filters, 
                                               const String& initial_directory, const String& initial_file, int flags);
    static String save_file_panel_with_settings(const String& title, const String& filters, 
                                               const String& initial_directory, const String& initial_file, int flags);

private:
    static bool _is_initialized;
    static void* _library_handle;
    
    // 内部状态缓存
    static bool _should_fit_monitor;
    static int _monitor_to_fit;
    static int _transparent_type;
    static int _hit_test_type;
    static float _opacity_threshold;
    static bool _hit_test_enabled;
    static Color _key_color;
    
    // 函数指针声明
    static bool load_native_library();
    static void unload_native_library();
    static bool load_function_pointers();
    
    // Native 函数指针（将在实现文件中定义）
};

#endif // UNIWINC_CORE_H