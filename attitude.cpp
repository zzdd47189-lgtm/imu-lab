#include"attitude.h"
#include<cmath>
constexpr double pi=3.141592653589793;
double calculateroll(const acceldata& accel)
{
    double roll=std::atan2(accel.ay,accel.az)*(180.0/pi);
    return roll;
}
double calculatepitch(const acceldata& accel)
{
    double pitch=std::atan2(-accel.ax,std::sqrt(accel.ay*accel.ay+accel.az*accel.az))*(180.0/pi);
    return pitch;
}