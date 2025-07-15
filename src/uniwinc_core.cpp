#include "uniwinc_core.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>

#ifdef _WIN32
#include <windows.h>
#define LIBRARY_EXTENSION ".dll"
#define LOAD_LIBRARY(path) LoadLibraryA(path)
#define GET_PROC_ADDRESS(handle, name) GetProcAddress((HMODULE)handle, name)
#define FREE_LIBRARY(handle) FreeLibrary((HMODULE)handle)
#elif defined(__APPLE__)
#include <dlfcn.h>
#define LIBRARY_EXTENSION ".bundle/Contents/MacOS/LibUniWinC"
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
#define FREE_LIBRARY(handle) dlclose(handle)
#else
#include <dlfcn.h>
#define LIBRARY_EXTENSION ".so"
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
#define FREE_LIBRARY(handle) dlclose(handle)
#endif

using namespace godot;

// 静态成员初始化
bool UniWinCore::_is_initialized = false;
void *UniWinCore::_library_handle = nullptr;

// 静态成员变量初始化（Unity兼容状态缓存）
bool UniWinCore::_should_fit_monitor = false;
int UniWinCore::_monitor_to_fit = 0;
int UniWinCore::_transparent_type = 1; // Alpha
int UniWinCore::_hit_test_type = 1;    // Opacity
float UniWinCore::_opacity_threshold = 0.1f;
bool UniWinCore::_hit_test_enabled = true;
Color UniWinCore::_key_color = Color(1.0f, 0.0f, 1.0f, 0.0f);

// Native 函数指针定义
typedef bool (*IsActiveFunc)();
typedef bool (*AttachMyWindowFunc)();
typedef bool (*AttachMyActiveWindowFunc)();
typedef bool (*AttachMyOwnerWindowFunc)();
typedef void (*DetachWindowFunc)();
typedef bool (*IsTransparentFunc)();
typedef bool (*IsBorderlessFunc)();
typedef bool (*IsTopmostFunc)();
typedef bool (*IsBottommostFunc)();
typedef bool (*IsMaximizedFunc)();
typedef bool (*IsMinimizedFunc)();
typedef bool (*IsZoomedFunc)();
typedef void (*SetTransparentFunc)(bool);
typedef void (*SetBorderlessFunc)(bool);
typedef void (*SetTopmostFunc)(bool);
typedef void (*SetBottommostFunc)(bool);
typedef void (*SetAlphaValueFunc)(float);
typedef void (*SetClickThroughFunc)(bool);
typedef void (*SetZoomedFunc)(bool);
typedef void (*SetPositionFunc)(float, float);
typedef void (*GetPositionFunc)(float *, float *);
typedef void (*SetSizeFunc)(float, float);
typedef void (*GetSizeFunc)(float *, float *);
typedef void (*GetClientSizeFunc)(float *, float *);
typedef int (*GetMonitorCountFunc)();
typedef void (*GetMonitorRectangleFunc)(int, float *, float *, float *, float *);
typedef int (*GetCurrentMonitorFunc)();
typedef void (*SetAllowDropFunc)(bool);
typedef void (*GetCursorPositionFunc)(float *, float *);
typedef void (*SetCursorPositionFunc)(float, float);
typedef int (*GetMouseButtonsFunc)();
typedef int (*GetModifierKeysFunc)();
typedef void (*MinimizeWindowFunc)();
typedef void (*MaximizeWindowFunc)();
typedef void (*RestoreWindowFunc)();
typedef void (*FitToMonitorFunc)(int);

// Unity兼容的扩展函数指针
typedef void (*SetTransparentTypeFunc)(int);
typedef int (*GetTransparentTypeFunc)();
typedef void (*SetKeyColorFunc)(float, float, float, float); // r, g, b, a
typedef void (*GetKeyColorFunc)(float *, float *, float *, float *);
typedef void (*SetHitTestTypeFunc)(int);
typedef int (*GetHitTestTypeFunc)();
typedef void (*SetOpacityThresholdFunc)(float);
typedef float (*GetOpacityThresholdFunc)();
typedef void (*SetHitTestEnabledFunc)(bool);
typedef bool (*GetHitTestEnabledFunc)();

