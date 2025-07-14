#ifndef UNIWINC_EXTENSION_H
#define UNIWINC_EXTENSION_H

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// 扩展初始化函数
void initialize_uniwinc_module(ModuleInitializationLevel p_level);
void uninitialize_uniwinc_module(ModuleInitializationLevel p_level);

extern "C" {
    // GDExtension 入口点
    GDExtensionBool GDE_EXPORT gdextension_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        const GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization
    );
}

#endif // UNIWINC_EXTENSION_H