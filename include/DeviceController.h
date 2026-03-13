#pragma once

#include <string>

// DeviceController simulates a semiconductor manufacturing tool's basic lifecycle.
class DeviceController {
public:
    explicit DeviceController(std::string name);
    //explicit 显示构造函数，禁止隐式类型转换，确保只能通过提供一个字符串参数来创建DeviceController对象，避免了潜在的错误。
    //eg：如果没有explicit的话，会导致意外的对象创建，使用了explict之后，必须使用DeviceController controller("Etcher-01"); 这种方式来创建对象，确保代码的清晰和安全。
    //比如某个函数需要一个DeviceController对象，如果传入一个字符串而不是DeviceController对象，编译器会报错，提示需要显式构造函数调用。代码示例：
    //void processDevice(const DeviceController &controller) {
    //    // 处理设备
    //}
    //processDevice("Etcher-01"); // 编译错误，必须显式创建DeviceController对象
    //processDevice(DeviceController("Etcher-01")); // 正确，显式创建DeviceController对象
    //如果不考虑的话可能会面临的问题：当构造一个对象时同时需要对某接口进行操作，析构时需要对接口进行清理，如果不使用explicit，可能会导致对象被意外创建，进而引发资源泄漏或未定义行为。
    //例如：如果有一个函数需要一个DeviceController对象，但传入了一个字符串，编译器会隐式调用构造函数创建一个临时的DeviceController对象
    //但是该对象并没有被使用，在本行语句结束时寿命结束调用析构，析构函数中将某个其他对象用的接口进行了清理，导致资源泄漏或未定义行为。

    void start();
    void stop();
    void emergencyShutdown();

    [[nodiscard]] bool isRunning() const;//nodiscard是C++17引入的属性，表示函数的返回值不应该被忽略，如果调用者没有使用返回值，编译器会发出警告。
    //实际体现比如：在main函数中调用controller.isRunning();但没有使用返回值，例如需要根据该函数的返回值进行判断是否stop，这时编译器会警告开发者注意这个返回值，避免潜在的逻辑错误。
    //函数括号后的const表示该函数为只读 不修改类里面的任何成员变量
    [[nodiscard]] const std::string &name() const;
    //如果在定义一个对象时使用了const DeviceController mainValue{"Etcher-01"}; 那么这个对象就不能被修改，任何试图修改这个对象的操作都会导致编译错误。如果要调用必须调用尾部带const的函数即保护类自身内部的数据不被篡改
    //本行语句中的const std::string &name() const; 表示这个函数返回一个常量引用，调用者不能修改这个字符串，同时函数本身也不会修改类的成员变量。这种设计确保了对象的不可变性和数据的安全性。
    //这个函数不修改对象，且调用者拿到的引用不能用来修改——但原对象仍可通过其他非常量函数修改，除非对象本身是 const 的

private:
    std::string m_name;
    bool m_running{false};//列表初始化，防止窄化转换（把高精度大容量数据塞进一个低精度小容量的变量里导致数据丢失或溢出），同时也提供了默认值，确保在对象创建时m_running被正确初始化为false。
    //现在C++17强推的列表初始化，规定和统一了初始化标准语法
};
