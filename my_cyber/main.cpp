
#define GLOG_USE_GLOG_EXPORT
#include "glog/logging.h"
#include "common/log.h"
#include "src/logger/async_logger.h"
#include <iostream>

int main(int argc, char *argv[])
{
    // 初始化Google Logging模块
    google::InitGoogleLogging(argv[0]);

    // 设置日志的输出等级
    FLAGS_stderrthreshold = google::ERROR; // 错误及以上的日志输出到标准错误
    FLAGS_colorlogtostderr = true;         // 日志输出带颜色
    FLAGS_logtostderr = false;             // 非错误日志不直接输出到标准错误

    // 设置日志文件的大小
    FLAGS_max_log_size = 10; // 每个日志文件最大10MB

    // 设置日志文件的路径
    google::SetLogDestination(google::INFO, "./");
    google::SetLogDestination(google::WARNING, "./");
    google::SetLogDestination(google::ERROR, "./");
    google::SetLogDestination(google::FATAL, "./");

    // 记录日志
    LOG(INFO) << "This is an info log message.";
    // LOG(WARNING) << "This is a warning log message.";
    // LOG(ERROR) << "This is an error log message.";

    // 以下代码不会执行，因为遇到FATAL日志会导致程序终止
    // LOG(FATAL) << "This is a fatal log message. Program will terminate.";

    // Init async logger
    auto async_logger = new ::apollo::cyber::logger::AsyncLogger(
        google::base::GetLogger(google::GLOG_INFO));
    google::base::SetLogger(google::GLOG_INFO, async_logger);
    async_logger->Start();

    while (true)
    {

        AWARN << " prio great than MAX_PRIO.";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}

// time_t 与 std::chrono::system_clock::time_point互转
//  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
//  time_t t = std::chrono::system_clock::to_time_t(tp);

// std::chrono::system_clock::time_point longIntToTimePoint(long int seconds) {
//     return std::chrono::system_clock::from_time_t(seconds);
// }

// // 使用示例
// int main() {
//     long int seconds = 1234567890;  // 示例秒数
//     std::chrono::system_clock::time_point tp = longIntToTimePoint(seconds);

//     // 输出结果
//     std::cout << "Time point: " << std::chrono::system_clock::to_time_t(tp) << std::endl;
//     return 0;
// }