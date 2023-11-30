#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/timerfd.h>
#include <unistd.h>

int main() {
    //int timerfd_create(clockid_t __clock_id, int __flags)
    //clock_id：CLOCK_REALTIME：系统范围的实时时钟   CLOCK_MONOTONIC：单调递增时钟
    //flags：0 或者 `TFD_NONBLOCK` 使文件描述符处于非阻塞模式。TFD_CLOEXEC` 在执行 `exec()` 系列函数时关闭文件描述符。
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0) {
        perror("timerfd_create error");
        return -1;
    }

    struct itimerspec itime;
    itime.it_value.tv_sec = 1;    // 第一次超时时间为1秒后
    itime.it_value.tv_nsec = 0;   // 没有额外纳秒
    itime.it_interval.tv_sec = 3; // 每次超时的时间间隔为3秒                 
    itime.it_interval.tv_nsec = 0;// 每次间隔中没有额外纳秒 

    //int timerfd_settime(int __ufd, int __flags, const itimerspec *__utmr, itimerspec *__otmr)
    //flags设置为0 表示__utmr为相对时间，TFD_TIMER_ABSTIME标志，`__utmr` 指定的时间会被看作是绝对时间
    timerfd_settime(timerfd, 0, &itime, NULL);

    //当定时器超时时，系统会向 timerfd 中写入一个8字节的数据，表示一次超时事件的发生。通过调用 read() 函数来读取 timerfd，可以获取该超时事件的次数。

    while (true) {
        uint64_t times;
        //定时器每次超时都会向 timerfd 写入一个 8 字节的数据，这个数据代表一次超时事件的发生。通过反复调用 read()函数，每次读取 8 字节的数据，就可以获得最近一次超时事件的次数。 read函数是阻塞的, 所以通过循环调用read函数，就表示两次read函数之间的超时次数,read读取timerfd文件描述符中的8字节内容(也就是超时次数),timerfd描述符的内容就被清空重置,再次重新计算超时次数
        int ret = read(timerfd, &times, 8);
        if (ret < 0) {
            perror("read error");
            return -1;
        }

        printf("超时了，距离上一次超时了%ld次\n", times);
        sleep(10);
    }

    close(timerfd);
    return 0;
}