#include "uniwinc_file_dialog.h"
#include "uniwinc_core.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// FileFilter 实现
String FileFilter::to_string() const {
    String result = title + String("\t");
    for (int i = 0; i < extensions.size(); i++) {
        if (i > 0) {
            result += String("\t");
        }
        result += extensions[i];
    }
    return result;
}

String FileFilter::join_filters(const std::vector<FileFilter>& filters) {
    String result = "";
    bool first = true;
    for (const auto& filter : filters) {
        if (!first) {
            result += String("\n");
        }
        result += filter.to_string();
        first = false;
    }
    return result;
}

void UniWinFileDialog::_bind_methods() {
    // 枚举绑定
    BIND_ENUM_CONSTANT(OPEN_FILE);
    BIND_ENUM_CONSTANT(SAVE_FILE);
    BIND_ENUM_CONSTANT(OPEN_DIRECTORY);
    BIND_ENUM_CONSTANT(OPEN_MULTIPLE_FILES);

    // 标志枚举绑定
    BIND_ENUM_CONSTANT(FLAG_NONE);
    BIND_ENUM_CONSTANT(FLAG_FILE_MUST_EXIST);
    BIND_ENUM_CONSTANT(FLAG_FOLDER_MUST_EXIST);
    BIND_ENUM_CONSTANT(FLAG_ALLOW_MULTIPLE_SELECTION);
    BIND_ENUM_CONSTANT(FLAG_CAN_CREATE_DIRECTORIES);
    BIND_ENUM_CONSTANT(FLAG_OVERWRITE_PROMPT);
    BIND_ENUM_CONSTANT(FLAG_CREATE_PROMPT);
    BIND_ENUM_CONSTANT(FLAG_SHOW_HIDDEN_FILES);
    BIND_ENUM_CONSTANT(FLAG_RETRIEVE_LINK);

    // 属性绑定
    ClassDB::bind_method(D_METHOD("set_title", "title"), &UniWinFileDialog::set_title);
    ClassDB::bind_method(D_METHOD("get_title"), &UniWinFileDialog::get_title);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");

    ClassDB::bind_method(D_METHOD("set_filters", "filters"), &UniWinFileDialog::set_filters);
    ClassDB::bind_method(D_METHOD("get_filters"), &UniWinFileDialog::get_filters);
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "filters"), "set_filters", "get_filters");

    ClassDB::bind_method(D_METHOD("add_filter", "title", "extensions"), &UniWinFileDialog::add_filter);
    ClassDB::bind_method(D_METHOD("clear_filters"), &UniWinFileDialog::clear_filters);

    ClassDB::bind_method(D_METHOD("set_initial_directory", "directory"), &UniWinFileDialog::set_initial_directory);
    ClassDB::bind_method(D_METHOD("get_initial_directory"), &UniWinFileDialog::get_initial_directory);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "initial_directory"), "set_initial_directory", "get_initial_directory");

    ClassDB::bind_method(D_METHOD("set_initial_file", "file"), &UniWinFileDialog::set_initial_file);
    ClassDB::bind_method(D_METHOD("get_initial_file"), &UniWinFileDialog::get_initial_file);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "initial_file"), "set_initial_file", "get_initial_file");

    ClassDB::bind_method(D_METHOD("set_default_extension", "extension"), &UniWinFileDialog::set_default_extension);
    ClassDB::bind_method(D_METHOD("get_default_extension"), &UniWinFileDialog::get_default_extension);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_extension"), "set_default_extension", "get_default_extension");

    ClassDB::bind_method(D_METHOD("set_dialog_type", "type"), &UniWinFileDialog::set_dialog_type);
    ClassDB::bind_method(D_METHOD("get_dialog_type"), &UniWinFileDialog::get_dialog_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "dialog_type", PROPERTY_HINT_ENUM, "OpenFile,SaveFile,OpenDirectory,OpenMultipleFiles"), "set_dialog_type", "get_dialog_type");

    // 标志设置方法
    ClassDB::bind_method(D_METHOD("set_file_must_exist", "must_exist"), &UniWinFileDialog::set_file_must_exist);
    ClassDB::bind_method(D_METHOD("get_file_must_exist"), &UniWinFileDialog::get_file_must_exist);
    
    ClassDB::bind_method(D_METHOD("set_folder_must_exist", "must_exist"), &UniWinFileDialog::set_folder_must_exist);
    ClassDB::bind_method(D_METHOD("get_folder_must_exist"), &UniWinFileDialog::get_folder_must_exist);
    
    ClassDB::bind_method(D_METHOD("set_allow_multiple_selection", "allow"), &UniWinFileDialog::set_allow_multiple_selection);
    ClassDB::bind_method(D_METHOD("get_allow_multiple_selection"), &UniWinFileDialog::get_allow_multiple_selection);
    
    ClassDB::bind_method(D_METHOD("set_can_create_directories", "allow"), &UniWinFileDialog::set_can_create_directories);
    ClassDB::bind_method(D_METHOD("get_can_create_directories"), &UniWinFileDialog::get_can_create_directories);
    
    ClassDB::bind_method(D_METHOD("set_overwrite_prompt", "prompt"), &UniWinFileDialog::set_overwrite_prompt);
    ClassDB::bind_method(D_METHOD("get_overwrite_prompt"), &UniWinFileDialog::get_overwrite_prompt);
    
    ClassDB::bind_method(D_METHOD("set_create_prompt", "prompt"), &UniWinFileDialog::set_create_prompt);
    ClassDB::bind_method(D_METHOD("get_create_prompt"), &UniWinFileDialog::get_create_prompt);
    
    ClassDB::bind_method(D_METHOD("set_show_hidden_files", "show"), &UniWinFileDialog::set_show_hidden_files);
    ClassDB::bind_method(D_METHOD("get_show_hidden_files"), &UniWinFileDialog::get_show_hidden_files);
    
    ClassDB::bind_method(D_METHOD("set_retrieve_link", "retrieve"), &UniWinFileDialog::set_retrieve_link);
    ClassDB::bind_method(D_METHOD("get_retrieve_link"), &UniWinFileDialog::get_retrieve_link);

    // 对话框方法
    ClassDB::bind_method(D_METHOD("open_file_dialog"), &UniWinFileDialog::open_file_dialog);
    ClassDB::bind_method(D_METHOD("open_file_dialog_multiple"), &UniWinFileDialog::open_file_dialog_multiple);
    ClassDB::bind_method(D_METHOD("save_file_dialog"), &UniWinFileDialog::save_file_dialog);
    ClassDB::bind_method(D_METHOD("open_directory_dialog"), &UniWinFileDialog::open_directory_dialog);

    // 静态便捷方法
    ClassDB::bind_static_method("UniWinFileDialog", D_METHOD("open_file", "title", "filters", "default_path"), &UniWinFileDialog::open_file, DEFVAL(""), DEFVAL(PackedStringArray()), DEFVAL(""));
    ClassDB::bind_static_method("UniWinFileDialog", D_METHOD("open_files", "title", "filters", "default_path"), &UniWinFileDialog::open_files, DEFVAL(""), DEFVAL(PackedStringArray()), DEFVAL(""));
    ClassDB::bind_static_method("UniWinFileDialog", D_METHOD("save_file", "title", "filters", "default_path", "default_filename"), &UniWinFileDialog::save_file, DEFVAL(""), DEFVAL(PackedStringArray()), DEFVAL(""), DEFVAL(""));
    ClassDB::bind_static_method("UniWinFileDialog", D_METHOD("open_directory", "title", "default_path"), &UniWinFileDialog::open_directory, DEFVAL(""), DEFVAL(""));
}

