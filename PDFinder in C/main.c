#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include "resources.h"



#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SETTINGS  1002
#define ID_TRAY_SEARCH  1003
#define IDC_COMBOBOX_DATES 101

#define IDC_SAVE_BUTTON 105
#define IDC_SEARCH_BUTTON 106



HWND hPopupTab = NULL;
HWND hPopupWnd = NULL;
HWND hButton = NULL;

HWND hSearchWnd= NULL;

HINSTANCE g_hInstance = NULL;

int screenWidth;
int screenHeight;

HWND hPdfImage = NULL;







wchar_t key[32];
wchar_t filePattern[256];
wchar_t finalFile[256];
wchar_t folderValue[256];
wchar_t expandedFolder[512];
wchar_t fullFolder[MAX_PATH];
wchar_t folderCheck[MAX_PATH];
wchar_t fullPath[MAX_PATH];


typedef enum {
    FIELD_LABEL,
    FIELD_EDIT,
    FIELD_COMBO,
    FIELD_CHECKBOX,
    FIELD_BUTTON,
} CONTROL_TYPE;



typedef struct
{
    wchar_t label[64];
    CONTROL_TYPE controlType;
    wchar_t sourceName[64];
    wchar_t value[256];

    wchar_t lastComboValue[256];
    bool userChanged;
    bool skipRecent;

    u_int x;
    int y;
    u_int width;
    u_int height;

    wchar_t placeholder[256];

    HWND hControl;

} FIELD_DATA;

typedef struct {
    wchar_t name[64];
    wchar_t type[16];


    wchar_t iniSection[64];
    FIELD_DATA fields[16];
    u_int fieldCount;

    bool hasButton;
} TAB_DATA;

typedef struct {
    wchar_t name[64];
    wchar_t text[64];

    u_int x;
    u_int y;
    u_int width;
    u_int height;

    HWND hWnd;
} BUTTON_DATA;

typedef struct {
    wchar_t label[64];
    wchar_t value[256];
} FIELD_VALUE;

LPCWSTR INI_PATH = L"C:\\watchFolder\\settings.ini";



#define MAX_TABS 32
#define MAX_FIELDS 32
HWND fieldLabels[MAX_FIELDS];
HWND fieldControls[MAX_FIELDS];
int activeFieldCount = 0;
TAB_DATA Tabs[MAX_TABS];



int running = 1;

int g_CurrentPage = 0;
WCHAR g_CurrentFilename[MAX_PATH];
u_int PAGE_COUNT = 0;


void CreateFieldsFromTab(HWND parent, TAB_DATA* tab);
HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text);
HWND OpenSearchWindow(HWND hwndParent);
void OpenSettings();






void DestroyActiveFields(void)
{
    for (int i = 0; i < activeFieldCount; i++)
    {
        if (fieldLabels[i])   DestroyWindow(fieldLabels[i]);
        if (fieldControls[i]) DestroyWindow(fieldControls[i]);
    }
    activeFieldCount = 0;
}

FIELD_DATA* FindFieldByHwnd(HWND hCtrl) {
    for (int t = 0; t < PAGE_COUNT; t++) {
        TAB_DATA* tab = &Tabs[t];
        for (int f = 0; f < tab->fieldCount; f++) {
            if (tab->fields[f].hControl == hCtrl)
                return &tab->fields[f];
        }
    }
    return NULL;
}

static bool IsInvalidFilenameChar(wchar_t c)
{
    if (c < 32) return true;

    switch (c) {
        case L'<': case L'>': case L':':
        case L'"': case L'/': case L'\\':
        case L'|': case L'?': case L'*':
            return true;
        default: ;
    }
    return false;
}

void SanitizeFilename(wchar_t* str)
{
    if (!str) return;

    for (wchar_t* p = str; *p; ++p) {
        if (IsInvalidFilenameChar(*p)) {
            *p = L'-';
        }
    }

    size_t len = wcslen(str);
    while (len > 0 && (str[len - 1] == L' ' || str[len - 1] == L'.')) {
        str[--len] = L'\0';
    }
}




u_int LoadTabCount(void)
{
    return GetPrivateProfileIntW(
        L"Tabs",
        L"Count",
        0,
        INI_PATH
    );
}

