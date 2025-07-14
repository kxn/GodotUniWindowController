#ifndef UNIWIN_FILE_DIALOG_H
#define UNIWIN_FILE_DIALOG_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <vector>
#include <memory>

using namespace godot;

// 文件对话框标志枚举 (对应Unity的FilePanel.Flag)
enum DialogFlag {
    FLAG_NONE = 0,
    FLAG_FILE_MUST_EXIST = 1,
    FLAG_FOLDER_MUST_EXIST = 2,
    FLAG_ALLOW_MULTIPLE_SELECTION = 4,
    FLAG_CAN_CREATE_DIRECTORIES = 16,
    FLAG_OVERWRITE_PROMPT = 256,
    FLAG_CREATE_PROMPT = 512,
    FLAG_SHOW_HIDDEN_FILES = 4096,
    FLAG_RETRIEVE_LINK = 8192
};

// 文件过滤器类 (对应Unity的FilePanel.Filter)
class FileFilter {
public:
    String title;
    PackedStringArray extensions;
    
    FileFilter() {}
    FileFilter(const String& title, const PackedStringArray& extensions) 
        : title(title), extensions(extensions) {}
    
    String to_string() const;
    static String join_filters(const std::vector<FileFilter>& filters);
};

// 面板设置结构体 (对应Unity的PanelSettings)
struct PanelSettings {
    String title;
    std::vector<FileFilter> filters;
    String initial_file;
    String initial_directory;
    String default_extension;
    int flags;
    
    PanelSettings() : flags(0) {}
};

class UniWinFileDialog : public RefCounted {
    GDCLASS(UniWinFileDialog, RefCounted)

public:
    enum DialogType {
        OPEN_FILE,
        SAVE_FILE,
        OPEN_DIRECTORY,
        OPEN_MULTIPLE_FILES
    };

private:
    String _title;
    std::vector<FileFilter> _filters;
    String _initial_directory;
    String _initial_file;
    String _default_extension;
    int _flags;
    DialogType _dialog_type;

protected:
    static void _bind_methods();

public:
    UniWinFileDialog();
    ~UniWinFileDialog();

    // 属性设置
    void set_title(const String& title);
    String get_title() const;

    void set_filters(const PackedStringArray& filters);
    PackedStringArray get_filters() const;
    
    void add_filter(const String& title, const PackedStringArray& extensions);
    void clear_filters();

    void set_initial_directory(const String& directory);
    String get_initial_directory() const;

    void set_initial_file(const String& file);
    String get_initial_file() const;

    void set_default_extension(const String& extension);
    String get_default_extension() const;

    void set_dialog_type(DialogType type);
    DialogType get_dialog_type() const;

    // 标志设置 (对应Unity的Flag)
    void set_file_must_exist(bool must_exist);
    bool get_file_must_exist() const;
    
    void set_folder_must_exist(bool must_exist);
    bool get_folder_must_exist() const;
    
    void set_allow_multiple_selection(bool allow);
    bool get_allow_multiple_selection() const;
    
    void set_can_create_directories(bool allow);
    bool get_can_create_directories() const;
    
    void set_overwrite_prompt(bool prompt);
    bool get_overwrite_prompt() const;
    
    void set_create_prompt(bool prompt);
    bool get_create_prompt() const;
    
    void set_show_hidden_files(bool show);
    bool get_show_hidden_files() const;
    
    void set_retrieve_link(bool retrieve);
    bool get_retrieve_link() const;

    // 对话框操作 (对应Unity的OpenFilePanel/SaveFilePanel)
    String open_file_dialog();
    PackedStringArray open_file_dialog_multiple();
    String save_file_dialog();
    String open_directory_dialog();

    // 便捷的静态方法
    static String open_file(const String& title = "", const PackedStringArray& filters = PackedStringArray(), const String& default_path = "");
    static PackedStringArray open_files(const String& title = "", const PackedStringArray& filters = PackedStringArray(), const String& default_path = "");
    static String save_file(const String& title = "", const PackedStringArray& filters = PackedStringArray(), const String& default_path = "", const String& default_filename = "");
    static String open_directory(const String& title = "", const String& default_path = "");

private:
    PanelSettings _create_panel_settings() const;
    PackedStringArray _parse_result_paths(const String& result) const;
    String _execute_dialog();
    void _set_flag(DialogFlag flag, bool value);
    bool _get_flag(DialogFlag flag) const;
};

VARIANT_ENUM_CAST(UniWinFileDialog::DialogType);
VARIANT_ENUM_CAST(DialogFlag);

#endif // UNIWIN_FILE_DIALOG_H