#pragma once

#include <cstdint>//绝对定义数据类型大小

// IMotor abstracts motor control for init, motion, and diagnostics.
class IMotor {
    //无构造函数默认一个无参数的构造函数，但是由于这个类是一个接口类（纯虚函数），通常不需要实例化对象，因此不需要定义构造函数。派生类将实现这些纯虚函数，并且可以根据需要定义自己的构造函数来初始化特定的成员变量。
public:
    virtual ~IMotor() = default;//虚析构函数，确保通过基类指针删除派生类对象时能够正确调用派生类的析构函数，避免资源泄漏和未定义行为。

    virtual void initialize() = 0;
    virtual void moveToPosition(double position_mm) = 0;//单位写死为毫米，明确接口的预期输入，避免单位混淆导致的错误。
    virtual void stop() = 0;
    virtual double currentPosition() const = 0;//绝不修改成员变量，承诺函数内部不会修改成员变量，确保调用者可以安全地获取当前位置信息而不担心对象状态被改变。
    virtual std::uint32_t status() const = 0;//电机状态要返回一个32位无符号整数，可以用来表示不同的状态码，例如0表示未初始化，1表示准备就绪，2表示正在移动，3表示错误等。使用std::uint32_t确保了状态码的大小和范围的一致性，便于跨平台开发和维护。
};
