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

std::string FileDialog::OpenModel()
{
    return std::string();
}

std::string FileDialog::SaveModel()
{
    return std::string();
}
