#pragma once

#include "IMotor.h"

// MockMotor simulates a motor with software-imposed delay to mimic movement time.
class MockMotor : public IMotor {
public:
    MockMotor() = default;

    void initialize() override;
    void moveToPosition(double position_mm) override;
    void stop() override;
    
    [[nodiscard]] double currentPosition() const override;
    [[nodiscard]] std::uint32_t status() const override;

private:
    double m_position{0.0};
    bool m_initialized{false};
    // Speed in mm/s for simulation calculation
    static constexpr double kSimulationSpeed = 100.0; 
    //static含义为静态存储期，所有对象共用一份，无this指针
    //constexpr含义为编译时常量，必须在编译时就能确定值，且隐式具有inline属性，允许在头文件中定义而不会导致链接错误。
    //命名规范（Google Style：k + CamelCase）类级常量标识
    //最终含义：这是一个属于类而不是对象的常量，在编译器确定了值，类型double，所有MockMotor对象共享这个值，且不能被修改，适合用来表示模拟运动的速度参数。
};
