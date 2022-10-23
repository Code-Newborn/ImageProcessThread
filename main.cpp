#include <iostream>
#include <Windows.h>
#include <thread>             // 提供线程类
#include <mutex>              // 提供互斥锁类(对部分和进行累加的时候需要加锁)，很好用
#include <opencv2/opencv.hpp> // 提供imread读取图片函数

using namespace cv;
using namespace std;

mutex mtx;     // 定义一个互斥锁
long totalSum; // 总和 公共变量
const enum RangeSpecify { LEFT_UP,
                          LEFT_DOWN,
                          RIGHT_UP,
                          RIGHT_DOWN };

/// @brief      截取图像的部分区域并进行计算
/// @msg        打印该过程的时间
/// @param      {Mat} &img 输入图像
/// @param      {enum RangeSpecify} r 截取的区域
/// @return     {*}
void ImageAverage(Mat &img, enum RangeSpecify r) // 线程代码
{
    int startRow, startCol, endRow, endCol;
    switch (r)
    {
    case LEFT_UP:
        startRow = 0;
        endRow = img.rows / 2;
        startCol = 0;
        endCol = img.cols / 2;
        break;
    case LEFT_DOWN:
        startRow = img.rows / 2;
        endRow = img.rows;
        startCol = 0;
        endCol = img.cols / 2;
        break;
    case RIGHT_UP:
        startRow = 0;
        endRow = img.rows / 2;
        startCol = img.cols / 2;
        endCol = img.cols;
        break;
    case RIGHT_DOWN:
        startRow = img.rows / 2;
        endRow = img.rows;
        startCol = img.cols / 2;
        endCol = img.cols;
        break;
    }

    LARGE_INTEGER t_start, t_end, tc;
    QueryPerformanceFrequency(&tc); // 获取系统频率

    long sum = 0;
    QueryPerformanceCounter(&t_start); // 开始计时
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = startCol; j < endCol; j++)
        {
            sum += img.at<unsigned char>(i, j);
        }
    }
    mtx.lock(); // 在访问公共变量totalSum 之前对其进行加锁
    totalSum += sum;
    mtx.unlock(); // 访问完毕立刻解锁

    QueryPerformanceCounter(&t_end); // 计时结束

    cout << "Region" << r << "Total : " << sum << endl;
    cout << "task completed! Time elapsed " << (t_end.QuadPart - t_start.QuadPart) * 1000000.0 / tc.QuadPart << endl; // 打印本次线程时间花费时间 单位微秒
}

int main(int, char **)
{
    Mat src = imread("../Camera1.png", cv::IMREAD_GRAYSCALE);
    LARGE_INTEGER t_start, t_end, tc;
    QueryPerformanceFrequency(&tc); // 获取系统频率
    cout << "Frequency: " << tc.QuadPart << endl;
    QueryPerformanceCounter(&t_start);     // 开始计时
    thread t0(ImageAverage, src, LEFT_UP); // 开辟线程，并输入函数参数
    thread t1(ImageAverage, src, LEFT_DOWN);
    thread t2(ImageAverage, src, RIGHT_UP);
    thread t3(ImageAverage, src, RIGHT_DOWN);

    t0.join(); // 等待子线程t0执行完毕
    t1.join();
    t2.join();
    t3.join();

    QueryPerformanceCounter(&t_end); // 结束计时

    cout << endl
         << "With MultiThread Time Elapse：" << (t_end.QuadPart - t_start.QuadPart) * 1000000.0 / tc.QuadPart << endl; // 打印本次线程时间花费时间 单位微秒
    cout << "Average Calculate Result： " << totalSum * 1.0 / (src.cols * src.rows) << endl
         << endl;

    // 验证准确性
    long sum = 0;

    QueryPerformanceCounter(&t_start); // 开始计时
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            sum += src.at<unsigned char>(i, j);
        }
    }
    QueryPerformanceCounter(&t_end); // 结束计时

    cout << "Without MultiThread Time Elapse：" << (t_end.QuadPart - t_start.QuadPart) * 1000000.0 / tc.QuadPart << endl; // 打印本次线程时间花费时间 单位微秒
    cout << "Average Calculate Result： " << sum * 1.0 / (src.rows * src.cols) << endl
         << endl;

    return 0;
}

// 多线程花费时间竞比单线程花费时间长，猜测可能是图片较小，在大尺寸图片才能体现多线程优势