// 回调注册函数指针
typedef void (*WStringCallbackFunc)(const wchar_t *);
typedef void (*BoolCallbackFunc)(bool);
typedef void (*IntCallbackFunc)(int);
typedef void (*FloatFloatCallbackFunc)(float, float);
typedef bool (*RegisterDropFilesCallbackFunc)(WStringCallbackFunc);
typedef bool (*RegisterWindowStyleChangedCallbackFunc)(BoolCallbackFunc);
typedef bool (*RegisterWindowMovedCallbackFunc)(FloatFloatCallbackFunc);
typedef bool (*RegisterWindowResizedCallbackFunc)(FloatFloatCallbackFunc);
typedef bool (*RegisterMonitorChangedCallbackFunc)(IntCallbackFunc);

// 文件对话框函数指针
typedef bool (*OpenFilePanelFunc)(void *, char *, int);
typedef bool (*SaveFilePanelFunc)(void *, char *, int);

// 函数指针实例
static IsActiveFunc native_is_active = nullptr;
static AttachMyWindowFunc native_attach_window = nullptr;
static AttachMyActiveWindowFunc native_attach_active_window = nullptr;
static AttachMyOwnerWindowFunc native_attach_owner_window = nullptr;
static DetachWindowFunc native_detach_window = nullptr;
static IsTransparentFunc native_is_transparent = nullptr;
static IsBorderlessFunc native_is_borderless = nullptr;
static IsTopmostFunc native_is_topmost = nullptr;
static IsBottommostFunc native_is_bottommost = nullptr;
static IsMaximizedFunc native_is_maximized = nullptr;
static IsMinimizedFunc native_is_minimized = nullptr;
static IsZoomedFunc native_is_zoomed = nullptr;
static SetTransparentFunc native_set_transparent = nullptr;
static SetBorderlessFunc native_set_borderless = nullptr;
static SetTopmostFunc native_set_topmost = nullptr;
static SetBottommostFunc native_set_bottommost = nullptr;
static SetAlphaValueFunc native_set_alpha_value = nullptr;
static SetClickThroughFunc native_set_clickthrough = nullptr;
static SetZoomedFunc native_set_zoomed = nullptr;
static SetPositionFunc native_set_position = nullptr;
static GetPositionFunc native_get_position = nullptr;
static SetSizeFunc native_set_size = nullptr;
static GetSizeFunc native_get_size = nullptr;
static GetClientSizeFunc native_get_client_size = nullptr;
static GetMonitorCountFunc native_get_monitor_count = nullptr;
static GetMonitorRectangleFunc native_get_monitor_rectangle = nullptr;
static GetCurrentMonitorFunc native_get_current_monitor = nullptr;
static SetAllowDropFunc native_set_allow_drop = nullptr;
static GetCursorPositionFunc native_get_cursor_position = nullptr;
static SetCursorPositionFunc native_set_cursor_position = nullptr;
static GetMouseButtonsFunc native_get_mouse_buttons = nullptr;
static GetModifierKeysFunc native_get_modifier_keys = nullptr;
static MinimizeWindowFunc native_minimize_window = nullptr;
static MaximizeWindowFunc native_maximize_window = nullptr;
static RestoreWindowFunc native_restore_window = nullptr;
static FitToMonitorFunc native_fit_to_monitor = nullptr;

// Unity兼容的扩展函数指针实例
static SetTransparentTypeFunc native_set_transparent_type = nullptr;
static GetTransparentTypeFunc native_get_transparent_type = nullptr;
static SetKeyColorFunc native_set_key_color = nullptr;
static GetKeyColorFunc native_get_key_color = nullptr;
static SetHitTestTypeFunc native_set_hit_test_type = nullptr;
static GetHitTestTypeFunc native_get_hit_test_type = nullptr;
static SetOpacityThresholdFunc native_set_opacity_threshold = nullptr;
static GetOpacityThresholdFunc native_get_opacity_threshold = nullptr;
static SetHitTestEnabledFunc native_set_hit_test_enabled = nullptr;
static GetHitTestEnabledFunc native_get_hit_test_enabled = nullptr;

// 回调函数指针实例
static RegisterDropFilesCallbackFunc native_register_drop_files_callback = nullptr;
static RegisterWindowStyleChangedCallbackFunc native_register_focus_changed_callback = nullptr;
static RegisterWindowMovedCallbackFunc native_register_window_moved_callback = nullptr;
static RegisterWindowResizedCallbackFunc native_register_window_resized_callback = nullptr;
static RegisterMonitorChangedCallbackFunc native_register_monitor_changed_callback = nullptr;

