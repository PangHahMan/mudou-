#include <functional>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;
//定时任务
class TimerTask {
private:
    uint64_t _id;           // 定时器任务对象ID
    uint32_t _timeout;      // 定时任务的超时时间
    bool _canceled;         // false-表示没有被取消， true-表示被取消
    TaskFunc _task_callback;// 定时器对象要执行的定时任务
    ReleaseFunc _release;   // 用于删除TimerWheel中保存的定时器对象信息
public:
    TimerTask(uint64_t id, uint32_t timeout, const TaskFunc &task_callback)
        : _id(id), _timeout(timeout), _task_callback(task_callback), _canceled(false) {}

    ~TimerTask() {
        if (_canceled == false) {
            _task_callback();
        }
        _release();
    }

    void SetRelease(const ReleaseFunc &release) {
        _release = release;
    }

    uint32_t GetDelayTime() {
        return _timeout;
    }

    void Cancel() {
        _canceled = true;
    }
};

class TimerWheel {
    using PtrTask = std::shared_ptr<TimerTask>;
    using WeakTask = std::weak_ptr<TimerTask>;

private:
    int _tick;    //当前的秒针，走到哪里释放哪里，释放哪里，就相当于执行哪里的任务
    int _capacity;// 表盘最大数量 -- 就是最大延迟时间
    std::vector<std::vector<PtrTask>> _wheel;
    //std::weak_ptr 不会增加对象的引用计数。这意味着 _timers 中的元素不会影响 TimerTask 实例的生命周期。TimerTask 实例的生命周期只由 _wheel 中的 std::shared_ptr 所控制，这保证了定时器任务可以独立于 _timers 映射而被析构。当 TimerTask 通过 _wheel 中的 std::shared_ptr 被销毁时，相应的 std::weak_ptr 会自动变为空（expired），这样我们就可以通过检查 std::weak_ptr 的状态来知道对应的 TimerTask 是否还存在。
    std::unordered_map<uint64_t, WeakTask> _timers;

private:
    void RemoveTimer(uint64_t id) {
        auto it = _timers.find(id);
        if (it != _timers.end()) {
            _timers.erase(it);
        }
    }

public:
    TimerWheel()
        : _capacity(60), _tick(0), _wheel(_capacity) {}
    //添加定时任务
    void TimerAdd(uint64_t id, uint32_t timeout, const TaskFunc &task_callback) {
        PtrTask pt(new TimerTask(id, timeout, task_callback));
        pt->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
        int pos = (_tick + timeout) % _capacity;
        _wheel[pos].push_back(pt);
        _timers[id] = WeakTask(pt);
    }
    //刷新/延迟定时任务
    void TimerRefresh(uint64_t id) {
        auto it = _timers.find(id);
        if (it == _timers.end()) {
            return;//没找到定时任务，没法刷新，没法延迟
        }

        PtrTask pt = it->second.lock();//lock获取weak_ptr管理的对象对应的shard_ptr
        int delay = pt->GetDelayTime();
        int pos = (_tick + delay) % _capacity;
        _wheel[pos].push_back(pt);
    }

    void TimerCancel(uint64_t id) {
        auto it = _timers.find(id);
        if (it == _timers.end()) {
            return;
        }

        PtrTask pt = it->second.lock();
        if (pt) {
            pt->Cancel();
        }
    }
    //这个函数应该每秒钟被执行一次，相等于秒钟向后走了一步
    void RunTimerTask() {
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear();//清空指定位置的数组，就会把数组中保存的所有管理对象的shared_ptr释放掉,就会执行对应的析构执行任务
    }
};

class Test {
public:
    Test() {
        std::cout << "构造" << std::endl;
    }
    ~Test() {
        std::cout << "析构" << std::endl;
    }
};

void DelTest(Test *t) {
    delete t;
}

int main() {
    TimerWheel tw;

    Test *t = new Test();

    tw.TimerAdd(888, 5, std::bind(DelTest, t));

    for (int i = 0; i < 5; i++) {
        tw.TimerRefresh(888);//刷新定时任务
        tw.RunTimerTask();   //向后移动秒针
        std::cout << "刷新了一下定时任务,重新需要5s中后才会销毁" << std::endl;
    }

    tw.TimerCancel(888);
    
    while (true) {
        sleep(1);
        std::cout << "---------------" << std::endl;
        tw.RunTimerTask();//向后移动秒针
    }
    return 0;
}