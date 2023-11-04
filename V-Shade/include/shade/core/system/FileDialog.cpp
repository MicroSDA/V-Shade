#include "shade_pch.h"
#include "FileDialog.h"

#ifdef SHADE_WINDOWS_PLATFORM
#include <shade/platforms/system/windows/WindowsFileDialog.h>
#endif // !SHADE_WINDOWS_PLATFORM

std::filesystem::path shade::FileDialog::OpenFile(const char* filter)
{
#ifdef SHADE_WINDOWS_PLATFORM
    return WindowsFileDialog::OpenFile(filter);
#endif // !SHADE_WINDOWS_PLATFORM 

    std::filesystem::path();
}

std::filesystem::path shade::FileDialog::SaveFile(const char* filter)
{
#ifdef SHADE_WINDOWS_PLATFORM
    return WindowsFileDialog::SaveFile(filter);
#endif // !SHADE_WINDOWS_PLATFORM 

    std::filesystem::path();
}

std::filesystem::path shade::FileDialog::SelectFolder(const char* filter)
{
#ifdef SHADE_WINDOWS_PLATFORM
    return WindowsFileDialog::SelectFolder(filter);
#endif // !SHADE_WINDOWS_PLATFORM 

    std::filesystem::path();
}
