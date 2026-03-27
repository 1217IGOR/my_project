#include "EquipmentController.h"
#include <iostream>
#include <chrono>

EquipmentController::EquipmentController(std::shared_ptr<IMotor> motor)
    : m_motor(std::move(motor)) {
    // 构造时可以暂不启动线程，在 initialize 启动，也可以直接启动。
    // 为简单起见，在构造函数中就启动后台监控线程。
    m_running = true;
    m_worker = std::thread(&EquipmentController::workerLoop, this);
    //创建线程并绑定成员函数的标准语法，&EquipmentController::workerLoop表示要执行的成员函数
    //第一个参数时要执行的函数指针，普通函数可以直接传函数名（如 &myFunc 或 myFunc）。类的成员函数必须通过 &ClassName::FunctionName 的形式来获取其地址。
    //第二个成员函数是当前对象的this指针，因为成员函数需要一个对象实例来调用，this指针就是传递给成员函数的隐式参数，确保线程能够访问类的成员变量和函数。
    //启动一个后台线程，执行workerLoop函数，this指针传递给成员函数，确保线程能够访问类的成员变量和函数。
}

EquipmentController::~EquipmentController() {
    stopProcess(); // 先停业务再停线程

    // 停止线程
    m_running = false; 
    // 唤醒可能正在休眠的线程，让它赶紧退出循环
    m_cv.notify_all(); //条件变量的使用，notify_all()函数用于唤醒所有正在等待该条件变量的线程，确保当m_running被设置为false时，所有可能正在等待的线程都能被唤醒并检查这个标志位，从而正确地退出循环和结束线程。

    if (m_worker.joinable()) {
        m_worker.join(); // 等待线程真正结束主线程才可以继续销毁对象，确保资源的正确释放和避免潜在的并发问题。
    }
}

void EquipmentController::initialize() {
    if (m_motor) {
        m_motor->initialize();
        //这里的initialize是IMotor接口的函数，调用了具体实现类MockMotor的initialize函数，模拟电机的初始化过程。
    }
    m_state = EquipmentState::Idle;
}

bool EquipmentController::startProcess() {
    EquipmentState expected = EquipmentState::Idle;
    // 只有 Idle 状态才能启动。CAS (Compare-And-Swap) 不是必须的，但在这演示严谨的状态检查
    // CAS是无锁状态机，能够防止并发连击状况，是CPU底层指令级别的原子操作，能够保证在多线程环境下对共享变量的安全访问和修改，避免了传统锁机制带来的性能损耗和死锁风险。
    // 如果用简单的if进行判断EquipmentState::Idle，可能会面临并发连击的情况：两个线程同时检查到状态是Idle，都通过了检查，然后都把状态改成Running，导致状态机进入了一个不合法的状态。
    if (m_state.compare_exchange_strong(expected, EquipmentState::Running)) {
        //如果一样就改成Running并返回true，如果不一样就把当前值写入expected并返回false，这样就能保证只有一个线程能够成功地将状态从Idle改成Running，
        //其他线程会发现状态已经被修改了，就不会进入这个流程，从而避免了并发连击导致的状态混乱。
        std::cout << "[EquipmentController] Process Started.\n";
        
        {
            std::lock_guard<std::mutex> lock(m_cv_mutex);//在当前{}内作用，}结束后自动解锁
            m_stop_requested = false;//每次启动流程都重置这个标志位，确保上一次的打断请求不会影响到新的流程运行
            //mutex是为了互斥，防止乱改数据，而后面的m_cv是为了同步，唤醒后台线程，是一个通信职责，是线程之间的信号灯
            m_process_active = true;
        }
        // 唤醒后台线程去干活，这里应理解为主线程修改了m_process_active的状态，告诉后台线程“嘿，工艺流程开始了，你可以开始干活了”，而m_cv.notify_all()就是发出这个信号的动作，确保后台线程能够及时地收到这个信号并做出响应，开始执行工艺流程。
        m_cv.notify_all(); 
        return true;
    }
    std::cout << "[EquipmentController] Cannot start. Current state: " << getStateString() << "\n";
    return false;
}

