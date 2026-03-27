#pragma once

#include "IMotor.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

enum class EquipmentState {
    Idle,//空闲
    Running,
    Error
};
/*enum class枚举类是C++11中引入的一种新的类型安全的枚举方式“强类型枚举”，用来定义一组固定的、命名的
常量，它可以避免传统枚举的一些问题，比如作用域和类型安全。 老式的枚举首先名字会污染，两个不同的
枚举组件中不能同名，其次枚举值会隐式地转换为整数，被拿去进行数学运算导致类型错误。
而现代工业规范必须使用enum class，首先禁止其数据类型转换，其次同名不再互相干扰。
在半导体设备中enum class是一切状态机的基础，设备当前运行的状态、报警的级别、通讯指令的类型等等
都可以用enum class来定义，保证了代码的清晰和安全性。
不同于std::varient，std::varient是一个类型安全的联合体，可以存储多种类型中的一种，并且在访问时
会进行类型检查，防止类型错误，它适用于需要存储不同类型数据的场景，本质是一个能够携带不同类型数据的
容器，而enum class更适合定义一组固定的常量值，表示状态或类别等，本质是一个状态标签。
#include <variant>
#include <string>

// 这个变量 param，要不就是 int，要不就是 double，要不就是 string
std::variant<int, double, std::string> param;

param = 200;           // 现在里面装的是 int
param = 10.5;          // 原来的 int 被自动清空，现在装的是 double
param = "Wafer_A";     // 现在装的是 string
*/

/*不同于mockMotor类这样的被动接收指令的底层类，EquipmentController是一个主动管理设备状态和流程的控制器类*/
class EquipmentController {
public:
    explicit EquipmentController(std::shared_ptr<IMotor> motor);//share_ptr智能指针，能够通过引用计数保证只要还有人在使用这个电机它就不会被销毁
    //避免使用裸指针因为他们不具备所有权语义，这个电机可能同时被多个控制器使用，使用shared_ptr可以确保资源的正确管理和避免内存泄漏。
    ~EquipmentController();

    // 显式地禁止拷贝，因为管理着线程和互斥量，捍卫独占资源
    EquipmentController(const EquipmentController&) = delete;
    EquipmentController& operator=(const EquipmentController&) = delete;

    // 业务接口
    void initialize();
    bool startProcess(); // 模拟开始一个工艺流程
    void stopProcess();  // 立即停止

    EquipmentState getState() const;
    std::string getStateString() const;

private:
    void workerLoop(); // 后台工作线程函数

    std::shared_ptr<IMotor> m_motor;
    std::atomic<EquipmentState> m_state{EquipmentState::Idle};
    /*atomic原子变量，这里使用原子变量的原因是，该变量要体现的是一个单一状态值，而在实际应用场景中可能在UI界面上每一秒钟都有
    60+帧，每一帧都去获取该状态，如果用mutex会导致频繁地争抢锁，性能损耗极大；而原子变量借助了CPU的底层指令，所以保证了对这个
    变量的读写是绝对不可分割的，既保证了线程安全（不会出现状态撕裂），也提高了性能*/

    // 线程管理
    std::thread m_worker;
    std::atomic<bool> m_running{false}; // 标记线程是否应该继续运行（workerloop是否存活，只要控制器没被销毁就一直存活）
    std::atomic<bool> m_process_active{false}; // 标记当前是否正在跑Process（只有工艺流程被stopProcess叫停才会变成false，即使它变成false，workerloop线程也不会退出，因为它的生命由m_running控制）
    //分别设立原子变量来标记线程生命和业务生命
    //新增原子变量 让workerLoop在运动等待期间可以立即响应stopProcess的调用的标志位 不复用m_process_active是因为它的语义是“工艺流程是否在跑”，如果复用当其为false时无法分辨是被请求中断结束还是自然结束，对日志、状态上报、错误处理等不友好
    std::atomic<bool> m_stop_requested{false};

    // 同步原语：用于实现可打断的 sleep，当条件变量被唤醒时，能够检查该线程是否应该继续等待还是被叫停
    std::mutex m_cv_mutex;
    std::condition_variable m_cv;
};