bool LoadButtonData(const wchar_t* tabSection, BUTTON_DATA* b)
{
    if (!b) return false;

    b->x = GetPrivateProfileIntW(tabSection, L"Button.X", -1, INI_PATH);
    b->y = GetPrivateProfileIntW(tabSection, L"Button.Y", -1, INI_PATH);

    if (b->x < 0 || b->y < 0)
        return false;

    b->width  = GetPrivateProfileIntW(tabSection, L"Button.Width", 80, INI_PATH);
    b->height = GetPrivateProfileIntW(tabSection, L"Button.Height", 25, INI_PATH);

    GetPrivateProfileStringW(
        tabSection,
        L"Button.Text",
        L"Save",
        b->text,
        64,
        INI_PATH
    );

    wcscpy_s(b->name, 64, L"SaveButton");
    return true;
}




void PopulateControlData(const FIELD_DATA* f)
{
    if (!f->hControl || f->controlType != FIELD_COMBO)
        return;

    SendMessage(f->hControl, CB_RESETCONTENT, 0, 0);

    if (wcslen(f->sourceName) == 0)
        return;

    wchar_t buffer[2048];
    GetPrivateProfileSectionW(f->sourceName, buffer, 2048, INI_PATH);

    for (wchar_t* p = buffer; *p; p += wcslen(p) + 1)
    {
        wchar_t* eq = wcschr(p, L'=');
        if (!eq) continue;
        SendMessage(f->hControl, CB_ADDSTRING, 0, (LPARAM)(eq + 1));
    }

    if (wcslen(f->lastComboValue) > 0) {
        int count = (int)SendMessage(f->hControl, CB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            wchar_t item[256];
            SendMessage(f->hControl, CB_GETLBTEXT, i, (LPARAM)item);
            if (_wcsicmp(item, f->lastComboValue) == 0) {
                SendMessage(f->hControl, CB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}






void LoadTabFields(int tabIndex, const wchar_t* section)
{
    TAB_DATA* tab = &Tabs[tabIndex];

    tab->fieldCount =
        GetPrivateProfileIntW(section, L"FieldCount", 0, INI_PATH);



    for (int f = 0; f < tab->fieldCount; f++)
    {
        FIELD_DATA* field = &tab->fields[f];

        wchar_t key[64];

        swprintf_s(key, 64, L"Field%d.Label", f);
        GetPrivateProfileStringW(section, key, L"",
            field->label, 64, INI_PATH);

        swprintf_s(key, 64, L"Field%d.Control", f);
        wchar_t ctrl[32];
        GetPrivateProfileStringW(section, key, L"",
            ctrl, 32, INI_PATH);

        swprintf_s(key, 64, L"Field%d.Placeholder", f);
        GetPrivateProfileStringW(section, key, L"", field->placeholder, 32, INI_PATH);

        swprintf_s(key, 64, L"Field%d.X", f);
        field->x = GetPrivateProfileIntW(section, key, -1, INI_PATH);

        swprintf_s(key, 64, L"Field%d.Y", f);
        field->y = GetPrivateProfileIntW(section, key, -1, INI_PATH);

        swprintf_s(key, 64, L"Field%d.Width", f);
        field->width = GetPrivateProfileIntW(section, key, -1, INI_PATH);

        swprintf_s(key, 64, L"Field%d.Height", f);
        field->height = GetPrivateProfileIntW(section, key, -1, INI_PATH);


        swprintf_s(key, 64, L"Field%d.SkipRecent", f);
        field->skipRecent = GetPrivateProfileIntW(section, key, 0, INI_PATH) != 0;


        if (field->controlType == FIELD_COMBO && !field->skipRecent) {
            swprintf_s(key, 64, L"Field%d.LastValue", f);
            GetPrivateProfileStringW(
                section,
                key,
                L"",
                field->lastComboValue,
                256,
                INI_PATH
            );
        }


        if (!_wcsicmp(ctrl, L"EDIT")) field->controlType = FIELD_EDIT;
        else if (!_wcsicmp(ctrl, L"COMBO")) field->controlType = FIELD_COMBO;
        else field->controlType = FIELD_LABEL;

        swprintf_s(key, 64, L"Field%d.Source", f);
        GetPrivateProfileStringW(section, key, L"",
            field->sourceName, 64, INI_PATH);
    }
}





void LoadTabsFromIni(HWND hTab)
{
    TabCtrl_DeleteAllItems(hTab);

    PAGE_COUNT = LoadTabCount();

    for (int i = 0; i < PAGE_COUNT; i++)
    {
        wchar_t section[16];
        swprintf_s(section, 16, L"Tab%d", i);

        swprintf_s(Tabs[i].iniSection, 64, L"Tab%d", i);


        GetPrivateProfileStringW(section, L"Name", L"Unnamed",
            Tabs[i].name, 64, INI_PATH);



        LoadTabFields(i, section);

        TCITEM tie = {0};
        tie.mask = TCIF_TEXT;
        tie.pszText = Tabs[i].name;
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}










void CreateFieldsFromTab(HWND parent, TAB_DATA* tab)
{
    DestroyActiveFields();

    int y = 50;
    int xCtrl  = 50;

    int widthCtrl = 150;
    int heightCtrl = 20;


    RECT rc;
    GetClientRect(hPopupTab, &rc);
    TabCtrl_AdjustRect(hPopupTab, FALSE, &rc);

    int baseX = rc.left;
    int baseY = rc.top;



    for (int i = 0; i < tab->fieldCount; i++)
    {
        FIELD_DATA* f = &tab->fields[i];

        int drawX = baseX + ((f->x != -1) ? f->x : xCtrl);
        int drawY = baseY + ((f->y != -1) ? f->y : y);

        int drawWidth  = (f->width != -1) ? f->width : widthCtrl;
        int drawHeight = (f->height != -1) ? f->height : heightCtrl;


        swprintf_s(key, 64, L"Field%d.SkipRecent", i);
        f->skipRecent = GetPrivateProfileIntW(tab->iniSection, key, 0, INI_PATH) != 0;



        fieldLabels[i] = CreateWindowW(
            L"STATIC",
            f->label,
            WS_CHILD | WS_VISIBLE,
            drawX, drawY - 20,
            180, 20,
            parent,
            NULL,
            g_hInstance,
            NULL
        );

        switch (f->controlType)
        {
            case FIELD_EDIT:
                f->hControl = CreateWindowExW(
                    WS_EX_CLIENTEDGE,
                    L"EDIT",
                    L"",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                    drawX, drawY,
                    drawWidth, drawHeight,
                    parent,
                    NULL,
                    g_hInstance,
                    NULL
                );

                y += 50;
                break;

            case FIELD_COMBO:
                f->hControl = CreateWindowW(
                    WC_COMBOBOX,
                    L"",
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,
                    drawX, drawY,
                    drawWidth, drawHeight,
                    parent,
                    NULL,
                    g_hInstance,
                    NULL
                );
                y += 50;
                break;
            default: ;
        }



        if (f->controlType == FIELD_COMBO)
        {
            PopulateControlData(f);
        }

        fieldControls[i] = f->hControl;


        activeFieldCount++;
    }
}


void CreateButtons(HWND parent, int pageIndex) {
    if (hButton) {
        DestroyWindow(hButton);
        hButton = NULL;
    }

    BUTTON_DATA btn;
    if (!LoadButtonData(Tabs[pageIndex].iniSection, &btn))
        return;

    RECT rc;
    GetClientRect(hPopupTab, &rc);
    TabCtrl_AdjustRect(hPopupTab, FALSE, &rc);

    int drawX = rc.left + btn.x;
    int drawY = rc.top  + btn.y;


    if (LoadButtonData(Tabs[pageIndex].iniSection, &btn)) {
        hButton = CreateWindowW(
            L"BUTTON",
            btn.text,
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
            drawX,
            drawY,
            btn.width,
            btn.height,
            parent,
            (HMENU)IDC_SAVE_BUTTON,
            g_hInstance,
            NULL
        );
    }
}























void MakeUniqueFilename(wchar_t* destPath, const wchar_t* basePath) {
    wcscpy_s(destPath, MAX_PATH, basePath);

    if (GetFileAttributesW(destPath) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fName[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];

    _wsplitpath_s(destPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fName, _MAX_FNAME, ext, _MAX_EXT);

    int counter = 2;
    wchar_t newPath[MAX_PATH];

    while (1) {
        swprintf_s(newPath, MAX_PATH, L"%s%s%s (%d)%s", drive, dir, fName, counter, ext);
        if (GetFileAttributesW(newPath) == INVALID_FILE_ATTRIBUTES) {
            wcscpy_s(destPath, MAX_PATH, newPath);
            break;
        }
        counter++;
    }
}

void ExpandTemplate(const wchar_t* input, wchar_t* output, size_t outSize, FIELD_VALUE* fields, int fieldCount) {
    wcscpy_s(output, outSize, input);

    for (int i = 0; i < fieldCount; i++) {
        wchar_t token[128];
        swprintf_s(token, 128, L"{Field:%s}", fields[i].label);

        wchar_t* pos;
        while ((pos = wcsstr(output, token)) != NULL) {
            wchar_t temp[MAX_PATH];
            wcscpy_s(temp, MAX_PATH, pos + wcslen(token));
            *pos = 0;
            wcscat_s(output, outSize, fields[i].value);
            wcscat_s(output, outSize, temp);
        }
    }
}






void SaveFile(int currentPage) {
    if (currentPage < 0 || currentPage >= PAGE_COUNT) return;

    TAB_DATA* tab = &Tabs[currentPage];

    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (f->controlType == FIELD_COMBO) {
            int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR) {
                SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);

                int fieldIndex = (int)(f - &Tabs[g_CurrentPage].fields[0]);
                wchar_t key[64];
                swprintf_s(key, 64, L"Field%d.LastValue", fieldIndex);
                WritePrivateProfileStringW(
                    Tabs[g_CurrentPage].iniSection,
                    key,
                    f->lastComboValue,
                    INI_PATH
                );
            }
        }
    }


    FIELD_VALUE fieldValues[64];
    int fieldCount = 0;
    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (!f->hControl) continue;

        wcscpy_s(fieldValues[fieldCount].label, 64, f->label);
        GetWindowTextW(f->hControl, fieldValues[fieldCount].value, 256);
        fieldCount++;
    }

    wchar_t oldFile[MAX_PATH];
    swprintf_s(oldFile, MAX_PATH, L"C:\\watchFolder\\%s", g_CurrentFilename);
    if (GetFileAttributesW(oldFile) == INVALID_FILE_ATTRIBUTES) {
        MessageBox(NULL, L"Source file not found!", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }

    wchar_t filePattern[256], finalFile[256];
    GetPrivateProfileStringW(tab->iniSection, L"SavedFileName", L"", filePattern, 256, INI_PATH);
    if (filePattern[0] == 0) {
        MessageBox(NULL, L"No SavedFileName defined!", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }
    ExpandTemplate(filePattern, finalFile, 256, fieldValues, fieldCount);
    SanitizeFilename(finalFile);

    for (int pnum = 1; pnum <= 10; pnum++) {
        wchar_t pathKey[32], folderTemplate[512], expandedFolder[512];
        swprintf_s(pathKey, 32, L"Path%d", pnum);
        GetPrivateProfileStringW(tab->iniSection, pathKey, L"", folderTemplate, 512, INI_PATH);
        if (folderTemplate[0] == 0) break;

        ExpandTemplate(folderTemplate, expandedFolder, 512, fieldValues, fieldCount);


        if (expandedFolder[wcslen(expandedFolder) - 1] != L'\\')
            wcscat_s(expandedFolder, 512, L"\\");

        wchar_t tempPath[MAX_PATH] = L"";
        for (size_t j = 0; j < wcslen(expandedFolder); j++) {
            tempPath[j] = expandedFolder[j];
            tempPath[j + 1] = 0;
            if (expandedFolder[j] == L'\\') {
                if (GetFileAttributesW(tempPath) == INVALID_FILE_ATTRIBUTES)
                    CreateDirectoryW(tempPath, NULL);
            }
        }

        wchar_t fullPath[MAX_PATH];
        swprintf_s(fullPath, MAX_PATH, L"%s%s", expandedFolder, finalFile);

        MakeUniqueFilename(fullPath, fullPath);

        if (!CopyFileW(oldFile, fullPath, FALSE)) {
            DWORD err = GetLastError();
            wchar_t buf[512];
            swprintf_s(buf, 512, L"Failed to copy file to:\n%ls\nError %lu", fullPath, err);
            MessageBox(NULL, buf, L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        }
        else {
            MessageBox(NULL, L"File Saved Successfully!", L"Success", MB_OK | MB_TOPMOST);
        }
    }

    DeleteFileW(oldFile);
}




bool IsFileSendReady(int currentPage) {
    if (currentPage < 0 || currentPage >= PAGE_COUNT)
        return false;

    TAB_DATA* tab = &Tabs[currentPage];

    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (!f->hControl)
            continue;

        switch (f->controlType) {
            case FIELD_EDIT: {
                wchar_t text[256];
                GetWindowText(f->hControl, text, 256);
                if (wcslen(text) == 0)
                    return false;
                break;
            }

            case FIELD_COMBO: {
                int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
                if (sel == CB_ERR)
                    return false;
                break;
            }


            default: break;
        }
    }

    return true;
}


void SetPage(int newPage)
{
    if (newPage < 0 || newPage >= PAGE_COUNT)
        return;

    g_CurrentPage = newPage;

    if (hPopupTab)
        TabCtrl_SetCurSel(hPopupTab, newPage);

    CreateFieldsFromTab(hPopupWnd, &Tabs[newPage]);

    CreateButtons(hPopupWnd, newPage);

    TAB_DATA* tab = &Tabs[g_CurrentPage];


    for (int i = 0; i < tab->fieldCount; i++)
        PopulateControlData(&tab->fields[i]);

}





NOTIFYICONDATA nid;


DWORD WINAPI WatchFolder(LPVOID lpParam) {
    HWND hwndParent = (HWND)lpParam;
    LPCWSTR folderPath = L"C:\\watchFolder";

    HANDLE hDir = CreateFileW(
        folderPath,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        MessageBox(hwndParent, L"Failed to open Watch Folder", L"Error", MB_OK);
        return 1;
    }

    BYTE buffer[1024];
    DWORD bytesReturned = 0;

    while (running) {
        if (ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME,
            &bytesReturned,
            NULL,
            NULL
        )) {
            DWORD offset = 0;
            do {
                FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)(buffer + offset);

                unsigned long long const len = fni->FileNameLength / sizeof(WCHAR);
                WCHAR filename[256] = {0};
                wcsncpy_s(filename, 256, fni->FileName, len);
                filename[len] = 0;

                if (fni->Action == FILE_ACTION_ADDED) {
                    WCHAR* fnCopy = _wcsdup(filename);
                    if (fnCopy != NULL) {
                        PostMessage(hwndParent, WM_APP + 1, 0, (LPARAM)fnCopy);
                    }
                }

                offset = fni->NextEntryOffset;
            } while (offset != 0);
        }
    }

    CloseHandle(hDir);
    return 0;
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_APP + 1: {
            WCHAR* filename = (WCHAR*)lParam;

            if (!filename)
                break;

            size_t len = wcslen(filename);

            if (len < 4 || _wcsicmp(filename + len - 4, L".pdf") != 0) {
                free(filename);
                break;
            }

            wcsncpy_s(g_CurrentFilename, MAX_PATH, filename, _TRUNCATE);
            OpenPopupWindow(hwnd, g_CurrentFilename);
            free(filename);
        } break;

        case WM_TRAYICON:
            if (lParam == WM_LBUTTONDOWN) {
                HMENU menu = CreatePopupMenu();

                AppendMenu(menu, MF_STRING, ID_TRAY_SEARCH, L"Search");
                AppendMenu(menu, MF_STRING, ID_TRAY_SETTINGS, L"Settings");
                AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                AppendMenu(menu, MF_STRING, ID_TRAY_EXIT, L"Quit");

                POINT p;
                GetCursorPos(&p);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(menu, TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            break;


        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            case ID_TRAY_SETTINGS:
                    OpenSettings(hwnd);
                    break;
            case ID_TRAY_SEARCH:
                    OpenSearchWindow(hwnd);
                    break;


            default: ;
            }
            break;


        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DestroyIcon(nid.hIcon);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);

    }
    return 0;
}

LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            hSearchWnd= NULL;
            break;

        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(148, 148, 148));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            return 1;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

    LRESULT CALLBACK PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        int currentPage = g_CurrentPage;
        switch(msg) {
            case WM_CLOSE:
                DestroyWindow(hwnd);
                break;
            case WM_DESTROY:
                hPopupWnd = NULL;
                break;
            case WM_KEYDOWN:
            {
                HWND hTab =
                    (hwnd == hPopupWnd)    ? hPopupTab    :
                    NULL;

                if (!hTab)
                    break;

                int page = TabCtrl_GetCurSel(hTab);

                switch (wParam) {
                    case VK_LEFT:
                        if (page > 0) {
                            TabCtrl_SetCurSel(hTab, page - 1);
                            SetPage(page - 1);
                        }
                        return 0;

                    case VK_RIGHT:
                        if (page < PAGE_COUNT - 1) {
                            TabCtrl_SetCurSel(hTab, page + 1);
                            SetPage(page + 1);
                        }
                        return 0;

                    case VK_ESCAPE:
                        DestroyWindow(hwnd);
                        return 0;
                    default: ;
                }

                break;
            }

            case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                if (pnmh->hwndFrom == hPopupTab && pnmh->code == TCN_SELCHANGE)
                {
                    int newPage = TabCtrl_GetCurSel(hPopupTab);
                    SetPage(newPage);
                }
            }
                break;
            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                case CBN_SELCHANGE: {
                    FIELD_DATA* f = FindFieldByHwnd((HWND)lParam);
                    if (f && f->controlType == FIELD_COMBO && !f->skipRecent) {
                        int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
                        if (sel != CB_ERR) {
                            SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);
                            f->userChanged = TRUE;

                            int fieldIndex = (int)(f - &Tabs[g_CurrentPage].fields[0]);

                            wchar_t key[64];
                            swprintf_s(key, 64, L"Field%d.LastValue", fieldIndex);
                            WritePrivateProfileStringW(
                                Tabs[g_CurrentPage].iniSection,
                                key,
                                f->lastComboValue,
                                INI_PATH
                            );
                        }
                    }

                    break;
                }

                case IDC_SAVE_BUTTON:


                        if (!IsFileSendReady(currentPage)) {
                            MessageBox(hwnd, L"Please fill in all required fields before saving file.", L"Warning", MB_OK | MB_ICONWARNING);
                            break;
                        }


                        SaveFile(currentPage);


                        DestroyWindow(hwnd);


                        break;
                default: ;
                }


            case WM_ERASEBKGND:
            {
                HDC hdc = (HDC)wParam;
                RECT rc;
                GetClientRect(hwnd, &rc);
                HBRUSH hBrush = CreateSolidBrush(RGB(148, 148, 148));
                FillRect(hdc, &rc, hBrush);
                DeleteObject(hBrush);
                return 1;
            }

            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }


HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text) {


    const wchar_t CLASS_NAME[] = L"PopupWindowClass";


    static bool registered = false;
    if (!registered) {
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = SearchWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowEx(
    WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
    CLASS_NAME,
    L"New File Alert",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
    400, 200, 700, 450,
    hwndParent,
    NULL,
    g_hInstance,
    NULL
);

    if (!hwnd) {
        return NULL;
    }

    hPopupWnd = hwnd;


    hPopupTab = CreateWindowEx(
    0,
    WC_TABCONTROL,
    NULL,
    WS_CHILD | WS_VISIBLE | WS_THICKFRAME | WS_BORDER,
    10, 10, 672, 405,
    hwnd,
    (HMENU)4001,
    NULL,
    NULL
);




    LoadTabsFromIni(hPopupTab);
    SetPage(0);







    return hwnd;
}



void OpenSettings(HWND hwnd) {
    ShellExecute(
    hwnd,
    L"open",
    L"notepad.exe",
    INI_PATH,
    NULL,
    SW_SHOWNORMAL
);
}

LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_COMMAND:
        {
            switch(LOWORD(wParam)) {
                case IDC_SEARCH_BUTTON:
                {

                    HBITMAP hBmp = (HBITMAP)LoadImageW(
                        NULL,
                        L"C:\\Users\\Macreal\\Desktop\\placeholder.pdf",
                        IMAGE_BITMAP,
                        0, 0,
                        LR_LOADFROMFILE
                    );

                    if (hBmp) {
                        HWND hPic = GetDlgItem(hwnd, 1234);
                        SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                    } else {
                        MessageBox(hwnd, L"Failed to load image", L"Error", MB_OK);
                    }

                    return 0;
                    default: ;
                }
            }
        } break;

        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            return 1;
        }
        default: ;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void RegisterFrameClass(HINSTANCE hInstance)
{
    static bool registered = false;
    if (!registered)
    {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = FrameWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"FrameClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wc);
        registered = true;
    }
}


