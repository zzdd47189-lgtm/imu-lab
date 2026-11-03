#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

int main()
{
    // 1. 打开串口
    HANDLE port = CreateFileA(
        "\\\\.\\COM3",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (port == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Cannot open COM3. Error: "
                  << GetLastError() << '\n';
        return 1;
    }

    // 2. 读取并修改串口配置
    DCB config{};
    config.DCBlength = sizeof(config);

    if (!GetCommState(port, &config))
    {
        std::cerr << "Cannot read serial settings.\n";
        CloseHandle(port);
        return 1;
    }

    config.BaudRate = CBR_115200;
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;

    config.fBinary = TRUE;
    config.fParity = FALSE;

    config.fOutxCtsFlow = FALSE;
    config.fOutxDsrFlow = FALSE;
    config.fDsrSensitivity = FALSE;
    config.fOutX = FALSE;
    config.fInX = FALSE;
    config.fDtrControl = DTR_CONTROL_DISABLE;
    config.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(port, &config))
    {
        std::cerr << "Cannot configure serial port.\n";
        CloseHandle(port);
        return 1;
    }

    // 3. 设置串口读取超时
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;

    if (!SetCommTimeouts(port, &timeouts))
    {
        std::cerr << "Cannot set serial timeout.\n";
        CloseHandle(port);
        return 1;
    }

    // 4. 打开数据文件，追加保存
    std::ofstream data_file(
        "imu_data.csv",
        std::ios::app | std::ios::ate
    );

    if (!data_file.is_open())
    {
        std::cerr << "Cannot open data file.\n";
        CloseHandle(port);
        return 1;
    }

    if (data_file.tellp() == 0)
    {
        data_file << "time_ms,roll_deg,pitch_deg,alarm\n";
    }

    // 5. 打开报警事件文件
    std::ofstream event_file(
        "alarm_events.csv",
        std::ios::app | std::ios::ate
    );

    if (!event_file.is_open())
    {
        std::cerr << "Cannot open event file.\n";
        CloseHandle(port);
        return 1;
    }

    if (event_file.tellp() == 0)
    {
        event_file
            << "time_ms,event,roll_deg,pitch_deg,duration_ms\n";
    }

    std::cout << "Connected to COM3. Press Ctrl+C to stop.\n";

    std::string line;
    bool discard_line = false;

    int previous_alarm = 0;              // 上一次报警状态
    unsigned long alarm_start_ms = 0;    // 报警开始时间
    bool start_known = false;           // 是否观察到了报警开始

    bool first_sample = true;           // 是否为第一条有效数据
    unsigned long previous_time_ms = 0; // 上一次数据的时间

    // 6. 持续读取串口
    while (true)
    {
        char ch;
        DWORD received = 0;

        if (!ReadFile(port, &ch, 1, &received, nullptr))
        {
            std::cerr << "Serial read failed. Error: "
                      << GetLastError() << '\n';
            break;
        }

        // 超时没有收到数据，继续等待
        if (received == 0)
        {
            continue;
        }

        // 收到换行符，处理完整的一行
        if (ch == '\n')
        {
            if (!discard_line)
            {
                unsigned long time_ms = 0;
                int roll = 0;
                int pitch = 0;
                int alarm = 0;

                int count = std::sscanf(
                    line.c_str(),
                    "%lu,%d,%d,%d",
                    &time_ms,
                    &roll,
                    &pitch,
                    &alarm
                );

                if (count == 4 && (alarm == 0 || alarm == 1))
                {
                    // 7. 显示数据
                    std::cout << "Time=" << time_ms << "ms"
                              << "  Roll=" << roll
                              << "  Pitch=" << pitch
                              << "  Alarm=" << alarm
                              << '\n' << std::flush;

                    // 8. 保存每条有效数据
                    data_file << time_ms << ','
                              << roll << ','
                              << pitch << ','
                              << alarm << '\n';

                    data_file.flush();

                    if (!data_file)
                    {
                        std::cerr << "Failed to write data file.\n";
                        break;
                    }

                    // 9. 时间变小：重新建立状态
                    // 例如 STM32 复位后，计时会重新开始
                    if (!first_sample && time_ms < previous_time_ms)
                    {
                        event_file << time_ms << ",TIME_RESTART,"
                                   << roll << ',' << pitch << ",\n";

                        first_sample = true;
                        start_known = false;
                    }

                    // 10. 第一条有效数据只用于确定当前状态
                    if (first_sample)
                    {
                        if (alarm == 1)
                        {
                            // 连接时已经报警，无法确定开始时间
                            event_file << time_ms << ",ACTIVE_AT_CONNECT,"
                                       << roll << ',' << pitch << ",\n";
                        }

                        first_sample = false;
                    }
                    // 11. 从不报警变成报警
                    else if (previous_alarm == 0 && alarm == 1)
                    {
                        alarm_start_ms = time_ms;
                        start_known = true;

                        event_file << time_ms << ",START,"
                                   << roll << ',' << pitch << ",\n";
                    }
                    // 12. 从报警变成不报警
                    else if (previous_alarm == 1 && alarm == 0)
                    {
                        event_file << time_ms << ",END,"
                                   << roll << ',' << pitch << ',';

                        // 知道开始时间，才能计算持续时间
                        if (start_known)
                        {
                            event_file << time_ms - alarm_start_ms;
                        }

                        event_file << '\n';
                        start_known = false;
                    }

                    // 判断完成后，再保存本次状态
                    previous_alarm = alarm;
                    previous_time_ms = time_ms;

                    event_file.flush();

                    if (!event_file)
                    {
                        std::cerr << "Failed to write event file.\n";
                        break;
                    }
                }
            }

            // 准备接收下一行
            line.clear();
            discard_line = false;
        }
        else if (ch != '\r' && !discard_line)
        {
            if (line.size() < 128)
            {
                line += ch;
            }
            else
            {
                // 超长行一直丢弃到换行符
                discard_line = true;
            }
        }
    }

    CloseHandle(port);
    return 0;
}