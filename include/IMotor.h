#pragma once

#include <cstdint>//绝对定义数据类型大小

// IMotor abstracts motor control for init, motion, and diagnostics.
class IMotor {
public:
    //无构造函数默认一个无参数的构造函数，但是由于这个类是一个接口类（纯虚函数），通常不需要实例化对象，因此不需要定义构造函数。派生类将实现这些纯虚函数，并且可以根据需要定义自己的构造函数来初始化特定的成员变量。
    using CommandID = std::uint32_t;//定义一个类型别名CommandID，表示命令的唯一标识符，使用std::uint32_t确保了命令ID的大小和范围的一致性，便于跨平台开发和维护。
    
    enum class MotionState : std::uint8_t{
        Uninitialized = 0,
        Idle,
        Moving,
        Arrived,//最近一次命令正常到位
        Stopped,//最近一次命令被stop中断
        Fault
    };

    struct MotionSnapshot{//运动快照，包含当前位置信息和状态信息，供外部查询使用
        CommandID active_command_id{0};//激活这个运动快照的命令ID，外部可以通过这个ID来追踪和关联具体的运动命令，方便日志记录、状态监控和故障诊断等功能。
        CommandID last_finished_command_id{0};//最近一次完成的命令ID，用于记录和查询历史命令状态。
        MotionState state{MotionState::Uninitialized};//当前的运动状态，初始状态为未初始化。
        double current_position_mm{0.0};//当前的位置信息，单位为毫米，初始位置为0.0。
        double target_position_mm{0.0};//当前的目标位置信息，单位为毫米，初始目标位置为0.0。
        std::uint32_t states_code{0};//状态码，可以用来表示电机的各种状态，例如错误码、警告码等，初始值为0。

    };

    virtual ~IMotor() = default;//虚析构函数，确保通过基类指针删除派生类对象时能够正确调用派生类的析构函数，避免资源泄漏和未定义行为。

    virtual void initialize() = 0;

    // moveToPosition函数的返回值类型是CommandID，表示每次调用moveToPosition都会返回一个唯一的命令ID，这个ID可以用来追踪和关联具体的运动命令，方便日志记录、状态监控和故障诊断等功能。
    virtual CommandID moveToPosition(double position_mm) = 0;//单位写死为毫米，明确接口的预期输入，避免单位混淆导致的错误。
    virtual void stop() = 0;
    virtual double currentPosition() const = 0;//绝不修改成员变量，承诺函数内部不会修改成员变量，确保调用者可以安全地获取当前位置信息而不担心对象状态被改变。
    virtual std::uint32_t status() const = 0;//电机状态要返回一个32位无符号整数，可以用来表示不同的状态码，例如0表示未初始化，1表示准备就绪，2表示正在移动，3表示错误等。使用std::uint32_t确保了状态码的大小和范围的一致性，便于跨平台开发和维护。

    virtual MotionSnapshot snapshot() const = 0;//提供一个接口来获取当前的运动快照，包含位置信息和状态信息，供外部控制器明确查询查询使用。
};
