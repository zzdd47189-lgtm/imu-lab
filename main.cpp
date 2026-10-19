#include <iostream>
#include <thread>
#include <chrono>
#include<random>
int main()
{double ax=0.0;
double ay=0.0;
double az=9.81;
double sum_az=0.0;
int count=0;
int seq=0;
std::mt19937 gen(std::random_device{}());
std::normal_distribution<double> noise( 0.0,0.02);
auto start=std::chrono::steady_clock::now();
auto next=std::chrono::steady_clock::now();
while (true)
{sum_az+=az;
 auto now=std::chrono::steady_clock::now();
double elapsed= std::chrono::duration<double>(now-start).count();
std::cout<<seq<<" "<<elapsed<<" "<<ax<<" "<<ay<<" "<<az<<std::endl;
next+=std::chrono::milliseconds(100);
std::this_thread::sleep_until(next);
double n=noise(gen);
az+=n;
seq++;
count++;
if(count%10==0)
std::cout<<"sum_az="<<sum_az/count<<std::endl;
   
}
return 0;}
