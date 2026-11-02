#include <iostream>
#include <thread>
#include <chrono>
#include<random>
#include<fstream>
#include<cmath>
#include"imu_types.h"
#include"attitude.h"
constexpr double pi=3.141592653589793;
int main()
{
double sum_az=0.0;
int count=0;
int seq=0;
double roll_gyro=0.0;//陀螺仪积分角度
double roll_fused=0.0;//x互补滤波角度
double pitch_fused=0.0;//y互补滤波角度
double previous_time = 0.0;
double previous_gx=10.1;
double previous_gy=5.1;
double pitch_gyro=0;
std::ofstream file("imu_data.csv");
file << "seq,time,ax,ay,az\n";
std::mt19937 gen(std::random_device{}());
std::normal_distribution<double> noise( 0.0,0.02);
auto start=std::chrono::steady_clock::now();
auto next=std::chrono::steady_clock::now();
while (true)
{auto now=std::chrono::steady_clock::now();
double elapsed= std::chrono::duration<double>(now-start).count();
double dt = elapsed - previous_time;
previous_time=elapsed;

  double roll_asc=10*elapsed;//理想的角度变化
  if(roll_asc>90)
    roll_asc=90;
double rad=roll_asc*(pi/180);  

double roll_pitch=5*elapsed;
if(roll_pitch>=45)
    roll_pitch=45;
double rad_pitch=roll_pitch*(pi/180);
acceldata accel{};
accel.ax=-9.81*std::sin(rad_pitch);
 accel.ay=9.81*std::sin(rad)*std::cos(rad_pitch);
accel.az=9.81*std::cos(rad)*std::cos(rad_pitch);

double n=noise(gen);
accel.az+=n;//噪声

double roll_acc=calculateroll(accel);//加速度计的测量绕x轴角度
double pitch_acc=calculatepitch(accel);//加速度计的测量绕y轴角度

gyrodata gyro{};
if(elapsed<9.0)
gyro.gx=10.0;
if(elapsed>=9.0)
gyro.gx=0.0;
gyro.gx+=0.1; //x零偏移
if(elapsed<9.0)
gyro.gy=5.0;
if(elapsed>=9.0)
gyro.gy=0.0;
gyro.gy+=0.1;//y零偏移
roll_gyro+=previous_gx*dt;//陀螺仪的角度
previous_gx=gyro.gx;
roll_fused=0.98*(roll_fused+dt*gyro.gx)+0.02*roll_acc;

pitch_gyro+=previous_gy*dt;
previous_gy=gyro.gy;


pitch_fused=0.98*(pitch_fused+dt*gyro.gy)+0.02*pitch_acc;
file << seq << "," << elapsed << "," << accel.ax << "," << accel.ay << "," << accel.az << "\n";
file.flush();
std::cout<<"seq="<<seq<<" "<<"time="<<elapsed<<" "<<"ax="<<accel.ax<<" "<<"ay="<<accel.ay<<" "<<"az="
<<accel.az<<" "<<"roll_acc="<<roll_acc<<" "<<"gyro.gx="<<gyro.gx<<" "
<<"roll_gyro="<<roll_gyro<<" "<<"roll_fused="<<roll_fused<<" "<<"dt="<<dt<<" "<<std::endl;

std::cout<<"ax_pitch="<<accel.ax<<" "<<"az_pitch="<<accel.az<<" "<<"pitch_acc="<<pitch_acc
<<" "<<"gyro.gy="<<gyro.gy<<" "<<"pitch_gyro="<<pitch_gyro
<<" "<<"pitch_fused="<<pitch_fused<<std::endl;

next+=std::chrono::milliseconds(100);
std::this_thread::sleep_until(next);
sum_az+=accel.az;
seq++;
count++;
if(count%10==0)
std::cout<<"sum_az="<<sum_az/count<<std::endl;
}
return 0;}
