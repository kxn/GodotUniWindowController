#ifndef UNIWINC_MESSAGE_QUEUE_H
#define UNIWINC_MESSAGE_QUEUE_H

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <mutex>
#include <queue>
#include <memory>

using namespace godot;

// 消息类型枚举
enum class MessageType {
    FILES_DROPPED,
    WINDOW_FOCUS_CHANGED,
    WINDOW_MOVED,
    WINDOW_RESIZED,
    MONITOR_CHANGED
};

// 消息基类
struct Message {
    MessageType type;
    
    Message(MessageType t) : type(t) {}
    virtual ~Message() = default;
};

// 文件拖拽消息
struct FilesDroppedMessage : public Message {
    String file_paths;
    
    FilesDroppedMessage(const String& paths) 
        : Message(MessageType::FILES_DROPPED), file_paths(paths) {}
};

// 窗口焦点变化消息
struct WindowFocusChangedMessage : public Message {
    bool focused;
    
    WindowFocusChangedMessage(bool f) 
        : Message(MessageType::WINDOW_FOCUS_CHANGED), focused(f) {}
};

// 窗口移动消息
struct WindowMovedMessage : public Message {
    float x, y;
    
    WindowMovedMessage(float px, float py) 
        : Message(MessageType::WINDOW_MOVED), x(px), y(py) {}
};

// 窗口大小变化消息
struct WindowResizedMessage : public Message {
    float width, height;
    
    WindowResizedMessage(float w, float h) 
        : Message(MessageType::WINDOW_RESIZED), width(w), height(h) {}
};

// 显示器变化消息
struct MonitorChangedMessage : public Message {
    int monitor_index;
    
    MonitorChangedMessage(int index) 
        : Message(MessageType::MONITOR_CHANGED), monitor_index(index) {}
};

// 线程安全的消息队列
class MessageQueue {
private:
    static std::mutex queue_mutex;
    static std::queue<std::unique_ptr<Message>> message_queue;
    static bool is_initialized;

public:
    // 初始化消息队列
    static void initialize();
    
    // 清理消息队列
    static void cleanup();
    
    // 添加消息到队列（线程安全）
    static void push_message(std::unique_ptr<Message> message);
    
    // 从队列中取出消息（线程安全）
    static std::unique_ptr<Message> pop_message();
    
    // 检查队列是否为空
    static bool is_empty();
    
    // 获取队列大小
    static size_t size();
    
    // 清空队列
    static void clear();
};

#endif // UNIWINC_MESSAGE_QUEUE_H