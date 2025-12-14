#include <iostream>
#include <windows.h>
#include <string>
#include <chrono>
#include <ctime>
#include <fwpmu.h>
#include <fstream>
#include <iomanip> 
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>

#pragma warning(push)
#pragma warning(disable: 4996) // disable warning about unsafe functions

#pragma comment (lib, "fwpuclnt.lib")
#pragma comment(lib, "Advapi32")

std::ofstream RithLog;

std::string get_time() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    return ss.str();
}

void error_logging(std::string error_value) {
    std::string l_time = get_time();

    RithLog.open("C:/pire/Rith.txt", std::ios::app);
    RithLog << l_time << ", " << error_value << std::endl;

    RithLog.close();
}

bool IsRunAsAdministrator() {
    BOOL isAdmin = FALSE;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    return isAdmin;
}

wchar_t* GetWC(const char* c) // convert char* -> wchar_t*
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}

void run_lags() {
    HANDLE engineHandle;
    if (DWORD result = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &engineHandle)) {
        std::cout << "Error 2";
    }
    FWPM_FILTER0 filter;
    SecureZeroMemory(&filter, sizeof(filter));
    FWPM_FILTER_CONDITION0 conditions[2];

    conditions[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
    conditions[0].conditionValue.type = FWP_UINT8;
    conditions[0].conditionValue.uint8 = 17;
    conditions[0].matchType = FWP_MATCH_EQUAL;

    conditions[1].fieldKey = FWPM_CONDITION_DIRECTION;
    conditions[1].conditionValue.type = FWP_UINT32;
    conditions[1].conditionValue.uint32 = FWP_DIRECTION_OUTBOUND;
    conditions[1].matchType = FWP_MATCH_EQUAL;

    filter.action.type = FWP_ACTION_BLOCK;
    filter.displayData.name = GetWC("UpdPause");
    filter.layerKey = FWPM_LAYER_DATAGRAM_DATA_V4;
    filter.numFilterConditions = 2;
    filter.weight.type = FWP_EMPTY;
    filter.filterCondition = conditions;

    int pressCount = 0;

    while (true) {
        bool isPressed = (GetKeyState(VK_CAPITAL) & 0x8000) != 0;

        if (isPressed == 1) {
            std::string time = get_time();
            pressCount++;

            if (pressCount % 2 == 1) {
                if (DWORD result = FwpmFilterAdd0(engineHandle, &filter, NULL, &filter.filterId)) {
                    error_logging("error 2");
                }
                else {
                    Beep(1000, 100);
                    std::cout << time << " : " << "activate" << std::endl;
                }
            }
            else {
                if (DWORD result = FwpmFilterDeleteById0(engineHandle, filter.filterId)) {
                    error_logging("error 3");
                }
                else {
                    Beep(1000, 200);
                    std::cout << time << " : " << "disable" << std::endl << std::endl;
                }
            }
        }

        Sleep(10);
    }
}

int main() {
    if (!IsRunAsAdministrator()) {
        error_logging("Run as administrator.");
        MessageBox(NULL, L"Run as administrator.", L"error 1", MB_ICONINFORMATION | MB_OK);

        return 1;
    }
    else {
        SetConsoleTitle(L"PireWare");

        struct stat sb;

        if ((stat("C:/pire", &sb)) != 0) {
            std::filesystem::current_path(std::filesystem::temp_directory_path());
            std::filesystem::create_directory("C:/pire");
        }
        std::cout << "successfully launched" << std::endl << std::endl;

        run_lags();
    }
}