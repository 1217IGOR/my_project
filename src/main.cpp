#include "DeviceController.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    DeviceController controller("Etcher-01");

    try {
        controller.start();
        std::cout << controller.name() << " running: " << std::boolalpha << controller.isRunning() << '\n';

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        controller.stop();
        std::cout << controller.name() << " running: " << std::boolalpha << controller.isRunning() << '\n';
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        controller.emergencyShutdown();
    }

    return 0;
}
