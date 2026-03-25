#include "MockMotor.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

MockMotor::MockMotor(){
    // 启动运动模拟线程
    m_running = true;//应该运行
    m_is_moving = false;//初始不应该在运行
    m_worker = std::thread(&MockMotor::motionLoop, this);
}

MockMotor::~MockMotor(){
    //销毁对象前必须确保所有线程均停止并回收资源
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_running = false;//告诉线程不应该再运行了
        m_is_moving = true;//防御性编程，让线程如果本身也没有在运动，也能及时醒来检查到m_running被设置为false了，赶紧退出循环
        //这一步的意义主要在于能够确保不论wait的判断逻辑是基于m_running还是m_is_moving，线程都能够及时地被唤醒
        //并在唤醒后优先检查到m_running被设置为false了，从而正确地退出循环和结束线程，避免了潜在的线程泄漏和资源浪费问题。
    }
    m_cv.notify_one();//唤醒线程让它检查m_running标志位，及时退出循环

    if(m_worker.joinable()){
        m_worker.join();//等待线程真正结束主线程才可以继续销毁对象，确保资源的正确释放和避免潜在的并发问题。
    }
}

void MockMotor::initialize() {
    std::cout << "[MockMotor] Initializing...\n";
    m_initialized = true;
}

void MockMotor::moveToPosition(double position_mm) {
    if (!m_initialized) {
        std::cerr << "[MockMotor] Error: Motor not initialized!\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_target_position = position_mm;
        m_is_moving = true;
    }
    m_cv.notify_one(); //通知运动线程有新的目标位置

    std::cout << "[MockMotor] Command received to move to " << position_mm << " mm.\n";
}

/*void MockMotor::moveToPosition(double position_mm) {
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
    //采用了chrono时间库，保证了时间单位的安全。这里的具体实现通过std::chrono::duration<double>将这里的普通double参数包装为一个以秒为单位的时间段
    //然后用sleep_for接收这个参数，从语法层面彻底杜绝了“把毫秒当成秒去休眠”的低级物理事故
    //std::this_thread::sleep_for是标准库函数中的对当前正在运行这个函数的进程的操作，在这里就是主线程，sleep_for就是挂起，让出CPU让其他就绪线程可以支配资源

    m_position = position_mm;
    std::cout << "[MockMotor] Arrived at " << m_position << ".\n";
}在当前的简单阻塞版本中，moveToPosition函数通过sleep_for来模拟电机运动的时间，这会导致整个线程被阻塞，无法响应其他指令，比如stop指令。实际的非阻塞实现需要通过多线程或者异步机制来实现，确保在运动过程中仍然能够响应停止命令。*/

void MockMotor::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_is_moving = false;//告诉电机不要再运动了
        //实际应用中可能还需要更新m_target_position为当前位置，或者设置一个特殊的停止标志位，让运动线程能够正确地处理停止命令，立即停止运动并保持当前位置不变。
    }
    m_cv.notify_one();//唤醒运动线程让它检查m_is_moving标志位，及时停止运动
    std::cout << "[MockMotor] Stop command executed immediately.\n";
}
    
/*void MockMotor::stop() {
    std::cout << "[MockMotor] Stopped.\n";
    // In a blocking simulation, stop is hard to interrupt without threads,
    // but conceptually this resets the state.
}在当前的简单阻塞版本中，从概念上stop停止了任务重置了标志位，但是实际上该函数并不能在移动过程中
起作用，只是一个象征性的函数，实际的实现需要通过多线程思路进行实现，控制线程不仅要实现move功能，还需要
设立一个标志位进行不断重复检查，保证在有人要叫停的时候能做出反馈*/

double MockMotor::currentPosition() const {
    //读数据也要加锁，防止读到脏数据
    std::lock_guard<std::mutex> lock(m_mutex);
    //这里的锁保护了对m_position的访问，确保在多线程环境下读取到的数据是最新的、完整的，避免了可能的竞态条件和数据不一致问题。
    return m_position;
}

std::uint32_t MockMotor::status() const {
    if(!m_initialized) return 0; //未初始化
    return m_is_moving ? 2 : 1; // 1表示准备就绪(空闲），2表示正在移动
}

bool MockMotor::isMoving() const {
    return m_is_moving;
}

//一个独立线程中运行的死循环，用来模拟物理运动
void MockMotor::motionLoop(){
    while(m_running){
        std::unique_lock<std::mutex> lock(m_mutex);

        //等待，如果没在动，也没有要推出，就挂起线程节约资源
        m_cv.wait(lock,[this]{
            return m_is_moving || !m_running;//如果正在运动或者不应该再运行了，就醒来检查
        });
        if(!m_running) break;//如果不应该再运行了，退出循环结束线程

        if(!m_is_moving) continue;//如果不该在运动了，回到循环顶部继续等待

        //模拟运动时间，按照距离和速度计算需要的时间
        double target = m_target_position;
        double diff = target - m_position;

        if(std::abs(diff) < 1e-6){
            m_position = target;
            m_is_moving = false;
            std::cout << "[MockMotor] Reached target position: " << m_position << "\n";
            continue;
        }

        double dt = 0.05;//时间切片为50ms，模拟电机每50ms更新一次位置，这个时间切片的选择是一个权衡，过大可能导致运动不够平滑，过小可能增加CPU负担。
        double step = kSimulationSpeed * dt;//计算这一步走了多远

        if(std::abs(diff) < step){
            step = diff;//如果剩余距离小于这一步的距离，就直接走到目标位置
        }

        m_position += step * (diff > 0 ? 1 : -1);//按照方向更新位置

        //关键点，在sleep前解锁，允许其他线程调用stop来修改m_is_moving标志位，从而实现可打断的运动模拟
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::duration<double>(dt));
    }
}