// 文件对话框函数指针实例
static OpenFilePanelFunc native_open_file_panel = nullptr;
static SaveFilePanelFunc native_save_file_panel = nullptr;

bool UniWinCore::initialize()
{
    if (_is_initialized)
    {
        return true;
    }

    if (!load_native_library())
    {
        UtilityFunctions::print("Failed to load native library");
        return false;
    }

    if (!load_function_pointers())
    {
        UtilityFunctions::print("Failed to load function pointers");
        unload_native_library();
        return false;
    }

    _is_initialized = true;
    UtilityFunctions::print("UniWinCore initialized successfully");
    return true;
}

void UniWinCore::cleanup()
{
    if (_is_initialized)
    {
        unload_native_library();
        _is_initialized = false;
        UtilityFunctions::print("UniWinCore cleaned up");
    }
}

bool UniWinCore::load_native_library()
{
    String library_path;

#ifdef _WIN32
    library_path = "res://addons/uniwinc/bin/windows/LibUniWinC" LIBRARY_EXTENSION;
#elif defined(__APPLE__)
    library_path = "res://addons/uniwinc/bin/macos/LibUniWinC" LIBRARY_EXTENSION;
#else
    library_path = "res://addons/uniwinc/bin/linux/LibUniWinC" LIBRARY_EXTENSION;
#endif

    // 转换为绝对路径
    String abs_path = ProjectSettings::get_singleton()->globalize_path(library_path);

    UtilityFunctions::print("Attempting to load native library: " + abs_path);

    // 检查文件是否存在
    if (!FileAccess::file_exists(abs_path))
    {
        UtilityFunctions::print("Native library file does not exist: " + abs_path);

        // 尝试备用路径
        String fallback_path = "./LibUniWinC" LIBRARY_EXTENSION;
        abs_path = ProjectSettings::get_singleton()->globalize_path(fallback_path);

        if (!FileAccess::file_exists(abs_path))
        {
            UtilityFunctions::print("Fallback path also does not exist: " + abs_path);
            return false;
        }

        UtilityFunctions::print("Using fallback path: " + abs_path);
    }

    _library_handle = LOAD_LIBRARY(abs_path.utf8().get_data());

    if (_library_handle)
    {
        UtilityFunctions::print("Successfully loaded native library");
        return true;
    }
    else
    {
        UtilityFunctions::print("Failed to load native library");

#ifdef _WIN32
        DWORD error = GetLastError();
        UtilityFunctions::print("Windows error code: " + String::num_int64(error));
#else
        const char *error = dlerror();
        if (error)
        {
            UtilityFunctions::print("dlerror: " + String(error));
        }
#endif
        return false;
    }
}

void UniWinCore::unload_native_library()
{
    if (_library_handle)
    {
        FREE_LIBRARY(_library_handle);
        _library_handle = nullptr;
    }
}

