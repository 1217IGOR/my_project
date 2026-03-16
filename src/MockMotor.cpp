#include "MockMotor.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

void MockMotor::initialize() {
    std::cout << "[MockMotor] Initializing...\n";
    m_initialized = true;
}

void MockMotor::moveToPosition(double position_mm) {
    if (!m_initialized) {
        std::cerr << "[MockMotor] Error: Motor not initialized!\n";
        return;
    }

    double distance = std::abs(position_mm - m_position);//abs取绝对值
    //符合物理规则的数值判断，工业级别的数值判断，避免浮点误差导致的无意义移动
    if (distance < 1e-6) return;

    // Calculate time required: time = distance / speed
    double duration_seconds = distance / kSimulationSpeed;
    
    std::cout << "[MockMotor] Moving from " << m_position << " to " << position_mm 
              << " (Time: " << duration_seconds << "s)...\n";

    // Block logic: sleep to simulate hardware movement time阻塞逻辑 通过睡眠（阻塞）模拟硬件运动时长
    // In a non-blocking implementation, this would start a thread or timer.在非阻塞实现模式下这一步应该启动一个线程或者计时器
    std::this_thread::sleep_for(std::chrono::duration<double>(duration_seconds));
    /*采用了chrono时间库，保证了时间单位的安全。这里的具体实现通过std::chrono::duration<double>将这里的普通double参数包装为一个以秒为单位的时间段
    然后用sleep_for接收这个参数，从语法层面彻底杜绝了“把毫秒当成秒去休眠”的低级物理事故
    std::this_thread::sleep_for是标准库函数中的对当前正在运行这个函数的进程的操作，在这里就是主线程，sleep_for就是挂起，让出CPU让其他就绪线程可以支配资源*/

    m_position = position_mm;
    std::cout << "[MockMotor] Arrived at " << m_position << ".\n";
}

void MockMotor::stop() {
    std::cout << "[MockMotor] Stopped.\n";
    // In a blocking simulation, stop is hard to interrupt without threads,
    // but conceptually this resets the state.
}/*在当前的简单阻塞版本中，从概念上stop停止了任务重置了标志位，但是实际上该函数并不能在移动过程中
起作用，只是一个象征性的函数，实际的实现需要通过多线程思路进行实现，控制线程不仅要实现move功能，还需要
设立一个标志位进行不断重复检查，保证在有人要叫停的时候能做出反馈*/

double MockMotor::currentPosition() const {
    return m_position;
}

std::uint32_t MockMotor::status() const {
    return m_initialized ? 1 : 0; // 0: Uninit, 1: Ready
}