UniWinFileDialog::UniWinFileDialog() {
    _title = "Select File";
    _dialog_type = OPEN_FILE;
    _initial_directory = "";
    _initial_file = "";
    _default_extension = "";
    _flags = 0;
}

UniWinFileDialog::~UniWinFileDialog() {
}

void UniWinFileDialog::set_title(const String& title) {
    _title = title;
}

String UniWinFileDialog::get_title() const {
    return _title;
}

void UniWinFileDialog::set_filters(const PackedStringArray& filters) {
    _filters.clear();
    for (int i = 0; i < filters.size(); i += 2) {
        if (i + 1 < filters.size()) {
            String title = filters[i];
            PackedStringArray extensions;
            String ext_str = filters[i + 1];
            PackedStringArray ext_list = ext_str.split(";");
            for (int j = 0; j < ext_list.size(); j++) {
                extensions.append(ext_list[j].strip_edges());
            }
            _filters.push_back(FileFilter(title, extensions));
        }
    }
}

PackedStringArray UniWinFileDialog::get_filters() const {
    PackedStringArray result;
    for (const auto& filter : _filters) {
        result.append(filter.title);
        String ext_str = "";
        for (int i = 0; i < filter.extensions.size(); i++) {
            if (i > 0) ext_str += String(";");
            ext_str += filter.extensions[i];
        }
        result.append(ext_str);
    }
    return result;
}

