#include "DeviceController.h"

#include <stdexcept>//标准异常头文件

DeviceController::DeviceController(std::string name)
    : m_name(std::move(name)) {}//冒号引导的成员初始化列表；std::move(name)移动语义，摒弃原本的copy->move构造函数，避免不必要的复制，而是直接将内部资源的所有权转移到m_name中，提高性能。

void DeviceController::start() {
    if (m_running) {
        throw std::runtime_error("Device already running");//抛出异常，提示设备已经在运行了，防止重复启动导致的逻辑错误或资源冲突。
    }
    m_running = true;
}

void DeviceController::stop() {
    if (!m_running) {
        throw std::runtime_error("Device already stopped");
    }
    m_running = false;
}

void DeviceController::emergencyShutdown() {
    m_running = false;
}

bool DeviceController::isRunning() const {//承诺函数内部不会修改成员变量
    return m_running;
}

const std::string &DeviceController::name() const {//返回值是只读引用，不可通过该引用修改对象的名称，同时函数本身也不会修改成员变量 
    return m_name;
}