bool UniWinCore::load_function_pointers()
{
    if (!_library_handle)
    {
        return false;
    }

    // 加载基础函数指针
    native_is_active = (IsActiveFunc)GET_PROC_ADDRESS(_library_handle, "IsActive");
    native_attach_window = (AttachMyWindowFunc)GET_PROC_ADDRESS(_library_handle, "AttachMyWindow");
    native_attach_active_window = (AttachMyActiveWindowFunc)GET_PROC_ADDRESS(_library_handle, "AttachMyActiveWindow");
    native_attach_owner_window = (AttachMyOwnerWindowFunc)GET_PROC_ADDRESS(_library_handle, "AttachMyOwnerWindow");
    native_detach_window = (DetachWindowFunc)GET_PROC_ADDRESS(_library_handle, "DetachWindow");

    // 状态查询函数
    native_is_transparent = (IsTransparentFunc)GET_PROC_ADDRESS(_library_handle, "IsTransparent");
    native_is_borderless = (IsBorderlessFunc)GET_PROC_ADDRESS(_library_handle, "IsBorderless");
    native_is_topmost = (IsTopmostFunc)GET_PROC_ADDRESS(_library_handle, "IsTopmost");
    native_is_bottommost = (IsBottommostFunc)GET_PROC_ADDRESS(_library_handle, "IsBottommost");
    native_is_maximized = (IsMaximizedFunc)GET_PROC_ADDRESS(_library_handle, "IsMaximized");
    native_is_minimized = (IsMinimizedFunc)GET_PROC_ADDRESS(_library_handle, "IsMinimized");
    native_is_zoomed = (IsZoomedFunc)GET_PROC_ADDRESS(_library_handle, "IsMaximized"); // Unity兼容

    // 基础设置函数
    native_set_transparent = (SetTransparentFunc)GET_PROC_ADDRESS(_library_handle, "SetTransparent");
    native_set_borderless = (SetBorderlessFunc)GET_PROC_ADDRESS(_library_handle, "SetBorderless");
    native_set_topmost = (SetTopmostFunc)GET_PROC_ADDRESS(_library_handle, "SetTopmost");
    native_set_bottommost = (SetBottommostFunc)GET_PROC_ADDRESS(_library_handle, "SetBottommost");
    native_set_alpha_value = (SetAlphaValueFunc)GET_PROC_ADDRESS(_library_handle, "SetAlphaValue");
    native_set_clickthrough = (SetClickThroughFunc)GET_PROC_ADDRESS(_library_handle, "SetClickThrough");
    native_set_zoomed = (SetZoomedFunc)GET_PROC_ADDRESS(_library_handle, "SetMaximized"); // Unity兼容

    // 位置和大小函数
    native_set_position = (SetPositionFunc)GET_PROC_ADDRESS(_library_handle, "SetPosition");
    native_get_position = (GetPositionFunc)GET_PROC_ADDRESS(_library_handle, "GetPosition");
    native_set_size = (SetSizeFunc)GET_PROC_ADDRESS(_library_handle, "SetSize");
    native_get_size = (GetSizeFunc)GET_PROC_ADDRESS(_library_handle, "GetSize");
    native_get_client_size = (GetClientSizeFunc)GET_PROC_ADDRESS(_library_handle, "GetClientSize");

    // 监视器相关函数
    native_get_monitor_count = (GetMonitorCountFunc)GET_PROC_ADDRESS(_library_handle, "GetMonitorCount");
    native_get_monitor_rectangle = (GetMonitorRectangleFunc)GET_PROC_ADDRESS(_library_handle, "GetMonitorRectangle");
    native_get_current_monitor = (GetCurrentMonitorFunc)GET_PROC_ADDRESS(_library_handle, "GetCurrentMonitor");
    native_fit_to_monitor = (FitToMonitorFunc)GET_PROC_ADDRESS(_library_handle, "FitToMonitor");

    // 文件拖拽和鼠标键盘
    native_set_allow_drop = (SetAllowDropFunc)GET_PROC_ADDRESS(_library_handle, "SetAllowDrop");
    native_get_cursor_position = (GetCursorPositionFunc)GET_PROC_ADDRESS(_library_handle, "GetCursorPosition");
    native_set_cursor_position = (SetCursorPositionFunc)GET_PROC_ADDRESS(_library_handle, "SetCursorPosition");
    native_get_mouse_buttons = (GetMouseButtonsFunc)GET_PROC_ADDRESS(_library_handle, "GetMouseButtons");
    native_get_modifier_keys = (GetModifierKeysFunc)GET_PROC_ADDRESS(_library_handle, "GetModifierKeys");

    // 窗口控制函数
    native_minimize_window = (MinimizeWindowFunc)GET_PROC_ADDRESS(_library_handle, "MinimizeWindow");
    native_maximize_window = (MaximizeWindowFunc)GET_PROC_ADDRESS(_library_handle, "MaximizeWindow");
    native_restore_window = (RestoreWindowFunc)GET_PROC_ADDRESS(_library_handle, "RestoreWindow");

    // Unity兼容的扩展函数（可能不存在于native库中，需要在C++层模拟）
    native_set_transparent_type = (SetTransparentTypeFunc)GET_PROC_ADDRESS(_library_handle, "SetTransparentType");
    native_get_transparent_type = (GetTransparentTypeFunc)GET_PROC_ADDRESS(_library_handle, "GetTransparentType");
    native_set_key_color = (SetKeyColorFunc)GET_PROC_ADDRESS(_library_handle, "SetKeyColor");
    native_get_key_color = (GetKeyColorFunc)GET_PROC_ADDRESS(_library_handle, "GetKeyColor");
    native_set_hit_test_type = (SetHitTestTypeFunc)GET_PROC_ADDRESS(_library_handle, "SetHitTestType");
    native_get_hit_test_type = (GetHitTestTypeFunc)GET_PROC_ADDRESS(_library_handle, "GetHitTestType");
    native_set_opacity_threshold = (SetOpacityThresholdFunc)GET_PROC_ADDRESS(_library_handle, "SetOpacityThreshold");
    native_get_opacity_threshold = (GetOpacityThresholdFunc)GET_PROC_ADDRESS(_library_handle, "GetOpacityThreshold");
    native_set_hit_test_enabled = (SetHitTestEnabledFunc)GET_PROC_ADDRESS(_library_handle, "SetHitTestEnabled");
    native_get_hit_test_enabled = (GetHitTestEnabledFunc)GET_PROC_ADDRESS(_library_handle, "GetHitTestEnabled");

    // 回调注册函数
    native_register_drop_files_callback = (RegisterDropFilesCallbackFunc)GET_PROC_ADDRESS(_library_handle, "RegisterDropFilesCallback");
    native_register_focus_changed_callback = (RegisterWindowStyleChangedCallbackFunc)GET_PROC_ADDRESS(_library_handle, "RegisterWindowStyleChangedCallback");
    native_register_window_moved_callback = (RegisterWindowMovedCallbackFunc)GET_PROC_ADDRESS(_library_handle, "RegisterWindowMovedCallback");
    native_register_window_resized_callback = (RegisterWindowResizedCallbackFunc)GET_PROC_ADDRESS(_library_handle, "RegisterWindowResizedCallback");
    native_register_monitor_changed_callback = (RegisterMonitorChangedCallbackFunc)GET_PROC_ADDRESS(_library_handle, "RegisterMonitorChangedCallback");

    // 文件对话框函数
    native_open_file_panel = (OpenFilePanelFunc)GET_PROC_ADDRESS(_library_handle, "OpenFilePanel");
    native_save_file_panel = (SaveFilePanelFunc)GET_PROC_ADDRESS(_library_handle, "SaveFilePanel");

    // 检查关键函数是否加载成功
    bool core_functions_loaded = native_is_active && native_attach_window && native_detach_window;

    // 检查drop files相关函数是否加载成功
    bool drop_files_loaded = native_set_allow_drop && native_register_drop_files_callback;

    // 检查监视器相关函数是否加载成功
    bool monitor_functions_loaded = native_get_monitor_count && native_get_monitor_rectangle;
    bool fit_monitor_available = native_fit_to_monitor != nullptr;

    UtilityFunctions::print("Function loading status:");
    UtilityFunctions::print("  Core functions: " + String(core_functions_loaded ? "OK" : "FAILED"));
    UtilityFunctions::print("  Drop files functions: " + String(drop_files_loaded ? "OK" : "FAILED"));
    UtilityFunctions::print("    - SetAllowDrop: " + String(native_set_allow_drop ? "OK" : "MISSING"));
    UtilityFunctions::print("    - RegisterDropFilesCallback (wide): " + String(native_register_drop_files_callback ? "OK" : "MISSING"));
    UtilityFunctions::print("  Monitor functions: " + String(monitor_functions_loaded ? "OK" : "FAILED"));
    UtilityFunctions::print("    - GetMonitorCount: " + String(native_get_monitor_count ? "OK" : "MISSING"));
    UtilityFunctions::print("    - GetMonitorRectangle: " + String(native_get_monitor_rectangle ? "OK" : "MISSING"));
    UtilityFunctions::print("    - FitToMonitor: " + String(fit_monitor_available ? "OK" : "MISSING"));
    UtilityFunctions::print("  Window control functions:");
    UtilityFunctions::print("    - SetMaximized: " + String(native_set_zoomed ? "OK" : "MISSING"));
    UtilityFunctions::print("    - IsMaximized: " + String(native_is_zoomed ? "OK" : "MISSING"));

    return core_functions_loaded;
}

