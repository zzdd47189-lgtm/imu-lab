#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio>

int main()
{
    // 1. 打开 COM3
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

    // 2. 读取串口当前配置
    DCB config{};
    config.DCBlength = sizeof(config);

    if (!GetCommState(port, &config))
    {
        std::cerr << "Cannot read serial settings.\n";
        CloseHandle(port);
        return 1;
    }

    // 3. 修改串口配置，与 STM32 保持一致
    config.BaudRate = CBR_115200;
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;

    config.fBinary = TRUE;
    config.fParity = FALSE;

    // 不使用硬件或软件流量控制
    config.fOutxCtsFlow = FALSE;
    config.fOutxDsrFlow = FALSE;
    config.fDsrSensitivity = FALSE;
    config.fOutX = FALSE;
    config.fInX = FALSE;
    config.fDtrControl = DTR_CONTROL_DISABLE;
    config.fRtsControl = RTS_CONTROL_DISABLE;

    // 4. 应用串口配置
    if (!SetCommState(port, &config))
    {
        std::cerr << "Cannot configure serial port.\n";
        CloseHandle(port);
        return 1;
    }

    // 5. 设置读取超时，单位为毫秒
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;

    if (!SetCommTimeouts(port, &timeouts))
    {
        std::cerr << "Cannot set serial timeout.\n";
        CloseHandle(port);
        return 1;
    }

    std::cout << "Connected to COM3. Press Ctrl+C to stop.\n";

    // 保存逐个接收到的字符
    std::string line;

    // 防止异常数据一直没有换行，导致字符串无限增长
    bool discard_line = false;

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

        // 本次超时，没有收到数据
        if (received == 0)
        {
            continue;
        }

        // 7. 遇到换行符，处理完整的一行
        if (ch == '\n')
        {
            unsigned long time_ms = 0;
            int roll = 0;
            int pitch = 0;
            int alarm = 0;

            if (!discard_line)
            {
                int count = std::sscanf(
                    line.c_str(),
                    "%lu,%d,%d,%d",
                    &time_ms,
                    &roll,
                    &pitch,
                    &alarm
                );

                if (count == 4)
                {
                    std::cout << "Time=" << time_ms << "ms"
                              << "  Roll=" << roll
                              << "  Pitch=" << pitch
                              << "  Alarm=" << alarm
                              << '\n' << std::flush;
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
                // 超长行丢弃到下一个换行符为止
                discard_line = true;
            }
        }
    }

    // 8. 退出循环后关闭串口
    CloseHandle(port);
    return 0;
}