void MakeWindowRounded(HWND hwnd, int width, int height, int radius) {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    HRGN hRgn = CreateRoundRectRgn(0, 0, width+1, height+1, 20, 20);
    SetWindowRgn(hwnd, hRgn, TRUE);

}



HWND OpenSearchWindow(HWND hwndParent) {

    const wchar_t CLASS_NAME[] = L"Search Menu";


    static bool registered = false;
    if (!registered) {
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = SearchWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wc);
        registered = true;
    }

    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);






    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Search",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        screenWidth, screenHeight,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hwnd)
        return NULL;



    RegisterFrameClass(GetModuleHandle(NULL));


    HWND frame = CreateWindowEx(
        0,
        L"FrameClass",
        NULL,
        WS_CHILD | WS_VISIBLE,
        100, 100, 450, 625,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );


    HWND button = CreateWindowEx(
        0,
        L"BUTTON",
        L"Search",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        100, 100, 100, 20,
        frame,
        (HMENU)IDC_SEARCH_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );

    MakeWindowRounded(frame, 800, 600, 20);






    hPdfImage = CreateWindowEx(
        0,
        L"STATIC",
        NULL,
        WS_CHILD | WS_VISIBLE | SS_BITMAP,
        800, 50, 550, 700,
        hwnd,
        NULL,
        g_hInstance,
        NULL
    );


    HWND pictureFrame = CreateWindowEx(
            0,
            L"STATIC",
            NULL,
            WS_CHILD | WS_VISIBLE,
            800, 50, 600, 725,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );













    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;

}