// 实现所有接口函数
bool UniWinCore::attach_window()
{
    if (!native_attach_window)
    {
        UtilityFunctions::print("Native attach_window function not available");
        return false;
    }

    bool result = native_attach_window();
    if (!result)
    {
        UtilityFunctions::print("AttachMyWindow failed, trying alternative methods...");

        // 尝试活动窗口附加
        if (native_attach_active_window)
        {
            result = native_attach_active_window();
            if (result)
            {
                UtilityFunctions::print("Successfully attached to active window");
                return true;
            }
        }

        // 尝试所有者窗口附加
        if (native_attach_owner_window)
        {
            result = native_attach_owner_window();
            if (result)
            {
                UtilityFunctions::print("Successfully attached to owner window");
                return true;
            }
        }

        UtilityFunctions::print("All attach methods failed");
    }
    else
    {
        UtilityFunctions::print("Successfully attached to window using default method");
    }

    return result;
}

void UniWinCore::detach_window()
{
    if (native_detach_window)
    {
        native_detach_window();
    }
}

bool UniWinCore::is_active()
{
    return native_is_active ? native_is_active() : false;
}

bool UniWinCore::is_transparent()
{
    return native_is_transparent ? native_is_transparent() : false;
}

bool UniWinCore::is_borderless()
{
    return native_is_borderless ? native_is_borderless() : false;
}

