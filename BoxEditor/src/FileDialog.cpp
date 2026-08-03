#include "FileDialog.h"
#include <vector>
#include <miniBoxLog.h>
#include <Windows.h>

std::string FileDialog::OpenTexture()
{
    
        OPENFILENAMEW ofn;
        // Wide buffer for file path
        std::vector<wchar_t> filename(MAX_PATH, L'\0');

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);

        // If you have a native HWND for the window, put it here; otherwise NULL is fine.
        ofn.hwndOwner = NULL;

        ofn.lpstrFile = filename.data();
        ofn.nMaxFile = static_cast<DWORD>(filename.size());

        // Double-null terminated wide-string filter (last \0 terminates the filter list)
        static const wchar_t filter[] =
            L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0"
            L"All Files\0*.*\0\0";
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;

        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;

        // Flags: require existing path/file, Explorer-style dialog
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

        if (GetOpenFileNameW(&ofn)) {
            // Convert selected wide string to UTF-8
            int required = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
            if (required > 0) {
                std::vector<char> utf8(required, 0);
                WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, utf8.data(), required, nullptr, nullptr);
                return std::string(utf8.data());
            }
            else {
                BOX_LOG_WARNING("openFileDialog: WideCharToMultiByte failed converting path.");
                return std::string();
            }
        }
        else {
            // If user cancelled, CommDlgExtendedError returns 0. Otherwise log the error code.
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                BOX_LOG_WARNING("openFileDialog: GetOpenFileNameW failed, CommDlgExtendedError=" << err);
            }
            return std::string();
        }
    

}

std::string FileDialog::OpenMBX(const char* defaultExt, const char* filter)
{
    OPENFILENAMEW ofn;
    // Wide buffer for file path
    std::vector<wchar_t> filename(MAX_PATH, L'\0');

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);

    // If you have a native HWND for the window, put it here; otherwise NULL is fine.
    ofn.hwndOwner = NULL;

    ofn.lpstrFile = filename.data();
    ofn.nMaxFile = static_cast<DWORD>(filename.size());

    // Double-null terminated wide-string filter (last \0 terminates the filter list)
    static const wchar_t mbxFilter[] =
        L"BoxEditor MBX Files (*.mbx)\0"
        L"*.mbx\0"
        L"All Files (*.*)\0"
        L"*.*\0\0";

    ofn.lpstrFilter = mbxFilter;

    ofn.lpstrDefExt = L"mbx";

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;

    // Flags: require existing path/file, Explorer-style dialog
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileNameW(&ofn)) {
        // Convert selected wide string to UTF-8
        int required = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
        if (required > 0) {
            std::vector<char> utf8(required, 0);
            WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, utf8.data(), required, nullptr, nullptr);
            return std::string(utf8.data());
        }
        else {
            BOX_LOG_WARNING("openFileDialog: WideCharToMultiByte failed converting path.");
            return std::string();
        }
    }
    else {
        // If user cancelled, CommDlgExtendedError returns 0. Otherwise log the error code.
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            BOX_LOG_WARNING("openFileDialog: GetOpenFileNameW failed, CommDlgExtendedError=" << err);
        }
        return std::string();
    }
}

std::string FileDialog::SaveMBX(const char* defaultExt, const char* filter)
{
    // Uses Win32 GetSaveFileNameW to show a Save dialog and returns UTF-8 path.
    OPENFILENAMEW ofn;
    std::vector<wchar_t> filename(MAX_PATH, L'\0');

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // or obtain HWND from GLFW if you want parented dialog
    ofn.lpstrFile = filename.data();
    ofn.nMaxFile = static_cast<DWORD>(filename.size());

    static const wchar_t mbxFilter[] =
        L"BoxEditor MBX Files (*.mbx)\0"
        L"*.mbx\0"
        L"All Files (*.*)\0"
        L"*.*\0\0";

    ofn.lpstrFilter = mbxFilter;

    ofn.lpstrDefExt = L"mbx";

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;

    // Prompt to overwrite existing files
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

    std::string result;

    if (GetSaveFileNameW(&ofn)) {
        // Convert wide string to UTF-8
        int required = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
        if (required > 0) {
            std::vector<char> utf8(required, 0);
            WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, utf8.data(), required, nullptr, nullptr);
            result.assign(utf8.data());
        }
    }
    else {
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            BOX_LOG_WARNING("openSaveFileDialog: GetSaveFileNameW failed, CommDlgExtendedError=" << err);
        }
    }

    return result;
}







// filter must be a double-null terminated wide string. We accept UTF-8 filter parameter.
// Convert filter to wide char and ensure double-null termination.
//std::wstring wfilter;
//if (filter && filter[0]) {
//    int required = MultiByteToWideChar(CP_UTF8, 0, filter, -1, nullptr, 0);
//    if (required > 0) {
//        wfilter.resize(required);
//        MultiByteToWideChar(CP_UTF8, 0, filter, -1, &wfilter[0], required);
//        // Ensure double null termination
//        if (wfilter.size() == 0 || wfilter.back() != L'\0') wfilter.push_back(L'\0');
//    }
//}
//else {
//    wfilter = L"MBX Files\0*.mbx\0All Files\0*.*\0\0";
//}
//ofn.lpstrFilter = wfilter.c_str();
//ofn.nFilterIndex = 1;

//// Default extension (e.g. "json")
//std::wstring wdefExt;
//if (defaultExt && defaultExt[0]) {
//    int req = MultiByteToWideChar(CP_UTF8, 0, defaultExt, -1, nullptr, 0);
//    if (req > 0) {
//        wdefExt.resize(req);
//        MultiByteToWideChar(CP_UTF8, 0, defaultExt, -1, &wdefExt[0], req);
//    }
//}
//ofn.lpstrDefExt = wdefExt.empty() ? nullptr : wdefExt.c_str();