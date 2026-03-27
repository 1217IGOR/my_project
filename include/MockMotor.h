#pragma once

#include "IMotor.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// MockMotor simulates a motor with software-imposed delay to mimic movement time.
class MockMotor : public IMotor {
public:
    MockMotor();//构造函数启动线程
    ~MockMotor() override;//析构函数，负责清理资源，确保在对象销毁时能够正确释放资源，避免内存泄漏和其他潜在问题。

    void initialize() override;
    CommandID moveToPosition(double position_mm) override;
    void stop() override;
    
    [[nodiscard]] double currentPosition() const override;
    [[nodiscard]] std::uint32_t status() const override;
    [[nodiscard]] MotionSnapshot snapshot() const override;

    [[nodiscard]] bool isMoving() const;

private:
    void motionLoop(); // 模拟运动的内部循环函数，实际实现中可能需要一个线程来执行这个函数，以便在moveToPosition中调用时能够模拟出非阻塞的运动效果。

    //运动学数据
    double m_position{0.0};
    double m_target_position{0.0};
    bool m_initialized{false};

    //命令与状态机
    std::atomic<CommandID> m_next_command_id{1};//命令ID生成器，初始值为1，每次调用moveToPosition时递增，确保每个命令都有一个唯一的ID，方便追踪和关联具体的运动命令。
    CommandID m_active_command_id{0};//当前正在执行的命令ID，初始值为0表示没有活动命令，当moveToPosition被调用时，这个ID会被更新为新的命令ID，stop命令可以通过这个ID来中断当前活动的命令。
    CommandID m_last_finished_command_id{0};//最近一次完成的命令ID，初始值为0，当一个运动命令完成时，这个ID会被更新为刚完成的命令ID，方便记录和查询历史命令状态。
    MotionState m_motion_state{MotionState::Unintialized};//当前的运动状态，初始状态为未初始化，当initialize()被调用时状态会变为Idle，moveToPosition被调用时状态会变为Moving，命令完成后状态会变为Arrived，如果stop被调用中断命令，状态会变为Stopped，如果发生错误状态会变为Fault。

    //非阻塞式模拟控制电机需要的状态变量
    std::thread m_worker;
    std::atomic<bool> m_running{false};//标记运动线程是否应该继续运行
    std::atomic<bool> m_is_moving{false};//标记电机线程当前是否在运动中 
    bool m_stop_requested{false};//标记是否有停止请求，运动线程需要检查这个标志位来决定是否应该立即停止运动

    mutable std::mutex m_mutex; // 保护对状态变量的访问，确保线程安全,这里的mutable是对锁的豁免权，因为在currentPosition和status等const函数中也需要访问这些状态变量，所以需要mutable来允许在const函数中修改锁的状态。
    std::condition_variable m_cv; // 用于通知运动线程有新的目标位置或者需要停止的信号

    // Speed in mm/s for simulation calculation
    static constexpr double kSimulationSpeed = 100.0; 
    //static含义为静态存储期，所有对象共用一份，无this指针
    //constexpr含义为编译时常量，必须在编译时就能确定值，且隐式具有inline属性，允许在头文件中定义而不会导致链接错误。
    //命名规范（Google Style：k + CamelCase）类级常量标识
    //最终含义：这是一个属于类而不是对象的常量，在编译器确定了值，类型double，所有MockMotor对象共享这个值，且不能被修改，适合用来表示模拟运动的速度参数。

};
