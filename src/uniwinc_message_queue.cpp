#include "uniwinc_message_queue.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// 静态成员定义
std::mutex MessageQueue::queue_mutex;
std::queue<std::unique_ptr<Message>> MessageQueue::message_queue;
bool MessageQueue::is_initialized = false;

void MessageQueue::initialize() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (!is_initialized) {
        // 清空队列（如果有残留数据）
        while (!message_queue.empty()) {
            message_queue.pop();
        }
        is_initialized = true;
        UtilityFunctions::print("MessageQueue initialized");
    }
}

void MessageQueue::cleanup() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (is_initialized) {
        // 清空队列
        while (!message_queue.empty()) {
            message_queue.pop();
        }
        is_initialized = false;
        UtilityFunctions::print("MessageQueue cleaned up");
    }
}

void MessageQueue::push_message(std::unique_ptr<Message> message) {
    if (!message) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (is_initialized) {
        message_queue.push(std::move(message));
    }
}

std::unique_ptr<Message> MessageQueue::pop_message() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (!is_initialized || message_queue.empty()) {
        return nullptr;
    }
    
    auto message = std::move(message_queue.front());
    message_queue.pop();
    return message;
}

bool MessageQueue::is_empty() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return !is_initialized || message_queue.empty();
}

size_t MessageQueue::size() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return is_initialized ? message_queue.size() : 0;
}

void MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (is_initialized) {
        while (!message_queue.empty()) {
            message_queue.pop();
        }
    }
}