bool UniWinCore::is_topmost()
{
    return native_is_topmost ? native_is_topmost() : false;
}

bool UniWinCore::is_maximized()
{
    bool result = native_is_maximized ? native_is_maximized() : false;
    UtilityFunctions::print("UniWinCore::is_maximized returning: " + String(result ? "true" : "false"));
    return result;
}

bool UniWinCore::is_minimized()
{
    return native_is_minimized ? native_is_minimized() : false;
}

void UniWinCore::set_transparent(bool transparent)
{
    if (native_set_transparent)
    {
        native_set_transparent(transparent);
    }
}

void UniWinCore::set_borderless(bool borderless)
{
    if (native_set_borderless)
    {
        native_set_borderless(borderless);
    }
}

void UniWinCore::set_topmost(bool topmost)
{
    if (native_set_topmost)
    {
        native_set_topmost(topmost);
    }
}

void UniWinCore::set_bottommost(bool bottommost)
{
    if (native_set_bottommost)
    {
        native_set_bottommost(bottommost);
    }
}

void UniWinCore::set_alpha_value(float alpha)
{
    if (native_set_alpha_value)
    {
        native_set_alpha_value(alpha);
    }
}

void UniWinCore::set_clickthrough(bool clickthrough)
{
    if (native_set_clickthrough)
    {
        native_set_clickthrough(clickthrough);
    }
}

void UniWinCore::set_position(float x, float y)
{
    if (native_set_position)
    {
        native_set_position(x, y);
    }
}

void UniWinCore::get_position(float *x, float *y)
{
    if (native_get_position && x && y)
    {
        native_get_position(x, y);
    }
}

void UniWinCore::set_size(float width, float height)
{
    if (native_set_size)
    {
        native_set_size(width, height);
    }
}

void UniWinCore::get_size(float *width, float *height)
{
    if (native_get_size && width && height)
    {
        native_get_size(width, height);
    }
}

int UniWinCore::get_monitor_count()
{
    return native_get_monitor_count ? native_get_monitor_count() : 1;
}

void UniWinCore::get_monitor_size(int monitor_index, float *width, float *height)
{
    if (native_get_monitor_rectangle && width && height)
    {
        float x, y;
        native_get_monitor_rectangle(monitor_index, &x, &y, width, height);
    }
}

int UniWinCore::get_current_monitor()
{
    return native_get_current_monitor ? native_get_current_monitor() : 0;
}

void UniWinCore::set_allow_drop_files(bool allow)
{
    UtilityFunctions::print("Setting allow drop files to: " + String(allow ? "true" : "false"));
    if (native_set_allow_drop)
    {
        native_set_allow_drop(allow);
        UtilityFunctions::print("Allow drop files setting applied successfully");
    }
    else
    {
        UtilityFunctions::print("ERROR: native_set_allow_drop function pointer is null");
    }
}

void UniWinCore::get_cursor_position(float *x, float *y)
{
    if (native_get_cursor_position && x && y)
    {
        native_get_cursor_position(x, y);
    }
}

