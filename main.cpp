#include <iostream>
#include <thread>
#include <chrono>
int main()
{double ax=0.0;
double ay=0.0;
double az=9.81;
int seq=0;
auto start=std::chrono::steady_clock::now();
auto next=std::chrono::steady_clock::now();
while (true)
{auto now=std::chrono::steady_clock::now();
double elapsed= std::chrono::duration<double>(now-start).count();
std::cout<<seq<<" "<<elapsed<<" "<<ax<<" "<<ay<<" "<<az<<std::endl;
next+=std::chrono::milliseconds(100);
std::this_thread::sleep_until(next);
az+=0.01;
seq++;
}
return 0;}
