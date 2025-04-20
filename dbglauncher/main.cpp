#include <iostream>
#include <windows.h>

#pragma comment(lib, "advapi32.lib")

// для справки https://anti-debug.checkpoint.com/techniques/interactive.html#self-debugging

int main()
{
    // 1. Запустить password manager и получить его PID
    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    wchar_t cmdLine[] = L"C:/Users/user/Documents/passwordManager/build/Desktop_Qt_6_8_2_MSVC2022_64bit-Debug/passwordManager.exe";

    if(CreateProcessW(  // Используем явно Unicode-версию функции
            NULL,       // Имя приложения (NULL, так как используем командную строку)
            cmdLine,    // Командная строка
            NULL,       // Атрибуты безопасности процесса
            NULL,       // Атрибуты безопасности потока
            FALSE,      // Наследование дескрипторов
            DEBUG_ONLY_THIS_PROCESS, // Флаги создания - отлаживаем только этот процесс
            NULL,       // Окружение (используем родительское)
            NULL,       // Текущий каталог (используем родительский)
            &si,
            &pi))
    {
        std::cout << "***CreateProcessW() success!" << std::endl;
        std::cout << "***CreateProcessW() pid = " << std::dec << pi.dwProcessId << std::endl;
    }else{
        std::cout << "***CreateProcessW() FAILED" << std::endl;
    }

    // 2. Подключиться к процессу как отладчик
    // https://learn.microsoft.com/en-us/windows/win32/debug/writing-the-debugger-s-main-loop

    bool isAttached = DebugActiveProcess(pi.dwProcessId);
    if(!isAttached){// проверка, удалось ли подключиться
        DWORD lastError = GetLastError();
        std::cout << std::hex << "***DebugActiveProcess() FAILED, GetLastError() = " << lastError;
    }
    else{
        std::cout << "***DebugActiveProcess() success!" << std::endl;
    }

    // 3. Пропускать поступающие сигналы отладки

    DEBUG_EVENT debugEvent;
    while(true){
        bool result1 = WaitForDebugEvent (&debugEvent, INFINITE);
        bool result2 = ContinueDebugEvent(debugEvent.dwProcessId,
                                          debugEvent.dwThreadId,
                                          DBG_CONTINUE);
    }

    return 0;
}
