
#ifndef content_browser_RunFileChooserImpl_h
#define content_browser_RunFileChooserImpl_h

#include "third_party/WebKit/public/web/WebFileChooserParams.h"
#include "third_party/WebKit/public/web/WebFileChooserCompletion.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/Source/wtf/text/WTFStringUtil.h"
#include <vector>

namespace content {

static void appendStringToVector(std::vector<char>* result, const Vector<char>& str)
{
    result->reserve(result->size() + str.size());
    const char* p = str.data();
    const char* end = p + str.size();
    while (p < end) {
        result->push_back(*p);
        ++p;
    }
}

static void appendStringToVector(std::vector<char>* result, const std::string& str)
{
    Vector<char> strBuf;
    strBuf.resize(str.size());
    memcpy(strBuf.data(), str.c_str(), str.size());
    appendStringToVector(result, strBuf);
}

std::string extentionForMimeType(const Vector<char>& mimeType)
{
    const char* end = mimeType.data() + mimeType.size();
    const char* p = end - 1;
    while (p > mimeType.data()) {
        if ('/' == *p) {
            return std::string(p + 1, end);
        }
        --p;
    }
    return "*";
}

std::string extentionForMimeType(const std::string& mimeType)
{
    Vector<char> mimeTypeBuf;
    mimeTypeBuf.resize(mimeType.size());
    memcpy(mimeTypeBuf.data(), mimeType.c_str(), mimeType.size());
    return extentionForMimeType(mimeTypeBuf);
}

static bool runFileChooserImpl(const blink::WebFileChooserParams& params, blink::WebFileChooserCompletion* completion)
{
    std::vector<char> filter;
    if (0 != params.acceptTypes.size()) {
        if (1 == params.acceptTypes.size()) {
            String mimeType = params.acceptTypes[0];
            Vector<char> mimeTypeBuf = WTF::ensureStringToUTF8(mimeType, false);
            appendStringToVector(&filter, mimeTypeBuf);
            filter.push_back('\0');
            appendStringToVector(&filter, "*.");
            appendStringToVector(&filter, extentionForMimeType(mimeTypeBuf));
        } else {
            appendStringToVector(&filter, "Custom Types");
            filter.push_back('\0');
            for (size_t i = 0; i < params.acceptTypes.size(); ++i) {
                if (0 != i)
                    filter.push_back(';');
                appendStringToVector(&filter, "*.");
                String mimeType = params.acceptTypes[i];
                Vector<char> mimeTypeBuf = WTF::ensureStringToUTF8(mimeType, false);
                appendStringToVector(&filter, extentionForMimeType(mimeTypeBuf));
            }
            for (size_t i = 0; i < params.acceptTypes.size(); ++i) {
                filter.push_back('\0');
                String mimeType = params.acceptTypes[i];
                Vector<char> mimeTypeBuf = WTF::ensureStringToUTF8(mimeType, false);
                appendStringToVector(&filter, mimeTypeBuf);
                filter.push_back('\0');
                appendStringToVector(&filter, "*.");
                appendStringToVector(&filter, extentionForMimeType(mimeTypeBuf));
            }
        }
        filter.push_back('\0');
    }

    appendStringToVector(&filter, "All Files");
    filter.push_back('\0');
    appendStringToVector(&filter, "*.*");
    filter.push_back('\0');
    filter.push_back('\0');

    String title = params.title;
    if (title.isEmpty())
        title = "Select File";

    std::vector<wchar_t> fileNameBuf;
    const int fileNameBufLen = 8192;
    fileNameBuf.resize(fileNameBufLen);
    memset(&fileNameBuf[0], 0, sizeof(wchar_t) * fileNameBufLen);

    String initialValue = params.initialValue;
    Vector<UChar> initialValueBuf;
    if (!initialValue.isNull() && !initialValue.isEmpty()) {
        initialValueBuf = WTF::ensureUTF16UChar(initialValue, false);
        if (initialValueBuf.size() < fileNameBufLen - 1)
            wcscpy(&fileNameBuf[0], initialValueBuf.data());
    }

    filter.push_back('\0');
    std::vector<UChar> filterW;
    WTF::MByteToWChar(&filter[0], filter.size() - 1, &filterW, CP_UTF8);

    Vector<UChar> titleBuf = WTF::ensureUTF16UChar(title, true);

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = nullptr;
    ofn.hInstance = nullptr;
    ofn.lpstrFilter = &filterW[0];
    ofn.lpstrFile = &fileNameBuf[0];
    ofn.nMaxFile = fileNameBufLen - 2;
    ofn.lpstrTitle = titleBuf.data();
    ofn.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NOCHANGEDIR;

    BOOL retVal = FALSE;
    if (params.saveAs) {
        ofn.Flags = OFN_OVERWRITEPROMPT;
        retVal = ::GetSaveFileNameW(&ofn);
    } else if (params.multiSelect) {
        ofn.Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT;
        retVal = ::GetOpenFileNameW(&ofn);
    } else {
        ofn.Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        retVal = ::GetOpenFileNameW(&ofn);
    }

    if (!retVal)
        return false;

    std::vector<std::wstring> selectedFiles;
    std::vector<std::wstring> selectedFilesRef;

    if (retVal) {
        // Figure out if the user selected multiple files.  If fileNameBuf is
        // a directory, then multiple files were selected!
        if ((ofn.Flags & OFN_ALLOWMULTISELECT) && (::GetFileAttributesW(&fileNameBuf[0]) & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring dirName = &fileNameBuf[0];
            const wchar_t* p = &fileNameBuf[0] + wcslen(&fileNameBuf[0]) + 1;
            while (*p) {
                selectedFiles.push_back(dirName);
                selectedFiles.back().append(L"\\");
                selectedFiles.back().append(p);
                p += wcslen(p) + 1;
            }
            selectedFilesRef.resize(selectedFiles.size());
            for (size_t i = 0; i < selectedFiles.size(); ++i) {
                selectedFilesRef[i] = selectedFiles[i];
            }
        } else {
            selectedFilesRef.push_back(std::wstring(&fileNameBuf[0]));
        }
    }

    if (0 == selectedFilesRef.size())
        return false;

    blink::WebVector<blink::WebString> wsFileNames(selectedFilesRef.size());
    for (size_t i = 0; i < selectedFilesRef.size(); ++i)
        wsFileNames[i] = String(selectedFilesRef[i].c_str());
    completion->didChooseFile(wsFileNames);

    return true;
}

}

#endif // content_browser_RunFileChooser_h