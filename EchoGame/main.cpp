#include "Application.h"

#include <exception>
#include <Windows.h>

int main()
{
    try
    {
        Echo::Application application;

        return application.Run();
    }
    catch (const std::exception& error)
    {
        MessageBoxA(
            nullptr,
            error.what(),
            "EchoGame error",
            MB_OK | MB_ICONERROR
        );

        return 1;
    }
}