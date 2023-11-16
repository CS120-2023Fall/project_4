#ifndef FILTER_H
#define FILTER_H
#include <iostream>
#include <vector>
#include <cmath>
#define M_PI acos(-1)
using namespace std;
// from https://blog.csdn.net/Q_Linfeng/article/details/129120537
// 低通滤波器类
class LowPassFilter {
public:
    LowPassFilter(double sample_rate, double cutoff_frequency) {
        double dt = 1.0 / sample_rate;
        double RC = 1.0 / (cutoff_frequency * 2.0 * M_PI);
        alpha_ = dt / (dt + RC);
        prev_output_ = 0.0;
    }

    // 更新滤波器输出
    double update(double input) {
        double output = alpha_ * input + (1.0 - alpha_) * prev_output_;
        prev_output_ = output;
        return output;
    }

private:
    double alpha_;
    double prev_output_;
};



#endif