void EquipmentController::stopProcess(){
    //幂等性设计，重复调用stopProcess不会有副作用，保证了即使在工艺流程已经停止的情况下再次调用stopProcess也不会导致错误或异常。
    {
        std::lock_guard<std::mutex> lock(m_cv_mutex);
        m_stop_requested = true;//设置打断请求标志位，告诉后台线程“嘿，用户叫停了，你赶紧停下来”，这个标志位的意义在于能够让后台线程在任何时候都能够检查到用户的打断请求，从而实现真正的可打断流程控制。
        m_process_active = false;//无论当前流程是否在跑，都把这个标志位设置为false，确保流程状态的一致性和正确性，避免了流程状态的
    }
    if(m_motor) {
        m_motor->stop();
    }
    m_cv.notify_all(); //唤醒后台线程让它检查m_stop_requested标志位，及时响应用户的打断请求，停止当前的工艺流程。
    std::cout << "[EquipmentController] Stop requested.\n";

}

/*void EquipmentController::stopProcess() {
    if (m_state == EquipmentState::Running) {
        std::cout << "[EquipmentController] Stopping Process...\n";
        // 修改标志位
        {
            std::lock_guard<std::mutex> lock(m_cv_mutex);
            m_process_active = false;
        }
        // 重点：立刻唤醒正在 wait_for 的线程！
        m_cv.notify_all();
        
        if (m_motor) {
            m_motor->stop();
        }
        m_state = EquipmentState::Idle;
    }
}*/

EquipmentState EquipmentController::getState() const {
    return m_state;
}

std::string EquipmentController::getStateString() const {
    switch (m_state) {
        case EquipmentState::Idle: return "Idle";
        case EquipmentState::Running: return "Running";
        case EquipmentState::Error: return "Error";
        default: return "Unknown";
    }
}

void EquipmentController::workerLoop(){//在修改了电机非阻塞运转后对应的设备控制器
    constexpr double kTargetPosition = 100.0;
    constexpr double kHomePos = 0.0;
    constexpr double kPosEps = 0.1;//

}

void EquipmentController::workerLoop() {//核心后台线程函数，负责执行工艺流程的具体步骤，并且在每个步骤之间检查是否有停止命令，以实现可打断的流程控制。
    while (m_running) {
        // 等待任务开始
        std::unique_lock<std::mutex> lock(m_cv_mutex);
        
        // 谓词防止虚假唤醒。
        m_cv.wait(lock, [this] {
            return !m_running || m_process_active; 
        });//条件变量的使用，wait()函数接受一个锁和一个谓词作为参数，线程会在这个条件变量上等待，直到被唤醒并且满足谓词条件才会继续执行。这里的谓词检查了两个条件：如果m_running为false（表示线程应该停止），或者m_process_active为true（表示有工艺流程需要执行），线程就会被唤醒并继续执行后续的工艺流程步骤。

        if (!m_running) break; // 析构退出

        // --- 开始业务流程 ---
        // 模拟一个需要长时间的动作：电机移动 -> 处理 -> 电机归位
        
        std::cout << "[Worker] Step 1: Moving Motor to 100.0...\n";
        // 这里只是发指令，假设 IMotor 还是阻塞的，我们这就会卡住。
        // 但我们要演示的是“两步之间”的可打断等待。
        // 假设电机移动很快，或者我们在移动前check。
        if(!m_process_active) { m_state = EquipmentState::Idle; continue; }
        
        // 模拟调用电机 (这里用的是阻塞 MockMotor，这其实会阻塞线程，
        // 真正的非阻塞电机应该立即返回)
        m_motor->moveToPosition(100.0);

        // --- 关键点：模拟 Process Time (例如 2秒)，但要求可打断 ---
        std::cout << "[Worker] Step 2: Processing (Wafer Etching) for 2s...\n";
        
        // 这里的 wait_for 替代了 sleep_for
        // 如果 timeout (2s) 到了，返回 false (timeout)，继续流程。
        // 如果 m_process_active 变成 false (被 stopProcess 叫停)，返回 true。
        bool stopped = m_cv.wait_for(lock, std::chrono::seconds(2), [this] {
            return !m_process_active; //第二个参数的含义是超时时间，即使没有信号打断，在该时间也会自动醒来，也就是模拟工艺正常运行时间
            //当超时并且过程中没有被打断说明工艺正常运行结束，如果在超时之前被打断了，那么说明工艺被叫停了，返回true，进入打断流程
        });

        if (stopped) {
            std::cout << "[Worker] Process INTERRUPTED by Stop command!\n";
            // 清理动作...
            m_state = EquipmentState::Idle;
            continue; // 回到 loop 顶端等待
        }

        std::cout << "[Worker] Step 3: Moving Home...\n";
        m_motor->moveToPosition(0.0);

        std::cout << "[Worker] Process Complete.\n";
        m_process_active = false;
        m_state = EquipmentState::Idle;
    }
}