void UniWinFileDialog::add_filter(const String& title, const PackedStringArray& extensions) {
    _filters.push_back(FileFilter(title, extensions));
}

void UniWinFileDialog::clear_filters() {
    _filters.clear();
}

void UniWinFileDialog::set_initial_directory(const String& directory) {
    _initial_directory = directory;
}

String UniWinFileDialog::get_initial_directory() const {
    return _initial_directory;
}

void UniWinFileDialog::set_initial_file(const String& file) {
    _initial_file = file;
}

String UniWinFileDialog::get_initial_file() const {
    return _initial_file;
}

void UniWinFileDialog::set_default_extension(const String& extension) {
    _default_extension = extension;
}

String UniWinFileDialog::get_default_extension() const {
    return _default_extension;
}

void UniWinFileDialog::set_dialog_type(DialogType type) {
    _dialog_type = type;
}

UniWinFileDialog::DialogType UniWinFileDialog::get_dialog_type() const {
    return _dialog_type;
}

// 标志设置方法的实现
void UniWinFileDialog::set_file_must_exist(bool must_exist) {
    _set_flag(FLAG_FILE_MUST_EXIST, must_exist);
}

bool UniWinFileDialog::get_file_must_exist() const {
    return _get_flag(FLAG_FILE_MUST_EXIST);
}

void UniWinFileDialog::set_folder_must_exist(bool must_exist) {
    _set_flag(FLAG_FOLDER_MUST_EXIST, must_exist);
}

bool UniWinFileDialog::get_folder_must_exist() const {
    return _get_flag(FLAG_FOLDER_MUST_EXIST);
}

void UniWinFileDialog::set_allow_multiple_selection(bool allow) {
    _set_flag(FLAG_ALLOW_MULTIPLE_SELECTION, allow);
}

bool UniWinFileDialog::get_allow_multiple_selection() const {
    return _get_flag(FLAG_ALLOW_MULTIPLE_SELECTION);
}

void UniWinFileDialog::set_can_create_directories(bool allow) {
    _set_flag(FLAG_CAN_CREATE_DIRECTORIES, allow);
}

bool UniWinFileDialog::get_can_create_directories() const {
    return _get_flag(FLAG_CAN_CREATE_DIRECTORIES);
}

void UniWinFileDialog::set_overwrite_prompt(bool prompt) {
    _set_flag(FLAG_OVERWRITE_PROMPT, prompt);
}

bool UniWinFileDialog::get_overwrite_prompt() const {
    return _get_flag(FLAG_OVERWRITE_PROMPT);
}

void UniWinFileDialog::set_create_prompt(bool prompt) {
    _set_flag(FLAG_CREATE_PROMPT, prompt);
}

bool UniWinFileDialog::get_create_prompt() const {
    return _get_flag(FLAG_CREATE_PROMPT);
}

void UniWinFileDialog::set_show_hidden_files(bool show) {
    _set_flag(FLAG_SHOW_HIDDEN_FILES, show);
}

bool UniWinFileDialog::get_show_hidden_files() const {
    return _get_flag(FLAG_SHOW_HIDDEN_FILES);
}

void UniWinFileDialog::set_retrieve_link(bool retrieve) {
    _set_flag(FLAG_RETRIEVE_LINK, retrieve);
}

bool UniWinFileDialog::get_retrieve_link() const {
    return _get_flag(FLAG_RETRIEVE_LINK);
}

String UniWinFileDialog::open_file_dialog() {
    return _execute_dialog();
}

