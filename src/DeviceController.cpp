#include "DeviceController.h"

#include <stdexcept>

DeviceController::DeviceController(std::string name)
    : m_name(std::move(name)) {}

void DeviceController::start() {
    if (m_running) {
        throw std::runtime_error("Device already running");
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

bool DeviceController::isRunning() const {
    return m_running;
}

const std::string &DeviceController::name() const {
    return m_name;
}
