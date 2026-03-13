#include "DeviceController.h"

#include <chrono>//时间单位/计时器；提供了时间相关的功能，例如std::chrono::milliseconds，用于指定睡眠时间。
#include <iostream>//标准输入输出流；提供了输入输出功能，例如std::cout和std::cerr，用于打印信息和错误消息。
#include <thread>//线程支持；提供了多线程功能，例如std::this_thread::sleep_for，用于让当前线程休眠指定的时间。

int main() {
    DeviceController controller("Etcher-01");//创建一个DeviceController对象并初始化其成员变量m_name为Etcher-01，模拟一个半导体制造工具的基本生命周期。

    try {//try块内部为主流程，捕获可能抛出的异常，确保程序的健壮性和稳定性。
        controller.start();
        std::cout << controller.name() << " running: " << std::boolalpha << controller.isRunning() << '\n';//boolalpha是一个流操纵器，用于将布尔值以true或false的形式输出，而不是默认的1或0。

        std::this_thread::sleep_for(std::chrono::milliseconds(100));//让当前线程休眠100毫秒，模拟设备运行一段时间的过程。
        controller.stop();
        std::cout << controller.name() << " running: " << std::boolalpha << controller.isRunning() << '\n';
    } catch (const std::exception &ex) {//这里的catch参数是标准的异常类的常量引用，捕获所有派生自std::exception的异常，确保能够处理各种可能的错误情况。
        //如果不使用“&”，而是直接使用std::exception ex，那么在抛出异常时会发生对象切割，导致捕获到的异常对象不完整，无法访问派生类特有的信息和功能。（C++多态 动态绑定概念）
        std::cerr << "Error: " << ex.what() << '\n';//std::cerr是标准错误流，用于输出错误消息；ex.what()返回一个描述异常的字符串，提供了关于错误的详细信息。
        controller.emergencyShutdown();
    }

    return 0;
}