PackedStringArray UniWinFileDialog::open_file_dialog_multiple() {
    String result = _execute_dialog();
    return _parse_result_paths(result);
}

String UniWinFileDialog::save_file_dialog() {
    return _execute_dialog();
}

String UniWinFileDialog::open_directory_dialog() {
    return _execute_dialog();
}

String UniWinFileDialog::open_file(const String& title, const PackedStringArray& filters, const String& default_path) {
    Ref<UniWinFileDialog> dialog;
    dialog.instantiate();
    
    if (!title.is_empty()) {
        dialog->set_title(title);
    }
    if (filters.size() > 0) {
        dialog->set_filters(filters);
    }
    if (!default_path.is_empty()) {
        dialog->set_initial_directory(default_path);
    }
    
    return dialog->open_file_dialog();
}

PackedStringArray UniWinFileDialog::open_files(const String& title, const PackedStringArray& filters, const String& default_path) {
    Ref<UniWinFileDialog> dialog;
    dialog.instantiate();
    
    if (!title.is_empty()) {
        dialog->set_title(title);
    }
    if (filters.size() > 0) {
        dialog->set_filters(filters);
    }
    if (!default_path.is_empty()) {
        dialog->set_initial_directory(default_path);
    }
    
    dialog->set_allow_multiple_selection(true);
    return dialog->open_file_dialog_multiple();
}

String UniWinFileDialog::save_file(const String& title, const PackedStringArray& filters, const String& default_path, const String& default_filename) {
    Ref<UniWinFileDialog> dialog;
    dialog.instantiate();
    
    if (!title.is_empty()) {
        dialog->set_title(title);
    }
    if (filters.size() > 0) {
        dialog->set_filters(filters);
    }
    if (!default_path.is_empty()) {
        dialog->set_initial_directory(default_path);
    }
    if (!default_filename.is_empty()) {
        dialog->set_initial_file(default_filename);
    }
    
    dialog->set_dialog_type(SAVE_FILE);
    return dialog->save_file_dialog();
}

String UniWinFileDialog::open_directory(const String& title, const String& default_path) {
    Ref<UniWinFileDialog> dialog;
    dialog.instantiate();
    
    if (!title.is_empty()) {
        dialog->set_title(title);
    }
    if (!default_path.is_empty()) {
        dialog->set_initial_directory(default_path);
    }
    
    dialog->set_dialog_type(OPEN_DIRECTORY);
    return dialog->open_directory_dialog();
}

// 私有方法实现
PanelSettings UniWinFileDialog::_create_panel_settings() const {
    PanelSettings settings;
    settings.title = _title;
    settings.filters = _filters;
    settings.initial_file = _initial_file;
    settings.initial_directory = _initial_directory;
    settings.default_extension = _default_extension;
    settings.flags = _flags;
    return settings;
}

PackedStringArray UniWinFileDialog::_parse_result_paths(const String& result) const {
    PackedStringArray files;
    
    if (!result.is_empty()) {
        PackedStringArray lines = result.split("\n");
        for (int i = 0; i < lines.size(); i++) {
            String file = lines[i].strip_edges();
            if (!file.is_empty()) {
                files.append(file);
            }
        }
    }
    
    return files;
}

String UniWinFileDialog::_execute_dialog() {
    PanelSettings settings = _create_panel_settings();
    String filters_str = FileFilter::join_filters(settings.filters);
    
    switch (_dialog_type) {
        case OPEN_FILE:
        case OPEN_MULTIPLE_FILES:
            return UniWinCore::open_file_panel(settings.title, filters_str, settings.initial_directory);
            
        case SAVE_FILE:
            return UniWinCore::save_file_panel(settings.title, filters_str, settings.initial_directory);
            
        case OPEN_DIRECTORY:
            // 目录选择可能需要特殊处理
            return UniWinCore::open_file_panel(settings.title, "", settings.initial_directory);
            
        default:
            UtilityFunctions::print("Unknown dialog type");
            return "";
    }
}

void UniWinFileDialog::_set_flag(DialogFlag flag, bool value) {
    if (value) {
        _flags |= flag;
    } else {
        _flags &= ~flag;
    }
}

bool UniWinFileDialog::_get_flag(DialogFlag flag) const {
    return (_flags & flag) != 0;
}