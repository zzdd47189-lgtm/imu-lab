#include <iostream>
#include <thread>
#include <chrono>
#include<random>
#include<fstream>
#include<cmath>
int main()
{constexpr double pi=3.141592653589793;
double ax=0.0;
double ay=0.0;
double az=0.0;
double sum_az=0.0;
int count=0;
int seq=0;
double gx;
double roll_gyro=0.0;
double roll_fused=0.0;
std::ofstream file("imu_data.csv");
file << "seq,time,ax,ay,az\n";
std::mt19937 gen(std::random_device{}());
std::normal_distribution<double> noise( 0.0,0.02);
auto start=std::chrono::steady_clock::now();
auto next=std::chrono::steady_clock::now();
while (true)
{
 auto now=std::chrono::steady_clock::now();
double elapsed= std::chrono::duration<double>(now-start).count();
  double roll_asc=10*elapsed;//理想的角度变化
  if(roll_asc>90)
    roll_asc=90;
double rad=roll_asc*(pi/180);  
 ay=9.81*std::sin(rad);
az=9.81*std::cos(rad);
double n=noise(gen);
az+=n;//噪声
double roll_acc=std::atan2(ay,az)*(180.0/pi);//加速度计的测量角度
if(elapsed<9.0)
gx=10.0;
if(elapsed>=9.0)
gx=0.0;
gx+=0.1; //零偏移
roll_gyro+=gx*0.1;//陀螺仪的角度
roll_fused=0.98*(roll_fused+0.1*gx)+0.02*roll_acc;
file << seq << "," << elapsed << "," << ax << "," << ay << "," << az << "\n";
file.flush();
std::cout<<"seq="<<seq<<" "<<"time="<<elapsed<<" "<<"ax="<<ax<<" "<<"ay="<<ay<<" "<<"az="<<az<<" "<<"roll_acc="<<roll_acc<<" "<<"gx="<<gx<<" "<<"roll_gyro="<<roll_gyro<<" "<<"roll_fused="<<roll_fused<<std::endl;
next+=std::chrono::milliseconds(100);
std::this_thread::sleep_until(next);
sum_az+=az;
seq++;
count++;
if(count%10==0)
std::cout<<"sum_az="<<sum_az/count<<std::endl;
}
return 0;}
