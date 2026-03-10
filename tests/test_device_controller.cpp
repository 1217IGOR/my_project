#include "DeviceController.h"

#include <cassert>
#include <iostream>

int main() {
    DeviceController controller("TestTool");

    assert(!controller.isRunning());
    controller.start();
    assert(controller.isRunning());

    controller.stop();
    assert(!controller.isRunning());

    controller.emergencyShutdown();
    assert(!controller.isRunning());

    std::cout << "All DeviceController tests passed.\n";
    return 0;
}
