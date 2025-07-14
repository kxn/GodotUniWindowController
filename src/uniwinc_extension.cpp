#include "uniwinc_extension.h"
#include "uniwinc_controller.h"
#include "uniwinc_file_dialog.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_uniwinc_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // 注册自定义类
    ClassDB::register_class<UniWindowController>();
    ClassDB::register_class<UniWinFileDialog>();
    
    UtilityFunctions::print("UniWindowController GDExtension initialized");
}

void uninitialize_uniwinc_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    UtilityFunctions::print("UniWindowController GDExtension uninitialized");
}

extern "C" {
    GDExtensionBool GDE_EXPORT gdextension_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        const GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization
    ) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        
        init_obj.register_initializer(initialize_uniwinc_module);
        init_obj.register_terminator(uninitialize_uniwinc_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
        
        return init_obj.init();
    }
}