#pragma once

#include <string>

// DeviceController simulates a semiconductor manufacturing tool's basic lifecycle.
class DeviceController {
public:
    explicit DeviceController(std::string name);

    void start();
    void stop();
    void emergencyShutdown();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] const std::string &name() const;

private:
    std::string m_name;
    bool m_running{false};
};
