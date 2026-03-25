// #include "DeviceController.h"
#include "EquipmentController.h"
#include "MockMotor.h"
#include <memory>
// #include <chrono> 时间单位计时器
// #include <iostream> 标准输入输出流
// #include <thread> 线程支持 提供多线程功能
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 1. 创建共享的电机资源
    auto motor = std::make_shared<MockMotor>();
    // std::make_shared是一个工厂函数，用于创建一个对象并返回一个指向该对象的std::shared_ptr智能指针。它的好处是能够避免手动使用new操作符，减少内存泄漏的风险，并且在创建对象时提供了更好的异常安全性。
    
    // 2. 创建高级控制器，注入依赖
    // 在真实的半导体厂里，机台启动时会先读取一个 XML 配置文件，发现这台机器挂载了 3 个电机、2 个阀门。程序会先在内存里把这些底层的 shared_ptr<Hardware> 挨个实例化，然后再统统“注入（传参）”给 Controller。这就实现了硬件配置与控制逻辑的完美解耦。
    EquipmentController equipment(motor);

    equipment.initialize();

    // 场景 1: 正常完整流程
    std::cout << "\n--- Scenario 1: Normal Run ---\n";
    if (equipment.startProcess()) {//这里是瞬间返回因为在startProcess中只是修改了标志位并notify_all了后台线程，真正的工艺流程是在后台线程中执行的，所以主线程可以继续往下走，等待足够长的时间让后台跑完。
        // 主线程等待足够长的时间让后台跑完
        std::this_thread::sleep_for(std::chrono::seconds(4));//这里的sleep_for是为了让主线程等待足够长的时间，让后台线程能够完成整个工艺流程的模拟运行，确保我们能够看到完整的流程输出。
        //通常在实际应用场景中这一步的意义是在UI界面保证用户能够看到完整的流程输出。
    }

    // 场景 2: 启动后中途打断
    std::cout << "\n--- Scenario 2: Start and Interrupt ---\n";
    if (equipment.startProcess()) {
        // 让它跑 1 秒（进入 Step 2 Processing 阶段）
        std::this_thread::sleep_for(std::chrono::seconds(1));//因为后台线程中设定的是2s的超时时间，这里刚好模拟中途打断
        std::cout << "[Main] USER PRESSED STOP!\n";
        equipment.stopProcess();
    }

    // 等待一会看状态
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "[Main] Final State: " << equipment.getStateString() << "\n";

    return 0;

    /* Original Code:
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
    */
}
