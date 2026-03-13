#include "DeviceController.h"

#include <cassert>//c标准断言库，提供了assert宏，用于在调试阶段验证程序的假设，如果条件为false，程序将终止并输出错误信息。
#include <iostream>

int main() {
    DeviceController controller("TestTool");

    //断言即契约，我认为此时条件为真，若不是，则代码有bug
    //断言的模式取决于是否定义宏，如果定义NDEBUG则断言被禁用，assert宏将不执行任何操作，条件表达式也不会被计算，这样可以避免在生产环境中引入性能开销。
    //反之，如果没有定义NDEBUG，assert宏将会检查条件表达式，如果条件为false，程序将输出错误信息并终止。这种机制有助于在开发和测试阶段捕捉潜在的逻辑错误和不一致性。
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