int WINAPI WinMain(

    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
) {




    const wchar_t CLASS_NAME[] = L"TrayAppClass";
    g_hInstance = hInstance;

    HICON hIcon = (HICON)LoadImage(
        hInstance,
        MAKEINTRESOURCE(IDI_TRAY),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
    );

    if (!hIcon) {
        MessageBox(NULL, L"Icon has failed to load", L"Error", MB_OK);
    }


    INITCOMMONCONTROLSEX initControls = {0};
    initControls.dwSize = sizeof(initControls);
    initControls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&initControls);



    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayAppClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
    0,
    CLASS_NAME,
    L"LISTBOX",
    WS_OVERLAPPEDWINDOW,
    0, 0, 0, 0,
    NULL,
    NULL,
    hInstance,
    NULL
);

    CreateThread(
    NULL,
    0,
    WatchFolder,
    hwnd,
    0,
    NULL
);
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon = hIcon;
    nid.uCallbackMessage = WM_TRAYICON;
    wcscpy_s(nid.szTip, sizeof(nid.szTip), L"File Warden");



    Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        HWND ownerWnd = NULL;
        HWND activeTab = NULL;


        if (hPopupWnd && msg.hwnd &&
            (msg.hwnd == hPopupWnd || IsChild(hPopupWnd, msg.hwnd)))
        {
            ownerWnd = hPopupWnd;
            activeTab = hPopupTab;
        }


        if (ownerWnd && msg.message == WM_KEYDOWN)
        {
            int page = TabCtrl_GetCurSel(activeTab);

            switch (msg.wParam)
            {
                case VK_LEFT:
                    if (page > 0) {
                        TabCtrl_SetCurSel(activeTab, page - 1);
                        SetPage(page - 1);
                    }
                    continue;

                case VK_RIGHT:
                    if (page < 2) {
                        TabCtrl_SetCurSel(activeTab, page + 1);
                        SetPage(page + 1);
                    }
                    continue;

                case VK_ESCAPE:
                    DestroyWindow(ownerWnd);
                    continue;
                default: ;
            }
        }



        HWND hDlg = ownerWnd ? ownerWnd : msg.hwnd;
        if (!IsDialogMessage(hDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