void UniWinCore::set_cursor_position(float x, float y)
{
    if (native_set_cursor_position)
    {
        native_set_cursor_position(x, y);
    }
}

int UniWinCore::get_mouse_buttons()
{
    return native_get_mouse_buttons ? native_get_mouse_buttons() : 0;
}

int UniWinCore::get_modifier_keys()
{
    return native_get_modifier_keys ? native_get_modifier_keys() : 0;
}

void UniWinCore::register_drop_files_callback(DropFilesCallback callback)
{
    UtilityFunctions::print("Registering drop files callback (wide character)...");
    if (native_register_drop_files_callback)
    {
        bool success = native_register_drop_files_callback((WStringCallbackFunc)callback);
        if (success)
        {
            UtilityFunctions::print("Drop files callback registered successfully");
        }
        else
        {
            UtilityFunctions::print("ERROR: Failed to register drop files callback");
        }
    }
    else
    {
        UtilityFunctions::print("ERROR: native_register_drop_files_callback function pointer is null");
    }
}

void UniWinCore::register_focus_changed_callback(FocusChangedCallback callback)
{
    if (native_register_focus_changed_callback)
    {
        native_register_focus_changed_callback((BoolCallbackFunc)callback);
    }
}

void UniWinCore::register_window_moved_callback(WindowMovedCallback callback)
{
    if (native_register_window_moved_callback)
    {
        native_register_window_moved_callback((FloatFloatCallbackFunc)callback);
    }
}

void UniWinCore::register_window_resized_callback(WindowResizedCallback callback)
{
    if (native_register_window_resized_callback)
    {
        native_register_window_resized_callback((FloatFloatCallbackFunc)callback);
    }
}

void UniWinCore::register_monitor_changed_callback(MonitorChangedCallback callback)
{
    if (native_register_monitor_changed_callback)
    {
        native_register_monitor_changed_callback((IntCallbackFunc)callback);
    }
}

String UniWinCore::open_file_panel(const String &title, const String &filters, const String &initial_path)
{
    return open_file_panel_with_settings(title, filters, initial_path, "", 0);
}

String UniWinCore::save_file_panel(const String &title, const String &filters, const String &initial_path)
{
    return save_file_panel_with_settings(title, filters, initial_path, "", 0);
}

String UniWinCore::open_file_panel_with_settings(const String &title, const String &filters,
                                                 const String &initial_directory, const String &initial_file, int flags)
{
    if (!native_open_file_panel)
    {
        UtilityFunctions::print("OpenFilePanel function not available in native library");
        return String();
    }

    // 创建缓冲区来接收结果
    const int buffer_size = 4096;
    char buffer[buffer_size];
    memset(buffer, 0, buffer_size);

    // 创建类似Unity PanelSettings的结构
    // 这里简化为直接传递参数，实际使用需要根据native库的API来调整
    void *settings = nullptr; // TODO: 构建实际的设置结构

    bool success = native_open_file_panel(settings, buffer, buffer_size);

    if (success && buffer[0] != '\0')
    {
        return String::utf8(buffer);
    }

    return String();
}

String UniWinCore::save_file_panel_with_settings(const String &title, const String &filters,
                                                 const String &initial_directory, const String &initial_file, int flags)
{
    if (!native_save_file_panel)
    {
        UtilityFunctions::print("SaveFilePanel function not available in native library");
        return String();
    }

    // 创建缓冲区来接收结果
    const int buffer_size = 4096;
    char buffer[buffer_size];
    memset(buffer, 0, buffer_size);

    // 创建类似Unity PanelSettings的结构
    void *settings = nullptr; // TODO: 构建实际的设置结构

    bool success = native_save_file_panel(settings, buffer, buffer_size);

    if (success && buffer[0] != '\0')
    {
        return String::utf8(buffer);
    }

    return String();
}

// 新增的Unity兼容方法实现
bool UniWinCore::is_bottommost()
{
    return native_is_bottommost ? native_is_bottommost() : false;
}

bool UniWinCore::is_zoomed()
{
    return native_is_zoomed ? native_is_zoomed() : false;
}

void UniWinCore::set_zoomed(bool zoomed)
{
    UtilityFunctions::print("UniWinCore::set_zoomed called with: " + String(zoomed ? "true" : "false"));
    if (native_set_zoomed)
    {
        native_set_zoomed(zoomed);
        UtilityFunctions::print("Native SetMaximized called successfully");
    }
    else
    {
        UtilityFunctions::print("ERROR: native_set_zoomed function pointer is null");
    }
}

void UniWinCore::set_transparent_type(int type)
{
    _transparent_type = type;
    if (native_set_transparent_type)
    {
        native_set_transparent_type(type);
    }
}

int UniWinCore::get_transparent_type()
{
    if (native_get_transparent_type)
    {
        return native_get_transparent_type();
    }
    return _transparent_type;
}

void UniWinCore::set_key_color(const Color &color)
{
    _key_color = color;
    if (native_set_key_color)
    {
        native_set_key_color(color.r, color.g, color.b, color.a);
    }
}

Color UniWinCore::get_key_color()
{
    if (native_get_key_color)
    {
        float r, g, b, a;
        native_get_key_color(&r, &g, &b, &a);
        return Color(r, g, b, a);
    }
    return _key_color;
}

void UniWinCore::set_hit_test_type(int type)
{
    _hit_test_type = type;
    if (native_set_hit_test_type)
    {
        native_set_hit_test_type(type);
    }
}

int UniWinCore::get_hit_test_type()
{
    if (native_get_hit_test_type)
    {
        return native_get_hit_test_type();
    }
    return _hit_test_type;
}

void UniWinCore::set_opacity_threshold(float threshold)
{
    _opacity_threshold = threshold;
    if (native_set_opacity_threshold)
    {
        native_set_opacity_threshold(threshold);
    }
}

float UniWinCore::get_opacity_threshold()
{
    if (native_get_opacity_threshold)
    {
        return native_get_opacity_threshold();
    }
    return _opacity_threshold;
}

void UniWinCore::set_hit_test_enabled(bool enabled)
{
    _hit_test_enabled = enabled;
    if (native_set_hit_test_enabled)
    {
        native_set_hit_test_enabled(enabled);
    }
}

bool UniWinCore::get_hit_test_enabled()
{
    if (native_get_hit_test_enabled)
    {
        return native_get_hit_test_enabled();
    }
    return _hit_test_enabled;
}

void UniWinCore::set_should_fit_monitor(bool should_fit)
{
    _should_fit_monitor = should_fit;
}

bool UniWinCore::get_should_fit_monitor()
{
    return _should_fit_monitor;
}

void UniWinCore::set_monitor_to_fit(int monitor_index)
{
    _monitor_to_fit = monitor_index;
}

int UniWinCore::get_monitor_to_fit()
{
    return _monitor_to_fit;
}

void UniWinCore::fit_to_monitor(int monitor_index)
{
    UtilityFunctions::print("UniWinCore::fit_to_monitor called with monitor: " + String::num_int64(monitor_index));
    if (native_fit_to_monitor)
    {
        UtilityFunctions::print("Using native FitToMonitor function");
        native_fit_to_monitor(monitor_index);
        UtilityFunctions::print("Native FitToMonitor called successfully");
    }
    else
    {
        UtilityFunctions::print("Native FitToMonitor not available - this is expected, as Unity version implements it in C# layer");
        UtilityFunctions::print("FitToMonitor logic is implemented in UniWindowController::fit_to_monitor() instead");
    }
}

void UniWinCore::get_client_size(float *width, float *height)
{
    if (native_get_client_size && width && height)
    {
        native_get_client_size(width, height);
    }
}

void UniWinCore::get_monitor_position(int monitor_index, float *x, float *y)
{
    if (native_get_monitor_rectangle && x && y)
    {
        float width, height;
        native_get_monitor_rectangle(monitor_index, x, y, &width, &height);
    }
}

void UniWinCore::get_monitor_rectangle(int monitor_index, float *x, float *y, float *width, float *height)
{
    if (native_get_monitor_rectangle && x && y && width && height)
    {
        native_get_monitor_rectangle(monitor_index, x, y, width, height);
    }
}

void UniWinCore::minimize_window()
{
    if (native_minimize_window)
    {
        native_minimize_window();
    }
}

void UniWinCore::maximize_window()
{
    if (native_maximize_window)
    {
        native_maximize_window();
    }
}

void UniWinCore::restore_window()
{
    if (native_restore_window)
    {
        native_restore_window();
    }
}