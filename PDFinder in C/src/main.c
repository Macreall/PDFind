#define UNICODE


#include "../resources.h"
#include "PDF Parser.h"




struct fz_context* pdf_ctx = NULL;
struct fz_document* doc = NULL;
int current_pdf_page = 0;
HWND g_pdfFrame = NULL;
HWND g_pictureFrame = NULL;

LPCWSTR INI_SAVE = L"C:\\watchFolder\\save_settings.ini";
LPCWSTR INI_SEARCH = L"C:\\watchFolder\\search_settings.ini";


HWND g_PrintFrame = NULL;
HWND hOptionsPanel;
HWND hPreviewFrame;

typedef struct {
    
    int copies;
    int pageStart;
    int pageEnd;

    bool allPages;



} PRINT_SETTINGS;


HWND hCopiesEdit;
HWND hStartPageEdit;
HWND hEndPageEdit;

HWND hAllPagesRadio;
HWND hCustomPagesRadio;

HBRUSH gOptionsBrush;
HBRUSH gPreviewBrush;



HWND hToolbar = NULL;

int total_pages = 0;

wchar_t* current_opened_pdf = NULL;
wchar_t* current_opened_pdf_full = NULL;


HWND g_toolbar = NULL;

HBITMAP g_pdfBitmap = NULL;
HDC     g_pdfMemDC = NULL;

int pdfWidth = 0;
int pdfHeight = 0;

int pageHeights[256];


#define PAGE_W  650
#define PAGE_H  755
#define PAGE_GAP 20  

HWND hPageLabel = NULL;
HWND hSearchCountLabel = NULL;



WNDPROC g_oldLabelProc = NULL;




#define MAX_RESULTS 256


typedef struct {
    wchar_t fullPath[MAX_PATH];
} FOUND_FILE;

typedef struct {
    FOUND_FILE items[MAX_RESULTS];
    int count;
} FOUND_LIST;

int g_CurrentResultIndex = 0;

FOUND_LIST* foundResults;



HWND hLeftArrow;
HWND hRightArrow;
HWND hFileNameText;
HWND hPrintButton;

HWND hCombo;

BOOL g_SearchActive = FALSE;


void LayoutCustomerNav(HWND hwndParent);




















#define MAX_NAMES 256
#define NAME_LEN 128

int CompareNames(const void* a, const void* b)
{
    const wchar_t* s1 = *(const wchar_t**)a;
    const wchar_t* s2 = *(const wchar_t**)b;

    return _wcsicmp(s1, s2);
}

void ModifyNameSorted(const wchar_t* iniSection, const wchar_t* name, bool remove)
{
    wchar_t section[4096];
    wchar_t names[MAX_NAMES][NAME_LEN];
    wchar_t* ptrs[MAX_NAMES];
    int count = 0;

    GetPrivateProfileSectionW(iniSection, section, 4096, INI_SAVE);

    wchar_t* p = section;
    while (*p && count < MAX_NAMES)
    {
        wchar_t* eq = wcschr(p, L'=');
        if (eq)
        {
            wchar_t* value = eq + 1;
            wcscpy(names[count], value);
            ptrs[count] = names[count];
            count++;
        }
        p += wcslen(p) + 1;
    }

    if (remove)
    {
        int newCount = 0;
        wchar_t* newPtrs[MAX_NAMES];
        for (int i = 0; i < count; i++)
        {
            if (wcscmp(ptrs[i], name) != 0)
            {
                newPtrs[newCount++] = ptrs[i];
            }
        }
        count = newCount;
        for (int i = 0; i < count; i++)
            ptrs[i] = newPtrs[i];
    }
    else
    {
        if (count < MAX_NAMES)
        {
            wcscpy(names[count], name);
            ptrs[count] = names[count];
            count++;
        }
    }

    qsort(ptrs, count, sizeof(wchar_t*), CompareNames);

    WritePrivateProfileStringW(iniSection, NULL, NULL, INI_SAVE);
    WritePrivateProfileStringW(iniSection, NULL, NULL, INI_SEARCH);

    for (int i = 0; i < count; i++)
    {
        wchar_t key[16];
        swprintf_s(key, 16, L"%d", i + 1);
        WritePrivateProfileStringW(iniSection, key, ptrs[i], INI_SAVE);
        WritePrivateProfileStringW(iniSection, key, ptrs[i], INI_SEARCH);
    }
}









void ClearSearchResults()
{
    if (foundResults)
    {
        free(foundResults);
        foundResults = NULL;
    }
}




void next_page(int total_pages) {
    if (current_pdf_page < total_pages - 1)
        current_pdf_page++;
}

void previous_page() {
    if (current_pdf_page > 0)
        current_pdf_page--;
}





#define WM_TRAYICON (WM_USER + 1)
#define WM_APP_REDRAW_PDF   (WM_APP + 1)
#define ID_TRAY_EXIT 90
#define ID_TRAY_SAVE_SETTINGS  91
#define ID_TRAY_SEARCH_SETTINGS  92
#define ID_TRAY_SEARCH  93
#define ID_TRAY_UNDO 94
#define IDC_COMBOBOX_DATES 101

#define IDC_SAVE_BUTTON 105
#define IDC_SEARCH_BUTTON 106


#define IDC_LEFT_ARROW 107
#define IDC_RIGHT_ARROW 108
#define IDC_NAME_TEXT 109
#define IDC_PRINT_BUTTON 110

#define IDC_COPIES_EDIT 120
#define IDC_CUSTOM_PAGES 121
#define IDC_ALL_PAGES 122
#define IDC_START_PAGE 123
#define IDC_END_PAGE 124
#define IDC_PRINT_PDF 127
#define IDC_CANCEL_PDF 128


HWND hPopupTab = NULL;
HWND hPopupWnd = NULL;

HWND hSearchTab = NULL;
HWND hSearchWnd = NULL;

HWND frame = NULL;

HINSTANCE g_hInstance = NULL;

int screenWidth;
int screenHeight;

HWND hPdfImage = NULL;

#ifndef OIC_PRINT
#define OIC_PRINT 32544
#endif






#define MAX_BUTTONS 10




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
    HWND hButtonPlus;
    HWND hButtonMinus;
    HWND hListboxButton;
    HWND hwnd;

} FIELD_DATA;

typedef enum {
    BTN_SAVE,
    BTN_SEARCH,
    BTN_OPEN_LIST
} BUTTON_FUNCTION;

typedef struct {
    wchar_t name[64];
    wchar_t text[64];

    u_int x;
    u_int y;
    u_int width;
    u_int height;

    HWND hWnd;
    int id;

    BUTTON_FUNCTION function;
} BUTTON_DATA;

typedef struct {
    wchar_t name[64];
    wchar_t type[16];


    wchar_t iniSection[64];
    FIELD_DATA fields[16];
    u_int fieldCount;
    HWND hPage; 

    BUTTON_DATA buttons[MAX_BUTTONS];
    int buttonCount;

    LPCWSTR iniPath;

    BOOL searchRecursive;
} TAB_DATA;




typedef struct {
    wchar_t label[64];
    wchar_t value[256];
} FIELD_VALUE;





#define MAX_TABS 32
#define MAX_FIELDS 32
HWND fieldLabels[MAX_FIELDS];
HWND fieldControls[MAX_FIELDS];
HWND fieldPlusButtons[MAX_FIELDS];
HWND fieldMinusButtons[MAX_FIELDS];
int activeFieldCount = 0;

TAB_DATA SearchTabs[MAX_TABS];
TAB_DATA SaveTabs[MAX_TABS];


int running = 1;

int g_CurrentPage = 0;
WCHAR g_CurrentFilename[MAX_PATH];
u_int PAGE_COUNT = 0;

u_int SearchTabCount;
u_int SaveTabCount;


void CreateFieldsFromTab(HWND parent, TAB_DATA* tab, LPCWSTR iniPath, BOOL showAddButtons, BOOL showListboxButton);
HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text);
HWND OpenSearchWindow(HWND hwndParent);
HWND OpenPrintWindow(HWND hwndParent);
LRESULT CALLBACK PictureFrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PrintWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ExpandTemplate(const wchar_t* input, wchar_t* output, size_t outSize, FIELD_VALUE* fields, int fieldCount);
void LoadNewPDF(const wchar_t* newPath);



static HWND g_hEdit;
static wchar_t* g_Result;
static int g_ResultSize;
static BOOL g_InputConfirmed = FALSE;




















































HWND hListBox;
HWND hNameEdit;
HWND hPOEdit;
HWND g_customerPopup = NULL;


#define IDC_SHOW_LISTBOX 1090
#define IDC_REMOVE_LISTBOX 1059
#define IDC_CLOSE_LISTBOX 1060



#define ID_NAME      1002
#define ID_PO        1003
#define ID_ADD       1004
#define ID_REMOVE    1005
#define ID_SAVE      1006
#define ID_CANCEL    1007
#define IDC_NAME_EDIT 1055
#define IDC_ADD_LISTBOX 1056
#define IDC_LISTBOX   1057
#define IDC_PO_EDIT 1058


typedef struct {
    wchar_t name[256];
    wchar_t poNumber[64];
} CustomerEntry;

typedef struct {
    int count;
    CustomerEntry* items;
} TempCustomerList;

TempCustomerList g_customerList = { 0, NULL };
wchar_t g_currentDate[16] = L"2026-03-26"; 



void RefreshListBox(HWND hListBox) {
    SendMessageW(hListBox, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < g_customerList.count; i++) {
        wchar_t buffer[320];
        swprintf_s(buffer, 320, L"%s|%s", g_customerList.items[i].name, g_customerList.items[i].poNumber);
        SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

void RemoveCustomerByIndex(int index)
{
    if (index < 0 || index >= g_customerList.count)
        return;

    for (int i = index; i < g_customerList.count - 1; i++)
    {
        g_customerList.items[i] = g_customerList.items[i + 1];
    }

    g_customerList.count--;
}



LRESULT CALLBACK CustomerPopupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_COMMAND:
    {

        int id = LOWORD(wParam);


        if (id == IDC_CLOSE_LISTBOX)
        {
            DestroyWindow(hwnd); 
        }


        if (id == IDC_REMOVE_LISTBOX)
        {
            int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);

            if (sel == LB_ERR)
            {
                MessageBox(hwnd, L"Please select an item to remove.", L"Info", MB_OK);
                break;
            }

            wchar_t text[256];
            SendMessage(hListBox, LB_GETTEXT, sel, (LPARAM)text);

            int result = MessageBox(
                hwnd,
                L"Are you sure you want to remove this item?",
                L"Confirm",
                MB_YESNO | MB_ICONQUESTION
            );

            if (result == IDYES)
            {
                SendMessage(hListBox, LB_DELETESTRING, sel, 0);

                RemoveCustomerByIndex(sel);

            }
        }





        if (id == IDC_ADD_LISTBOX)
        {
            wchar_t Namebuffer[256];
            GetWindowTextW(hNameEdit, Namebuffer, 256);

            wchar_t poBuffer[256];
            GetWindowTextW(hPOEdit, poBuffer, 256);

            if (wcslen(Namebuffer) > 0 && wcslen(poBuffer) > 0)
            {
                CustomerEntry* newItems = (CustomerEntry*)realloc(
                    g_customerList.items,
                    (g_customerList.count + 1) * sizeof(CustomerEntry)
                );

                if (!newItems) {
                    MessageBoxW(hwnd, L"Memory allocation failed!", L"Error", MB_OK);
                    return 0;
                }

                g_customerList.items = newItems;

                wcscpy_s(g_customerList.items[g_customerList.count].name, 256, Namebuffer);
                wcscpy_s(g_customerList.items[g_customerList.count].poNumber, 64, poBuffer);

                g_customerList.count++;

                RefreshListBox(hListBox);

                SetWindowTextW(hNameEdit, L"");
                SetWindowTextW(hPOEdit, L"");

                SetFocus(hNameEdit);
            }

        }
        break;
    }
    

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:

        EnableWindow(hPopupWnd, TRUE);

        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void OpenCustomerPopup(HWND parent) {

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = CustomerPopupProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = L"CustomerPopupClass";
    RegisterClassW(&wc);
    
    int width = 550;
    int height = 450;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW | WS_EX_CONTROLPARENT,
        L"CustomerPopupClass",
        L"Customer List",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX,
        x, y, width, height,
        parent,
        NULL,
        g_hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBoxW(parent, L"Failed to create popup!", L"Error", MB_OK);
        return;
    }

    g_customerPopup = hwnd;


    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);


    int marginLeft = 20;
    int labelWidth = 80;
    int editWidth = 200;
    int rowHeight = 25;
    int spacingY = 10; 
    int yName = 20;
    int yPO = yName + rowHeight + spacingY;
    int yList = yPO + rowHeight + 20;


    CreateWindowW(
        L"STATIC",
        L"Name:",
        WS_CHILD | WS_VISIBLE,
        marginLeft, yName,
        labelWidth, rowHeight,
        hwnd,
        NULL,
        g_hInstance,
        NULL
    );

    hNameEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        marginLeft + labelWidth, yName,
        editWidth, rowHeight,
        hwnd,
        (HMENU)IDC_NAME_EDIT,
        g_hInstance,
        NULL
    );

    CreateWindowW(
        L"STATIC",
        L"PO:",
        WS_CHILD | WS_VISIBLE,
        marginLeft, yPO,
        labelWidth, rowHeight,
        hwnd,
        NULL,
        g_hInstance,
        NULL
    );

    hPOEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
        marginLeft + labelWidth, yPO,
        editWidth, rowHeight,
        hwnd,
        (HMENU)IDC_PO_EDIT,
        g_hInstance,
        NULL
    );

    CreateWindowW(
        L"BUTTON",
        L"Add",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        marginLeft + labelWidth + editWidth + 10, yPO,
        80, rowHeight,
        hwnd,
        (HMENU)IDC_ADD_LISTBOX,
        g_hInstance,
        NULL
    );

    CreateWindowW(
        L"BUTTON",
        L"Remove",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        marginLeft + labelWidth + editWidth + 10, yName,
        80, rowHeight,
        hwnd,
        (HMENU)IDC_REMOVE_LISTBOX,
        g_hInstance,
        NULL
    );

    CreateWindowW(
        L"BUTTON",
        L"Close",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        marginLeft + labelWidth + editWidth + 10, 350, 
        80, rowHeight,
        hwnd,
        (HMENU)IDC_CLOSE_LISTBOX, 
        g_hInstance,
        NULL
    );

    hListBox = CreateWindowW(
        L"LISTBOX",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        marginLeft, yList,
        labelWidth + editWidth + 90, 200,
        hwnd,
        (HMENU)IDC_LISTBOX,
        g_hInstance,
        NULL
    );


    if (g_customerList.count > 0) {
        RefreshListBox(hListBox);
    }
  
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    
    SetFocus(hNameEdit);

    EnableWindow(parent, FALSE);


}



void ParseListItem(const wchar_t* input, wchar_t* nameOut, int* idOut)
{
    const wchar_t* separator = wcschr(input, L'|');

    if (!separator)
    {
        wcscpy_s(nameOut, 256, input);
        *idOut = -1;
        return;
    }

    int nameLen = (int)(separator - input);
    wcsncpy_s(nameOut, 256, input, nameLen);

    *idOut = _wtoi(separator + 1);
}




void SaveToJson(const wchar_t* date, const wchar_t* path)
{
    FILE* file;

    wchar_t fullpath[MAX_PATH];
    wcscpy_s(fullpath, 256, path);

    PathRemoveFileSpecW(fullpath);
    wcscat_s(fullpath, MAX_PATH, L"\\List.json");


    errno_t err = _wfopen_s(&file, fullpath, L"a, ccs=UTF-8");

    if (err != 0 || !file)
    {
        wchar_t buf[256];
        swprintf_s(buf, 256, L"Open failed! errno: %d", err);
        MessageBoxW(NULL, buf, L"Error", MB_OK);
        return;
    }
 
    for (int i = 0; i < g_customerList.count; i++)
    {
        fwprintf(file,
            L"{\"date\":\"%s\",\"name\":\"%s\",\"id\":\"%s\"}\n",
            date,
            g_customerList.items[i].name,
            g_customerList.items[i].poNumber
        );
    }

    fclose(file);

}

























void PopulatePrinterCombo(HWND hCombo)
{
    DWORD needed = 0, returned = 0;

    EnumPrinters(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
        NULL,
        2,
        NULL,
        0,
        &needed,
        &returned
    );

    BYTE* buffer = (BYTE*)malloc(needed);

    if (!EnumPrinters(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
        NULL,
        2,
        buffer,
        needed,
        &needed,
        &returned))
    {
        free(buffer);
        return;
    }

    PRINTER_INFO_2* printers = (PRINTER_INFO_2*)buffer;

    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

    for (DWORD i = 0; i < returned; i++)
    {
        SendMessage(
            hCombo,
            CB_ADDSTRING,
            0,
            (LPARAM)printers[i].pPrinterName
        );
    }

    free(buffer);

    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

















LRESULT CALLBACK InputBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_CREATE:
    {
        CreateWindowW(L"STATIC", L"Enter Here:",
            WS_CHILD | WS_VISIBLE,
            20, 20, 200, 20,
            hwnd, NULL, NULL, NULL);

        g_hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            20, 45, 240, 25,
            hwnd, (HMENU)1, NULL, NULL);

        CreateWindowW(L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE,
            60, 85, 70, 30,
            hwnd, (HMENU)2, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE,
            150, 85, 70, 30,
            hwnd, (HMENU)3, NULL, NULL);

        break;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        if (id == 2)
        {
            GetWindowTextW(g_hEdit, g_Result, g_ResultSize);
            g_InputConfirmed = TRUE;
            DestroyWindow(hwnd);
        }

        if (id == 3)
        {
            g_InputConfirmed = FALSE;
            DestroyWindow(hwnd);
        }

        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            int len = GetWindowTextLengthW(g_hEdit);
            if (len > 0)
            {
                GetWindowTextW(g_hEdit, g_Result, g_ResultSize);
                g_InputConfirmed = TRUE;
                DestroyWindow(hwnd);
            }
            else
            {
                SetFocus(g_hEdit);
            }
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}





BOOL InputBox(HWND parent, HINSTANCE hInstance, wchar_t* result, int resultSize)
{
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = InputBoxProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"InputBoxClass";

    RegisterClassW(&wc);

    g_Result = result;
    g_ResultSize = resultSize;
    g_InputConfirmed = FALSE;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"InputBoxClass",
        L"Add New",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        320, 180,
        parent,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);


    RECT rcParent, rcWnd;

    GetWindowRect(parent, &rcParent);
    GetWindowRect(hwnd, &rcWnd);

    int width = rcWnd.right - rcWnd.left;
    int height = rcWnd.bottom - rcWnd.top;

    int x = rcParent.left + ((rcParent.right - rcParent.left) - width) / 2;
    int y = rcParent.top + ((rcParent.bottom - rcParent.top) - height) / 2;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);


    MSG msg;

    while (IsWindow(hwnd) && GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return g_InputConfirmed;
}









void PreviousResult() {
    if (foundResults && foundResults->count > 0)
    {
        if (g_CurrentResultIndex > 0)
        {
            g_CurrentResultIndex--;
            LoadNewPDF(foundResults->items[g_CurrentResultIndex].fullPath);

            WCHAR pageText[64];
            swprintf_s(pageText, 64, L"Result   %d / %d", g_CurrentResultIndex + 1, foundResults->count);
            SetWindowTextW(hSearchCountLabel, pageText);

        }
    }

}


void NextResult() {
    if (foundResults && foundResults->count > 0)
    {
        if (g_CurrentResultIndex < foundResults->count - 1)
        {
            g_CurrentResultIndex++;
            LoadNewPDF(foundResults->items[g_CurrentResultIndex].fullPath);


            WCHAR pageText[64];
            swprintf_s(pageText, 64, L"Result   %d / %d", g_CurrentResultIndex + 1, foundResults->count);
            SetWindowTextW(hSearchCountLabel, pageText);

        }
    }
}



int GetTextPixelWidth(HWND hStatic, const wchar_t* text)
{
    HDC hdc = GetDC(hStatic);

    HFONT hFont = (HFONT)SendMessage(hStatic, WM_GETFONT, 0, 0);
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);

    SIZE size;
    GetTextExtentPoint32W(hdc, text, lstrlenW(text), &size);

    SelectObject(hdc, hOld);
    ReleaseDC(hStatic, hdc);

    return size.cx;
}




BOOL CALLBACK DestroyChildProc(HWND hwnd, LPARAM lParam)
{
    DestroyWindow(hwnd);
    return TRUE; 
}

void OpenSaveSettings(HWND hwnd) {
    ShellExecute(
        hwnd,
        L"open",
        L"notepad.exe",
        INI_SAVE,
        NULL,
        SW_SHOWNORMAL
    );
}

void OpenSearchSettings(HWND hwnd) {
    ShellExecute(
        hwnd,
        L"open",
        L"notepad.exe",
        INI_SEARCH,
        NULL,
        SW_SHOWNORMAL
    );
}


void PrintCurrentPDF(HWND hwnd)
{


    PRINT_SETTINGS* settings =
        (PRINT_SETTINGS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!settings)
        return;


    if (!current_opened_pdf_full)
    {
        MessageBox(NULL, L"No PDF loaded", L"Error", MB_OK | MB_ICONERROR);
        return;
    }



    wchar_t printerName[256];

    int sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR)
    {
        MessageBox(NULL, L"No printer selected", L"Error", MB_OK);
        return;
    }

    SendMessage(hCombo, CB_GETLBTEXT, sel, (LPARAM)printerName);

    int copies = GetDlgItemInt(hOptionsPanel, IDC_COPIES_EDIT, NULL, FALSE);


    
    BOOL allPages = settings->allPages;

    BOOL startValid = FALSE;
    BOOL endValid = FALSE;

    int startPage = GetDlgItemInt(hOptionsPanel, IDC_START_PAGE, &startValid, FALSE);
    int endPage = GetDlgItemInt(hOptionsPanel, IDC_END_PAGE, &endValid, FALSE);



    int start, end;

    if (allPages)
    {
        start = 0;
        end = total_pages;
    }
    else
    {
        if (startValid && endValid)
        {
            start = max(0, startPage - 1);
            end = min(total_pages, endPage);
        }
        else if (startValid && !endValid)
        {
            start = max(0, startPage - 1);
            end = total_pages;
        }
        else if (!startValid && endValid)
        {
            start = 0;
            end = min(total_pages, endPage);
        }
        else
        {
            start = 0;
            end = total_pages;
        }
    }

    if (start >= end)
    {
        MessageBox(hwnd, L"Invalid page range", L"Error", MB_OK | MB_ICONERROR);
        return;
    }


    HDC hdcPrinter = CreateDCW(L"WINSPOOL", printerName, NULL, NULL);
    if (!hdcPrinter)
    {
        MessageBox(hwnd, L"Printer DC failed", L"Error", MB_OK);
        return;
    }

    DOCINFOW di = { 0 };
    di.cbSize = sizeof(di);
    di.lpszDocName = L"PDF Print Job";

    if (StartDocW(hdcPrinter, &di) <= 0)
    {
        DeleteDC(hdcPrinter);
        return;
    }

   

    int physicalWidth = GetDeviceCaps(hdcPrinter, PHYSICALWIDTH);
    int physicalHeight = GetDeviceCaps(hdcPrinter, PHYSICALHEIGHT);
    int physicalOffsetX = GetDeviceCaps(hdcPrinter, PHYSICALOFFSETX);
    int physicalOffsetY = GetDeviceCaps(hdcPrinter, PHYSICALOFFSETY);


    int printableWidth = physicalWidth - (2 * physicalOffsetX); 
    int printableHeight = physicalHeight - (2 * physicalOffsetY);


    for (int c = 0; c < max(1, copies); c++)
    {
        for (int i = start; i < end; i++)
        {
            if (StartPage(hdcPrinter) <= 0)
                break;



            fz_page* page = fz_load_page(pdf_ctx, doc, i);
            fz_rect bounds = fz_bound_page(pdf_ctx, page);

            float pageW = bounds.x1 - bounds.x0;
            float pageH = bounds.y1 - bounds.y0;


            float scaleX = (float)printableWidth / pageW;
            float scaleY = (float)printableHeight / pageH;

            float scale = min(scaleX, scaleY);



            float dpi = (float)GetDeviceCaps(hdcPrinter, LOGPIXELSX);
            float zoom = dpi / 72.0f;

            float qualityBoost = 1.5f;

            fz_matrix ctm = fz_scale(zoom * qualityBoost, zoom * qualityBoost);

            ctm = fz_translate(-bounds.x0, -bounds.y0);






            fz_rect r = fz_transform_rect(bounds, ctm);
            fz_irect bbox = fz_round_rect(r);
            fz_pixmap* pix = fz_new_pixmap_with_bbox(pdf_ctx, fz_device_rgb(pdf_ctx), bbox, NULL, 1);



            fz_clear_pixmap_with_value(pdf_ctx, pix, 255);

            fz_device* dev = fz_new_draw_device(pdf_ctx, ctm, pix);
            fz_run_page(pdf_ctx, page, dev, ctm, NULL);
            fz_drop_device(pdf_ctx, dev);


            int w = fz_pixmap_width(pdf_ctx, pix);
            int h = fz_pixmap_height(pdf_ctx, pix);

            //int offsetX = (printableWidth - w) / 2;
            //int offsetY = (printableHeight - h) / 2;

            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = w;
            bmi.bmiHeader.biHeight = -h;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

        

            StretchDIBits(
                hdcPrinter,
                0, 0,
                printableWidth, printableHeight,
                0, 0,
                w, h,
                fz_pixmap_samples(pdf_ctx, pix),
                &bmi,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            EndPage(hdcPrinter);

            fz_drop_pixmap(pdf_ctx, pix);   
            fz_drop_page(pdf_ctx, page);


            
        }
    }



    EndDoc(hdcPrinter);
    DeleteDC(hdcPrinter);


    DestroyWindow(hwnd);
}



void ClearTabFields(int tabIndex, TAB_DATA* tabs)
{
    TAB_DATA* tab = &tabs[tabIndex];
    for (int f = 0; f < tab->fieldCount; f++)
    {
        FIELD_DATA* field = &tab->fields[f];

        if (!field->hControl)
            continue;

        switch (field->controlType)
        {
        case FIELD_EDIT:
            SendMessage(field->hControl, WM_SETTEXT, 0, (LPARAM)L"");
            break;

        case FIELD_COMBO:
            break;

        case FIELD_LABEL:
            break;
        }
    }
}




void RenderPageToCache(HWND hwnd)
{
    

    if (pdf_ctx)
    {
        fz_empty_store(pdf_ctx);
    }



    if (g_pdfBitmap) {
        DeleteObject(g_pdfBitmap);
        DeleteDC(g_pdfMemDC);
    }

    HDC hdc = GetDC(hwnd);

    pdfWidth = PAGE_W;
    pdfHeight = 0;
    int pageHeights[256];
    for (int i = 0; i < total_pages; i++) {
        fz_page* page = fz_load_page(pdf_ctx, doc, i);
        fz_rect bounds = fz_bound_page(pdf_ctx, page);
        fz_irect ib = fz_round_rect(bounds);
        pageHeights[i] = (ib.y1 - ib.y0) + PAGE_GAP;
        pdfHeight += pageHeights[i];
        fz_drop_page(pdf_ctx, page);
    }

    g_pdfMemDC = CreateCompatibleDC(hdc);
    g_pdfBitmap = CreateCompatibleBitmap(hdc, pdfWidth, pdfHeight);
    SelectObject(g_pdfMemDC, g_pdfBitmap);

    HBRUSH bg = CreateSolidBrush(RGB(220, 220, 220));
    RECT r = { 0,0,pdfWidth,pdfHeight };
    FillRect(g_pdfMemDC, &r, bg);
    DeleteObject(bg);

    int y_offset = 0;
    for (int i = 0; i < total_pages; i++) {
        fz_page* page = fz_load_page(pdf_ctx, doc, i);
        fz_rect bounds = fz_bound_page(pdf_ctx, page);
        fz_irect ib = fz_round_rect(bounds);

        int pageW = ib.x1 - ib.x0;
        int pageH = ib.y1 - ib.y0;

        pageHeights[i] = pageH + PAGE_GAP;


        RECT pr = { 0, y_offset, PAGE_W, y_offset + pageH };
        HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(g_pdfMemDC, &pr, white);
        DeleteObject(white);

        fz_matrix ctm = fz_translate(-bounds.x0, -bounds.y0);

        fz_pixmap* pix = fz_new_pixmap_with_bbox(
            pdf_ctx,
            fz_device_rgb(pdf_ctx),
            fz_round_rect(fz_transform_rect(bounds, ctm)),
            NULL,
            1
        );
        fz_clear_pixmap_with_value(pdf_ctx, pix, 255);

        fz_device* dev = fz_new_draw_device(pdf_ctx, ctm, pix);
        fz_run_page(pdf_ctx, page, dev, ctm, NULL);
        fz_drop_device(pdf_ctx, dev);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = fz_pixmap_width(pdf_ctx, pix);
        bmi.bmiHeader.biHeight = -fz_pixmap_height(pdf_ctx, pix);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        StretchDIBits(
            g_pdfMemDC,
            0, y_offset,
            pdfWidth, pageH,
            0, 0,
            fz_pixmap_width(pdf_ctx, pix),
            fz_pixmap_height(pdf_ctx, pix),
            fz_pixmap_samples(pdf_ctx, pix),
            &bmi,
            DIB_RGB_COLORS,
            SRCCOPY
        );

        y_offset += pageH + PAGE_GAP;

        fz_drop_pixmap(pdf_ctx, pix);
        fz_drop_page(pdf_ctx, page);
    }

    ReleaseDC(hwnd, hdc);
    scrollY = 0;
    maxScroll = pdfHeight - PAGE_H;
    if (maxScroll < 0) maxScroll = 0;
}










void LoadNewPDF(const wchar_t* newPath)
{
    if (!newPath) return;

    if (doc)
    {
        fz_drop_document(pdf_ctx, doc);
        doc = NULL;
    }

    if (g_pdfBitmap)
    {
        DeleteObject(g_pdfBitmap);
        DeleteDC(g_pdfMemDC);
        g_pdfBitmap = NULL;
        g_pdfMemDC = NULL;
    }

    char utf8_path[MAX_PATH * 3];
    int len = WideCharToMultiByte(
        CP_UTF8, 0,
        newPath, -1,
        utf8_path, sizeof(utf8_path),
        NULL, NULL
    );

    if (len == 0)
        return;

    fz_try(pdf_ctx)
    {
        doc = fz_open_document(pdf_ctx, utf8_path);
    }
    fz_catch(pdf_ctx)
    {
        MessageBoxA(NULL, fz_caught_message(pdf_ctx), "Failed to open PDF", MB_OK);
        return;
    }

    total_pages = fz_count_pages(pdf_ctx, doc);
    current_pdf_page = 0;


    wchar_t fullPath[MAX_PATH];
    wcscpy_s(fullPath, MAX_PATH, newPath);

    if (current_opened_pdf_full)
    {
        free(current_opened_pdf_full);
    }


    current_opened_pdf_full = _wcsdup(fullPath);
    current_opened_pdf = PathFindFileNameW(current_opened_pdf_full);



    RenderPageToCache(g_pictureFrame);

    PostMessage(g_pictureFrame, WM_APP_REDRAW_PDF, 0, 0);

    if (g_toolbar)
        InvalidateRect(g_toolbar, NULL, TRUE);

    WCHAR pageText[64];
    swprintf_s(pageText, 64, L"Page   %d / %d", current_pdf_page + 1, total_pages);
    SetWindowTextW(hPageLabel, pageText);
}








void AddFound(FOUND_LIST* list, const wchar_t* path)
{
    if (list->count >= MAX_RESULTS)
        return;

    wcscpy_s(list->items[list->count].fullPath, MAX_PATH, path);
    list->count++;
}


wchar_t seenDates[256][64];
int seenCount = 0;

BOOL AlreadySeen(wchar_t seenDates[][64], int count, const wchar_t* date)
{
    for (int i = 0; i < count; i++)
    {
        if (_wcsicmp(seenDates[i], date) == 0)
            return TRUE;
    }
    return FALSE;
}








BOOL ExtractJsonValue(const wchar_t* line, const wchar_t* key, wchar_t* out, int outSize)
{
    wchar_t pattern[64];
    swprintf_s(pattern, 64, L"\"%s\":\"", key);

    wchar_t* start = wcsstr(line, pattern);
    if (!start) return FALSE;

    start += wcslen(pattern);

    wchar_t* end = wcschr(start, L'"');
    if (!end) return FALSE;

    int len = (int)(end - start);
    if (len >= outSize) len = outSize - 1;

    wcsncpy_s(out, outSize, start, len);
    out[len] = 0;

    return TRUE;
}




void SearchJson(const wchar_t* folderPath, TAB_DATA* tab, FOUND_LIST* results)
{
    wchar_t jsonPath[MAX_PATH];
    swprintf_s(jsonPath, MAX_PATH, L"%s\\List.json", folderPath);

    FILE* file;
    if (_wfopen_s(&file, jsonPath, L"r, ccs=UTF-8") != 0 || !file)
    {
        MessageBox(NULL, L"Could not open List.json", L"Error", MB_OK);
        return;
    }

    wchar_t line[512];

    while (fgetws(line, 512, file))
    {
        wchar_t name[256] = L"";
        wchar_t date[256] = L"";
        wchar_t id[256] = L"";

        ExtractJsonValue(line, L"name", name, 256);
        ExtractJsonValue(line, L"date", date, 256);
        ExtractJsonValue(line, L"id", id, 256);

        for (int i = 0; i < tab->fieldCount; i++)
        {
            wchar_t text[256];
            FIELD_DATA* f = &tab->fields[i];
            if (!f->hControl) continue;

            GetWindowTextW(f->hControl, text, 256);

            if (FilenameContains(name, text) || FilenameContains(id, text))
            {

                if (AlreadySeen(seenDates, seenCount, date))
                    break;


                wchar_t fullPath[MAX_PATH];

                swprintf_s(fullPath, MAX_PATH, L"%s\\%s.pdf", folderPath, date);

                if (GetFileAttributesW(fullPath) != INVALID_FILE_ATTRIBUTES)
                {
                    AddFound(results, fullPath);

                    wcscpy_s(seenDates[seenCount], 64, date);
                    seenCount++;
                }
                else {
                    MessageBox(g_pdfFrame, L"Could not find file", L"Error", MB_ICONERROR);
                }

                break;
            }
        }


    }

    fclose(file);
}





BOOL FilenameContains(const wchar_t* filename, const wchar_t* text)
{
    if (!text[0]) return FALSE;

    wchar_t f[MAX_PATH], t[256];

    wcscpy_s(f, MAX_PATH, filename);
    wcscpy_s(t, 256, text);

    _wcslwr_s(f, MAX_PATH);
    _wcslwr_s(t, 256);

    return wcsstr(f, t) != NULL;
}


void SearchFolder(
    const wchar_t* folderTemplate,
    TAB_DATA* tab,
    FOUND_LIST* results
)
{


    FIELD_VALUE fieldValues[64];
    int fieldCount = 0;

    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (!f->hControl) continue;

        wcscpy_s(fieldValues[fieldCount].label, 64, f->label);
        GetWindowTextW(f->hControl, fieldValues[fieldCount].value, 256);
        fieldCount++;
    }

    wchar_t expandedFolder[512];
    ExpandTemplate(folderTemplate, expandedFolder, 512, fieldValues, fieldCount);

    wchar_t pattern[MAX_PATH];
    swprintf_s(pattern, MAX_PATH, L"%s\\*", expandedFolder);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBox(hSearchWnd, expandedFolder, L"Folder not found", MB_OK);
        return;
    }

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        for (int i = 0; i < tab->fieldCount; i++) {

            wchar_t text[256];
            FIELD_DATA* f = &tab->fields[i];
            if (!f->hControl) continue;

            GetWindowTextW(f->hControl, text, 256);

            if (FilenameContains(fd.cFileName, text)) {

                wchar_t fullPath[MAX_PATH];
                swprintf_s(fullPath, MAX_PATH, L"%s\\%s", expandedFolder, fd.cFileName);

                AddFound(results, fullPath);
                break;
            }
        }

    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
}

void SearchFolderRecursive(
    const wchar_t* folder,
    FIELD_VALUE* fieldValues,
    int fieldCount,
    FOUND_LIST* results
)
{
    wchar_t pattern[MAX_PATH];
    swprintf_s(pattern, MAX_PATH, L"%s\\*", folder);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);


    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, pattern, L"No file found", MB_OK);
        return;
    }

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 ||
            wcscmp(fd.cFileName, L"..") == 0)
            continue;

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            continue;

        wchar_t fullPath[MAX_PATH];
        swprintf_s(fullPath, MAX_PATH, L"%s\\%s", folder, fd.cFileName);


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            SearchFolderRecursive(fullPath, fieldValues, fieldCount, results);
        }
        else
        {
            for (int i = 0; i < fieldCount; i++)
            {
                if (wcslen(fieldValues[i].value) == 0)
                    continue;


                if (FilenameContains(fd.cFileName, fieldValues[i].value))
                {
                    AddFound(results, fullPath);
                    break;
                }
            }
        }

    } while (FindNextFileW(hFind, &fd));


    FindClose(hFind);
}

void SearchFromIniPaths(int currentPage, FOUND_LIST* results, TAB_DATA* tabs)
{
    TAB_DATA* tab = &tabs[currentPage];


    FIELD_VALUE fieldValues[64];
    int fieldCount = 0;

    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (!f->hControl) continue;

        wcscpy_s(fieldValues[fieldCount].label, 64, f->label);
        GetWindowTextW(f->hControl, fieldValues[fieldCount].value, 256);
        fieldCount++;
    }


    wchar_t jsonFlag[8];
    GetPrivateProfileStringW(
        tab->iniSection,
        L"JsonSearch",
        L"false",
        jsonFlag,
        8,
        INI_SEARCH
    );

    BOOL useJsonSearch = (_wcsicmp(jsonFlag, L"true") == 0);



    for (int p = 0; p < 10; p++) {

        wchar_t key[32];
        swprintf_s(key, 32, L"Path%d", p);

        wchar_t folder[512];
        GetPrivateProfileStringW(
            tab->iniSection,
            key,
            L"",
            folder,
            512,
            INI_SEARCH
        );

        if (!folder[0])
            break;

        wchar_t expandedFolder[512];
        ExpandTemplate(folder, expandedFolder, 512, fieldValues, fieldCount);



        if (useJsonSearch)
        {
            SearchJson(expandedFolder, tab, results);
        }
        else
        {
            if (tab->searchRecursive)
            {
                SearchFolderRecursive(expandedFolder, fieldValues, fieldCount, results);
            }
            else
            {
                SearchFolder(expandedFolder, tab, results);
            }
        }



    }
}


void FindFilesFromTab(int currentPage, TAB_DATA* tabs)
{

    ClearSearchResults();

    foundResults = calloc(1, sizeof(FOUND_LIST));
    if (!foundResults) return;

    SearchFromIniPaths(currentPage, foundResults, tabs);



    if (foundResults->count == 0) {
        MessageBoxW(NULL, L"No files found", L"Search", MB_OK | MB_ICONWARNING);
        ClearSearchResults();
        return;
    }

    g_SearchActive = TRUE;

    g_CurrentResultIndex = 0;

    LoadNewPDF(foundResults->items[0].fullPath);
    



    for (int i = 1; i < foundResults->count; i++) {
        OutputDebugStringW(foundResults->items[i].fullPath);
        OutputDebugStringW(L"\n");
    }

}






































void getCurrentPDFPage() {

}





void SaveLastPath(LPCWSTR src, LPCWSTR dest, LPCWSTR iniPath)
{
    WritePrivateProfileStringW(L"Undo", L"LastSrc", src, iniPath);
    WritePrivateProfileStringW(L"Undo", L"LastDest", dest, iniPath);
}

void LoadLastPath(wchar_t* src, wchar_t* dest, DWORD size, LPCWSTR iniPath)
{
    GetPrivateProfileStringW(L"Undo", L"LastSrc", L"", src, size, iniPath);
    GetPrivateProfileStringW(L"Undo", L"LastDest", L"", dest, size, iniPath);
}

void RemoveLastPath(LPCWSTR iniPath) {
    WritePrivateProfileStringW(L"Undo", L"LastSrc", NULL, iniPath);
    WritePrivateProfileStringW(L"Undo", L"LastDest", NULL, iniPath);

}

void DestroyActiveFields(void) {



    for (int i = 0; i < activeFieldCount; i++)
    {
        if (fieldLabels[i])   DestroyWindow(fieldLabels[i]);
        if (fieldControls[i]) DestroyWindow(fieldControls[i]);
        if (fieldPlusButtons[i])  DestroyWindow(fieldPlusButtons[i]);
        if (fieldMinusButtons[i])  DestroyWindow(fieldMinusButtons[i]);

      
    }
    activeFieldCount = 0;
}

void DestroyTabButtons(TAB_DATA* tab)
{
    for (int i = 0; i < tab->buttonCount; i++)
    {
        BUTTON_DATA* b = &tab->buttons[i];

        if (b->hWnd && IsWindow(b->hWnd))
        {
            DestroyWindow(b->hWnd);
            b->hWnd = NULL;
        }
    }
}

FIELD_DATA* FindFieldByHwnd(HWND hCtrl, TAB_DATA* tabs, int tabCount) {
    for (int t = 0; t < tabCount; t++) {
        TAB_DATA* tab = &tabs[t];
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
    default:;
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




u_int LoadTabCount(LPCWSTR iniPath)
{
    return GetPrivateProfileIntW(
        L"Tabs",
        L"Count",
        0,
        iniPath
    );
}
bool LoadButtonsData(const wchar_t* tabSection, TAB_DATA* tab, LPCWSTR iniPath)
{
    tab->buttonCount = GetPrivateProfileIntW(tabSection, L"ButtonCount", 0, iniPath);

    if (tab->buttonCount <= 0)
        return false;

    for (int i = 0; i < tab->buttonCount; i++)
    {
        BUTTON_DATA* b = &tab->buttons[i];
        wchar_t key[64];

        swprintf(key, 64, L"Button%d.X", i);
        b->x = GetPrivateProfileIntW(tabSection, key, -1, iniPath);

        swprintf(key, 64, L"Button%d.Y", i);
        b->y = GetPrivateProfileIntW(tabSection, key, -1, iniPath);

        if (b->x < 0 || b->y < 0)
            continue;

        swprintf(key, 64, L"Button%d.Width", i);
        b->width = GetPrivateProfileIntW(tabSection, key, 80, iniPath);

        swprintf(key, 64, L"Button%d.Height", i);
        b->height = GetPrivateProfileIntW(tabSection, key, 25, iniPath);

        swprintf(key, 64, L"Button%d.Text", i);
        GetPrivateProfileStringW(tabSection, key, L"Button", b->text, 64, iniPath);

        wchar_t func[64];
        swprintf(key, 64, L"Button%d.Function", i);
        GetPrivateProfileStringW(tabSection, key, L"Save", func, 64, iniPath);

        if (wcscmp(func, L"Save") == 0) b->function = BTN_SAVE;
        else if (wcscmp(func, L"Search") == 0) b->function = BTN_SEARCH;
        else if (wcscmp(func, L"OpenList") == 0) b->function = BTN_OPEN_LIST;
    }

    return true;
}



void PopulateControlData(const FIELD_DATA* f, LPCWSTR iniPath)
{
    if (!f->hControl || f->controlType != FIELD_COMBO)
        return;

    SendMessage(f->hControl, CB_RESETCONTENT, 0, 0);

    if (wcslen(f->sourceName) == 0)
        return;

    wchar_t buffer[2048];
    GetPrivateProfileSectionW(f->sourceName, buffer, 2048, iniPath);

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






void LoadTabFields(int tabIndex, const wchar_t* section, LPCWSTR iniPath, TAB_DATA* tabs)
{
    TAB_DATA* tab = &tabs[tabIndex];

    tab->fieldCount = GetPrivateProfileIntW(section, L"FieldCount", 0, iniPath);


    int val = GetPrivateProfileIntW(section, L"SearchRecursive", 0, iniPath);
    tab->searchRecursive = val != 0;


    for (int f = 0; f < tab->fieldCount; f++)
    {
        FIELD_DATA* field = &tab->fields[f];

        wchar_t key[64];

        swprintf_s(key, 64, L"Field%d.Label", f);
        GetPrivateProfileStringW(section, key, L"",
            field->label, 64, iniPath);

        swprintf_s(key, 64, L"Field%d.Control", f);
        wchar_t ctrl[32];
        GetPrivateProfileStringW(section, key, L"",
            ctrl, 32, iniPath);

        swprintf_s(key, 64, L"Field%d.Placeholder", f);
        GetPrivateProfileStringW(section, key, L"", field->placeholder, 32, iniPath);

        swprintf_s(key, 64, L"Field%d.X", f);
        field->x = GetPrivateProfileIntW(section, key, -1, iniPath);

        swprintf_s(key, 64, L"Field%d.Y", f);
        field->y = GetPrivateProfileIntW(section, key, -1, iniPath);

        swprintf_s(key, 64, L"Field%d.Width", f);
        field->width = GetPrivateProfileIntW(section, key, -1, iniPath);

        swprintf_s(key, 64, L"Field%d.Height", f);
        field->height = GetPrivateProfileIntW(section, key, -1, iniPath);


        swprintf_s(key, 64, L"Field%d.SkipRecent", f);
        field->skipRecent = GetPrivateProfileIntW(section, key, 0, iniPath) != 0;




        if (!_wcsicmp(ctrl, L"EDIT")) field->controlType = FIELD_EDIT;
        else if (!_wcsicmp(ctrl, L"COMBO")) field->controlType = FIELD_COMBO;
        else field->controlType = FIELD_LABEL;

        if (field->controlType == FIELD_COMBO && !field->skipRecent) {
            swprintf_s(key, 64, L"Field%d.LastValue", f);
            GetPrivateProfileStringW(
                section,
                key,
                L"",
                field->lastComboValue,
                256,
                iniPath
            );
        }

        swprintf_s(key, 64, L"Field%d.Source", f);
        GetPrivateProfileStringW(section, key, L"",
            field->sourceName, 64, iniPath);
    }
}





void LoadTabsFromIni(HWND hTab, LPCWSTR iniPath, TAB_DATA* tabs, int* tabCount)
{
    TabCtrl_DeleteAllItems(hTab);

    *tabCount = LoadTabCount(iniPath);



    for (int i = 0; i < *tabCount; i++)
    {
        wchar_t section[16];
        swprintf_s(section, 16, L"Tab%d", i);

        swprintf_s(tabs[i].iniSection, 64, L"Tab%d", i);


        GetPrivateProfileStringW(section, L"Name", L"Unnamed",
            tabs[i].name, 64, iniPath);



        LoadTabFields(i, section, iniPath, tabs);

        TCITEM tie = { 0 };
        tie.mask = TCIF_TEXT;
        tie.pszText = tabs[i].name;
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}





void CreateFieldsFromTab(HWND parent, TAB_DATA* tab, LPCWSTR iniPath, BOOL showAddButtons, BOOL showListboxButton)
{
    DestroyActiveFields();

    int y = 50;
    int xCtrl = 50;

    int widthCtrl = 150;
    int heightCtrl = 20;


    RECT rc;
    GetClientRect(parent, &rc);
    TabCtrl_AdjustRect(parent, FALSE, &rc);




    int baseX = rc.left;
    int baseY = rc.top;



    for (int i = 0; i < tab->fieldCount; i++)
    {
        FIELD_DATA* f = &tab->fields[i];

        int drawX = baseX + ((f->x != -1) ? f->x : xCtrl);
        int drawY = baseY + ((f->y != -1) ? f->y : y);

        int drawWidth = (f->width != -1) ? f->width : widthCtrl;
        int drawHeight = (f->height != -1) ? f->height : heightCtrl;


        swprintf_s(key, 64, L"Field%d.SkipRecent", i);
        f->skipRecent = GetPrivateProfileIntW(tab->iniSection, key, 0, iniPath) != 0;



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


            if (showAddButtons) {
                int btnHeight = 15;
                int btnWidth = 20;
                int spacing = 5;

                f->hButtonPlus = CreateWindowW(
                    L"BUTTON",
                    L"+",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    drawX + drawWidth - btnWidth,
                    drawY - 20,
                    btnWidth,
                    btnHeight,
                    parent,
                    (HMENU)(INT_PTR)(3500 + i),
                    g_hInstance,
                    NULL
                );

                SetWindowLongPtr(f->hButtonPlus, GWLP_USERDATA, (LONG_PTR)f);



                f->hButtonMinus = CreateWindowW(
                    L"BUTTON",
                    L"-",
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    drawX + drawWidth - btnWidth - spacing - btnWidth,
                    drawY - 20,
                    btnWidth,
                    btnHeight,
                    parent,
                    (HMENU)(INT_PTR)(3580 + i),
                    g_hInstance,
                    NULL
                );

                SetWindowLongPtr(f->hButtonMinus, GWLP_USERDATA, (LONG_PTR)f);


                if (showListboxButton) {
                    f->hListboxButton = CreateWindowW(
                        L"BUTTON",
                        L"Add To List",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        drawX + drawWidth - btnWidth - spacing - btnWidth,
                        drawY + 50,
                        140,
                        35,
                        parent,
                        (HMENU)IDC_SHOW_LISTBOX,
                        g_hInstance,
                        NULL
                    );
                }


            }

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
        default:;
        }



        if (f->controlType == FIELD_COMBO)
        {
            PopulateControlData(f, iniPath);
        }

        fieldControls[i] = f->hControl;
        fieldPlusButtons[i] = f->hButtonPlus;
        fieldMinusButtons[i] = f->hButtonMinus;



        activeFieldCount++;
    }
}
void CreateButtons(HWND parent, int pageIndex, LPCWSTR iniPath, TAB_DATA* tabs)
{
    TAB_DATA* tab = &tabs[pageIndex];

    if (!LoadButtonsData(tab->iniSection, tab, iniPath))
        return;

    RECT rc;
    GetClientRect(parent, &rc);
    TabCtrl_AdjustRect(parent, FALSE, &rc);

    for (int i = 0; i < tab->buttonCount; i++)
    {
        BUTTON_DATA* b = &tab->buttons[i];

        int drawX = rc.left + b->x;
        int drawY = rc.top + b->y;

        b->id = 2000 + (pageIndex * 100) + i;

        b->hWnd = CreateWindowW(
            L"BUTTON",
            b->text,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            drawX, drawY,
            b->width, b->height,
            parent,
            (HMENU)(INT_PTR)b->id,
            g_hInstance,
            NULL
        );
    }

    tab->iniPath = iniPath;
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




void SaveFile(int currentPage, TAB_DATA* tab)
{
    if (currentPage < 0 || currentPage >= SaveTabCount)
        return;



    for (int i = 0; i < tab->fieldCount; i++)
    {
        FIELD_DATA* f = &tab->fields[i];

        if (f->controlType == FIELD_COMBO)
        {
            int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);

            if (sel != CB_ERR)
            {
                SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);

                int fieldIndex = i; 

                wchar_t key[64];
                swprintf_s(key, 64, L"Field%d.LastValue", fieldIndex);

                WritePrivateProfileStringW(
                    tab->iniSection,
                    key,
                    f->lastComboValue,
                    INI_SAVE
                );
            }
        }
    }

    FIELD_VALUE fieldValues[64];
    int fieldCount = 0;
    wchar_t firstEditValue[256] = L"";
    wchar_t fullPath[MAX_PATH] = L"";



    for (int i = 0; i < tab->fieldCount && fieldCount < 64; i++)
    {
        FIELD_DATA* f = &tab->fields[i];
        if (!f->hControl) continue;

        wcscpy_s(fieldValues[fieldCount].label, 64, f->label);
        GetWindowTextW(f->hControl, fieldValues[fieldCount].value, 256);

        if (f->controlType == FIELD_EDIT && firstEditValue[0] == L'\0')
        {
            wcscpy_s(firstEditValue, 256, fieldValues[fieldCount].value);
        }

        fieldCount++;
    }


    wchar_t oldFile[MAX_PATH];
    swprintf_s(oldFile, MAX_PATH, L"C:\\watchFolder\\%s", g_CurrentFilename);

    if (GetFileAttributesW(oldFile) == INVALID_FILE_ATTRIBUTES)
    {
        MessageBox(NULL, L"Source file not found!", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }

    wchar_t filePattern[256], finalFile[256];

    GetPrivateProfileStringW(tab->iniSection, L"SavedFileName", L"", filePattern, 256, INI_SAVE);

    if (filePattern[0] == 0)
    {
        MessageBox(NULL, L"No SavedFileName defined!", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }

    ExpandTemplate(filePattern, finalFile, 256, fieldValues, fieldCount);
    SanitizeFilename(finalFile);

    BOOL copiedAtLeastOnce = FALSE;

    for (int pnum = 1; pnum <= 10; pnum++)
    {
        wchar_t pathKey[32];
        wchar_t folderTemplate[512];
        wchar_t expandedFolder[512];

        swprintf_s(pathKey, 32, L"Path%d", pnum);

        GetPrivateProfileStringW(tab->iniSection, pathKey, L"", folderTemplate, 512, INI_SAVE);

        if (folderTemplate[0] == 0)
            break;

        ExpandTemplate(folderTemplate, expandedFolder, 512, fieldValues, fieldCount);

        size_t len = wcslen(expandedFolder);
        if (len > 0 && expandedFolder[len - 1] != L'\\')
        {
            wcscat_s(expandedFolder, 512, L"\\");
        }

        wchar_t tempPath[MAX_PATH] = L"";
        size_t folderLen = wcslen(expandedFolder);

        for (size_t j = 0; j < folderLen; j++)
        {
            tempPath[j] = expandedFolder[j];
            tempPath[j + 1] = 0;

            if (expandedFolder[j] == L'\\')
            {
                if (GetFileAttributesW(tempPath) == INVALID_FILE_ATTRIBUTES)
                {
                    CreateDirectoryW(tempPath, NULL);
                }
            }
        }

        swprintf_s(fullPath, MAX_PATH, L"%s%s", expandedFolder, finalFile);

        MakeUniqueFilename(fullPath, fullPath);

        if (CopyFileW(oldFile, fullPath, FALSE))
        {
            copiedAtLeastOnce = TRUE;
            SaveLastPath(oldFile, fullPath, INI_SAVE);
        }
        else
        {
            DWORD err = GetLastError();

            wchar_t buf[512];
            swprintf_s(buf, 512, L"Failed to copy file to:\n%ls\nError %lu", fullPath, err);

            MessageBox(NULL, buf, L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        }
    }

    if (copiedAtLeastOnce)
    {

        SaveToJson(firstEditValue, fullPath);

        DeleteFileW(oldFile);
        MessageBox(NULL, L"File Saved Successfully!", L"Success", MB_OK | MB_TOPMOST);
    }
}



bool IsFileSendReady(int currentPage) {
    if (currentPage < 0 || currentPage >= SaveTabCount)
        return false;

    TAB_DATA* tab = &SaveTabs[currentPage];

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


static int g_currentPageGuide = -1;

void SetPage(
    HWND hTab,
    HWND hwndParent,
    TAB_DATA* tabs,
    int pageCount,
    int newPage,
    LPCWSTR iniPath,
    int buttonId,
    bool showAddButtons
    )
{
    if (newPage < 0 || newPage >= pageCount)
        return;

    if (g_currentPageGuide != -1)
    {
        DestroyTabButtons(&tabs[g_currentPageGuide]);
    }

    TabCtrl_SetCurSel(hTab, newPage);

    DestroyActiveFields();

    BOOL showListboxButton = 0;

    CreateFieldsFromTab(hwndParent, &tabs[newPage], iniPath, showAddButtons, showListboxButton);
    CreateButtons(hwndParent, newPage, iniPath, tabs);

    TAB_DATA* tab = &tabs[newPage];

    for (int i = 0; i < tab->fieldCount; i++)
        PopulateControlData(&tab->fields[i], iniPath);

    g_currentPageGuide = newPage;

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
                WCHAR filename[256] = { 0 };
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


LRESULT CALLBACK LabelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);

        DefWindowProc(hwnd, WM_PAINT, (WPARAM)hdc, 0);

        EndPaint(hwnd, &ps);

        return CallWindowProc(g_oldLabelProc, hwnd, msg, 0, 0);


    }


    default:
        return CallWindowProc(g_oldLabelProc, hwnd, msg, wParam, lParam);
    }
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_APP + 1: {

        if (g_pdfFrame && IsWindow(g_pdfFrame))
        {
            MessageBox(hwnd,
                L"Search window is open.\nClose it before continuing?",
                L"Confirm",
                MB_OK | MB_ICONERROR | MB_TOPMOST);

            SetForegroundWindow(hSearchWnd);
            return 0;
        }


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
            AppendMenu(menu, MF_SEPARATOR, 0, NULL);
            AppendMenu(menu, MF_STRING, ID_TRAY_UNDO, L"Undo");
            AppendMenu(menu, MF_SEPARATOR, 0, NULL);
            AppendMenu(menu, MF_STRING, ID_TRAY_SEARCH_SETTINGS, L"Search Settings");
            AppendMenu(menu, MF_STRING, ID_TRAY_SAVE_SETTINGS, L"Save Settings");
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
            int result = MessageBox(
                hwnd,
                L"Are you sure you want to exit?",
                L"Confirm Exit",
                MB_YESNO | MB_ICONQUESTION
            );

            if (result == IDYES)
            {
                DestroyWindow(hwnd);
                PostQuitMessage(0);
            }
            return 0;
        case ID_TRAY_SAVE_SETTINGS:
            OpenSaveSettings(hwnd);
            break;
        case ID_TRAY_SEARCH_SETTINGS:
            OpenSearchSettings(hwnd);
            break;
        case ID_TRAY_SEARCH:

            if (hPopupWnd && IsWindow(hPopupWnd))
            {
                MessageBox(
                    hwnd,
                    L"Save window is currently open.\nClose it before opening Search.",
                    L"Operation Not Allowed",
                    MB_OK | MB_ICONERROR | MB_TOPMOST
                );

                SetForegroundWindow(hPopupWnd);
                return 0;

            }

            OpenSearchWindow(hwnd);
            break;
        case ID_TRAY_UNDO:

            int undoresult = MessageBox(
                hwnd,
                L"Are you sure you want to undo?",
                L"Confirm Undo",
                MB_YESNO | MB_ICONQUESTION
            );

            if (undoresult -= IDYES)
            {
                return 0;
            }

            wchar_t lastSrc[MAX_PATH];
            wchar_t lastDest[MAX_PATH];

            LoadLastPath(lastSrc, lastDest, MAX_PATH, INI_SAVE);

            if (lastSrc[0] && lastDest[0]) {
                MoveFileExW(lastDest, lastSrc, MOVEFILE_REPLACE_EXISTING);
            }

            else {
                MessageBox(hwnd, L"No items to undo.", L"Error", MB_OK);
            }



            break;


        default:;
        }
        break;


    case WM_DESTROY:
        RemoveLastPath(INI_SAVE);
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
    switch (msg) {
    case WM_CLOSE:

        ClearSearchResults();

        if (doc) { fz_drop_document(pdf_ctx, doc); doc = NULL; fz_empty_store(pdf_ctx); }
        if (g_pdfBitmap) { DeleteObject(g_pdfBitmap); g_pdfBitmap = NULL; }
        if (g_pdfMemDC) { DeleteDC(g_pdfMemDC); g_pdfMemDC = NULL; }

        scrollY = 0;
        maxScroll = 0;
        pdfWidth = 0;
        pdfHeight = 0;
        total_pages = 0;
        current_pdf_page = 0;
        g_SearchActive = FALSE;

        current_opened_pdf = L"No PDF loaded";

        if (g_toolbar) InvalidateRect(g_toolbar, NULL, TRUE);
        if (g_pictureFrame) InvalidateRect(g_pictureFrame, NULL, TRUE);

        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        DestroyActiveFields();

        current_opened_pdf = NULL;
        current_opened_pdf_full = NULL;
        doc = NULL;

        g_pdfFrame = NULL;
        g_pictureFrame = NULL;
        hSearchTab = NULL;
        frame = NULL;


        return 0;




    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->hwndFrom == hSearchTab && pnmh->code == TCN_SELCHANGE)
        {
            ClearSearchResults();

            int newPage = TabCtrl_GetCurSel(hSearchTab);
            g_CurrentPage = newPage;

            SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, newPage, INI_SEARCH, IDC_SEARCH_BUTTON, FALSE);
        }
        break;

    }


    case WM_KEYDOWN:
    {
        if (wParam == VK_TAB)
        {
            HWND current = GetFocus();

            if (!current)
                current = hwnd;

            HWND root = GetAncestor(current, GA_ROOT);

            if (!root)
                root = hwnd;

            BOOL backwards = (GetKeyState(VK_SHIFT) & 0x8000);


            HWND parent = GetParent(current);
            HWND next = GetNextDlgTabItem(parent, current, backwards);

            if (next)
                SetFocus(next);

            return 0;
        }

        if (wParam == VK_RETURN)
        {
            HWND focused = GetFocus();

            if (!focused)
                focused = hwnd;

            int activeTab = TabCtrl_GetCurSel(hSearchTab);

            TAB_DATA* tab = &SearchTabs[activeTab];

            for (int i = 0; i < tab->fieldCount; i++)
            {
                FIELD_DATA* field = &tab->fields[i];
                if (field->controlType == FIELD_EDIT && field->hControl == focused)
                {
                    ClearSearchResults();


                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SEARCH_BUTTON, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_SEARCH_BUTTON));
                    return 0;
                }
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);




        TAB_DATA* tab = &SearchTabs[g_CurrentPage];

        for (int i = 0; i < tab->buttonCount; i++)
        {
            BUTTON_DATA* b = &tab->buttons[i];


            if (id == b->id)
            {
                switch (b->function)
                {
                case BTN_SAVE:


                    break;

                case BTN_SEARCH:


                    



                    ClearSearchResults();




                    int activeTab = TabCtrl_GetCurSel(hSearchTab);

                    FindFilesFromTab(activeTab, SearchTabs);

                    ClearTabFields(activeTab, SearchTabs);


                    WCHAR searchCountText[64];

                    if (!foundResults || foundResults->count == 0)
                    {
                        g_CurrentResultIndex = -1;

                        swprintf_s(searchCountText, 64, L"Result   0 / 0");
                    }
                    else
                    {
                        if (g_CurrentResultIndex < 0)
                            g_CurrentResultIndex = 0;

                        swprintf_s(searchCountText, 64, L"Result   %d / %d",
                            g_CurrentResultIndex + 1,
                            foundResults->count);
                    }

                    SetWindowTextW(hSearchCountLabel, searchCountText);


                    WCHAR pageText[64];
                    swprintf_s(pageText, 64, L"Page   %d / %d", current_pdf_page + 1, total_pages);
                    SetWindowTextW(hPageLabel, pageText);



                    FIELD_DATA* field = &SearchTabs[activeTab].fields[0];
                    if (field->controlType == FIELD_EDIT && field->hControl)
                    {
                        SetFocus(field->hControl);
                    }

                    break;

                case BTN_OPEN_LIST:
                    break;
                }
                return 0;
            }
        }









        switch (id)
        {

        case CBN_SELCHANGE:
        {
            FIELD_DATA* f = FindFieldByHwnd((HWND)lParam, SearchTabs, SearchTabCount);
            if (f && f->controlType == FIELD_COMBO && !f->skipRecent)
            {
                int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR)
                {
                    SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);
                    f->userChanged = TRUE;

                }
            }
            break;
        }
        }

        break;
    }



    case WM_SIZE:
        RECT rc;
        GetClientRect(hwnd, &rc);

        MoveWindow(
            g_pictureFrame,
            800,
            55,
            650,
            rc.bottom - 55,
            TRUE
        );
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
    break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    int currentPage = g_CurrentPage;
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        free(g_customerList.items);
        g_customerList.items = NULL;
        g_customerList.count = 0;
        hPopupWnd = NULL;
        break;


    case WM_KEYDOWN:
    {
        if (wParam == VK_TAB)
        {
            HWND current = GetFocus();

            if (!current)
                current = hwnd;


            BOOL backwards = (GetKeyState(VK_SHIFT) & 0x8000);

            HWND next = GetNextDlgTabItem(hwnd, current, backwards);

            if (next)
                SetFocus(next);

            return 0;
        }

        else
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }


    }


    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->hwndFrom == hPopupTab && pnmh->code == TCN_SELCHANGE)
        {
            int newPage = TabCtrl_GetCurSel(hPopupTab);
            g_CurrentPage = newPage;

            SetPage(hPopupTab, hPopupWnd, SaveTabs, SaveTabCount, newPage, INI_SAVE, IDC_SAVE_BUTTON, TRUE);
        }
    }
    break;
    case WM_COMMAND:

        int id = LOWORD(wParam);



        TAB_DATA* tab = &SaveTabs[g_CurrentPage];

        for (int i = 0; i < tab->buttonCount; i++)
        {
            BUTTON_DATA* b = &tab->buttons[i];


            if (id == b->id)
            {
                switch (b->function)
                {
                case BTN_SAVE:


                    SaveFile(currentPage, tab);


                    DestroyWindow(hwnd);


                    break;

                case BTN_SEARCH:
                    break;

                case BTN_OPEN_LIST:
                    OpenCustomerPopup(hwnd);
                    break;
                }
                return 0;
            }
        }




        if (id >= 3500 && id < 3540)
        {
            HWND btn = (HWND)lParam;

            FIELD_DATA* f = (FIELD_DATA*)GetWindowLongPtr(btn, GWLP_USERDATA);

            if (!f)
                return 0;

            wchar_t name[128];

            if (InputBox(hwnd, g_hInstance, name, 128))
            {
                ModifyNameSorted(f->sourceName, name, false); 
                PopulateControlData(f, INI_SAVE);   
            }

            return 0;
        }

        if (id >= 3545 && id < 3600)
        {
            HWND btn = (HWND)lParam;

            FIELD_DATA* f = (FIELD_DATA*)GetWindowLongPtr(btn, GWLP_USERDATA);

            if (!f)
                return 0;

            int sel = (int)SendMessageW(f->hControl, CB_GETCURSEL, 0, 0);

            if (sel != CB_ERR)
            {
                wchar_t itemText[256];
                SendMessageW(f->hControl, CB_GETLBTEXT, sel, (LPARAM)itemText);

                wchar_t msg[300];
                swprintf_s(msg, 300, L"Would you like to remove \"%s\"?", itemText);

                if (MessageBoxW(hwnd, msg, L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                    SendMessageW(f->hControl, CB_DELETESTRING, sel, 0);
                    ModifyNameSorted(f->sourceName, itemText, true);
                    PopulateControlData(f, INI_SAVE);
                }
            }
        }




        switch (LOWORD(wParam)) {


        


        case CBN_SELCHANGE: {
            FIELD_DATA* f = FindFieldByHwnd((HWND)lParam, SaveTabs, SaveTabCount);
            if (f && f->controlType == FIELD_COMBO && !f->skipRecent) {
                int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);
                    f->userChanged = TRUE;

                    int activeTab = TabCtrl_GetCurSel(hPopupTab);
                    TAB_DATA* tab = &SaveTabs[activeTab];

                    int fieldIndex = (int)(f - &tab->fields[0]);

                    wchar_t key[64];
                    swprintf_s(key, 64, L"Field%d.LastValue", fieldIndex);
                    WritePrivateProfileStringW(
                        SaveTabs[g_CurrentPage].iniSection,
                        key,
                        f->lastComboValue,
                        INI_SAVE
                    );
                }
            }

            break;
        }

        default:;
        }
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
    return 0;

}



LRESULT CALLBACK OptionsPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
    {
        HWND hwndParent = GetParent(hwnd);
        SendMessage(hwndParent, WM_COMMAND, wParam, lParam);
    }
    return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}



LRESULT CALLBACK PrintWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    PRINT_SETTINGS* settings = (PRINT_SETTINGS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);


    switch (msg)
    {

    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY:
    {

        g_PrintFrame = NULL;


        if (settings)
            free(settings);


        if (g_pdfFrame) {
            EnableWindow(g_pdfFrame, TRUE);
            SetForegroundWindow(g_pdfFrame);
        }

        return 0;
    }



    case WM_CREATE:
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        settings = (PRINT_SETTINGS*)cs->lpCreateParams;

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)settings);



        gOptionsBrush = CreateSolidBrush(RGB(240, 240, 240));
        gPreviewBrush = CreateSolidBrush(RGB(220, 220, 220));

        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = OptionsPanelProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"OptionsPanel";
        wc.hbrBackground = gOptionsBrush;

        RegisterClass(&wc);


        WNDCLASS wc1 = { 0 };
        wc1.lpfnWndProc = DefWindowProc;
        wc1.hInstance = GetModuleHandle(NULL);
        wc1.lpszClassName = L"PreviewPanel";
        wc1.hbrBackground = gPreviewBrush;

        RegisterClass(&wc1);


        hOptionsPanel = CreateWindowW(
            L"OptionsPanel",
            NULL,
            WS_CHILD | WS_VISIBLE,
            0, 0, 250, 600,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );





        hCombo = CreateWindow(
            L"COMBOBOX",
            NULL,
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            20, 20, 150, 200,
            hOptionsPanel,
            (HMENU)1001,
            NULL,
            NULL
        );

        PopulatePrinterCombo(hCombo);






        // Copies
        CreateWindowW(
            L"STATIC",
            L"Copies:",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            20, 60, 80, 20,
            hOptionsPanel,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );

        hCopiesEdit = CreateWindowW(
            L"EDIT",
            L"1",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | WS_TABSTOP,
            100, 60, 60, 22,
            hOptionsPanel,
            (HMENU)IDC_COPIES_EDIT,
            GetModuleHandle(NULL),
            NULL
        );

        // Page Count
        hAllPagesRadio = CreateWindowW(
            L"BUTTON",
            L"All Pages",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,
            20, 100, 120, 20,
            hOptionsPanel,
            (HMENU)IDC_ALL_PAGES,
            GetModuleHandle(NULL),
            NULL
        );

        hCustomPagesRadio = CreateWindowW(
            L"BUTTON",
            L"Custom Pages",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP,
            20, 130, 120, 20,
            hOptionsPanel,
            (HMENU)IDC_CUSTOM_PAGES,
            GetModuleHandle(NULL),
            NULL
        );





        // Start Pages
        CreateWindowW(
            L"STATIC",
            L"From:",
            WS_CHILD | WS_VISIBLE,
            40, 190, 40, 20,
            hOptionsPanel,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );

        hStartPageEdit = CreateWindowW(
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | WS_TABSTOP,
            80, 190, 50, 22,
            hOptionsPanel,
            (HMENU)IDC_START_PAGE,
            GetModuleHandle(NULL),
            NULL
        );


        // End Pages
        CreateWindowW(
            L"STATIC",
            L"To:",
            WS_CHILD | WS_VISIBLE,
            40, 220, 40, 20,
            hOptionsPanel,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );

        hEndPageEdit = CreateWindowW(
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | WS_TABSTOP,
            80, 220, 50, 22,
            hOptionsPanel,
            (HMENU)IDC_END_PAGE,
            GetModuleHandle(NULL),
            NULL
        );


        CreateWindowW(
            L"BUTTON",
            L"Cancel",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP | WS_TABSTOP,
            60, 510, 80, 30,
            hOptionsPanel,
            (HMENU)IDC_CANCEL_PDF,
            GetModuleHandle(NULL),
            NULL
        );

        // Print Button
        CreateWindowW(
            L"BUTTON",
            L"Print",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP | WS_TABSTOP,
            160, 510, 80, 30,
            hOptionsPanel,
            (HMENU)IDC_PRINT_PDF,
            GetModuleHandle(NULL),
            NULL
        );

       




        SendMessage(hAllPagesRadio, BM_SETCHECK, BST_CHECKED, 0);
        settings->allPages = true;
        EnableWindow(hStartPageEdit, FALSE);
        EnableWindow(hEndPageEdit, FALSE);



        // Preview Frame
        hPreviewFrame = CreateWindowW(
            L"PreviewPanel",
            NULL,
            WS_CHILD | WS_VISIBLE,
            250, 0, 550, 600,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );

        return 0;

    }

    case WM_COMMAND:
    {

        switch (LOWORD(wParam)) {

        case IDC_ALL_PAGES:
        {
            settings->allPages = true;
            EnableWindow(hStartPageEdit, FALSE);
            EnableWindow(hEndPageEdit, FALSE);
            break;
        }

        case IDC_CUSTOM_PAGES:
        {
            settings->allPages = false;
            EnableWindow(hStartPageEdit, TRUE);
            EnableWindow(hEndPageEdit, TRUE);
            break;
        }

        case IDC_PRINT_PDF:
        {
            PrintCurrentPDF(hwnd);
            break;
        }

        case IDC_CANCEL_PDF:
        {
            DestroyWindow(hwnd);
            break;
        }

        }
        
    }
    return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);

}









HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text) {

    if (hPopupWnd && IsWindow(hPopupWnd))
        return hPopupWnd;

    g_CurrentPage = 0;
    g_currentPageGuide = -1;


    const wchar_t CLASS_NAME[] = L"PopupWindowClass";


    static bool registered = false;
    if (!registered) {
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = PopupWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_CONTROLPARENT,
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
        WS_EX_CONTROLPARENT,
        WC_TABCONTROL,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_THICKFRAME | WS_BORDER,
        10, 10, 672, 405,
        hwnd,
        (HMENU)4001,
        NULL,
        NULL
    );




    LoadTabsFromIni(hPopupTab, INI_SAVE, SaveTabs, &SaveTabCount);
    SetPage(hPopupTab, hPopupWnd, SaveTabs, SaveTabCount, 0, INI_SAVE, IDC_SAVE_BUTTON, FALSE);





    if (PopupWndProc) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }


    return hwnd;
}








LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND: {

        HWND parent = GetParent(hwnd);
        if (parent)
            return SendMessage(parent, msg, wParam, lParam);
        break;
    }

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->hwndFrom == hSearchTab &&
            pnmh->code == TCN_SELCHANGE)
        {
            ClearSearchResults();


            int newPage = TabCtrl_GetCurSel(hSearchTab);
            g_CurrentPage = newPage;

            SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, newPage, INI_SEARCH, IDC_SEARCH_BUTTON, FALSE);
            return 0;
        }
    }
    break;


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
    default:;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK ToolbarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hToolbarBrush = NULL;


    switch (msg)
    {
    case WM_CREATE:
    {
        g_toolbar = hwnd;
        hToolbarBrush = CreateSolidBrush(RGB(70, 73, 79));
        break;
    }


    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

        if (dis->CtlType == ODT_BUTTON)
        {
            wchar_t text[64];
            GetWindowTextW(dis->hwndItem, text, 64);

            HBRUSH brush;

            if (dis->itemState & ODS_SELECTED)
                brush = CreateSolidBrush(RGB(90, 95, 105));
            else
                brush = CreateSolidBrush(RGB(79, 84, 94));

            FillRect(dis->hDC, &dis->rcItem, brush);
            DeleteObject(brush);

            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(255, 255, 255));

            DrawTextW(
                dis->hDC,
                text,
                -1,
                &dis->rcItem,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE
            );

            return TRUE;
        }
    }
    break;


    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;

        if (hCtrl == hPageLabel || hCtrl == hSearchCountLabel || hCtrl == hFileNameText)
        {
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(70, 73, 79));

            return (LRESULT)hToolbarBrush;
        }

        break;

    }



    case WM_PAINT:
    {



        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH bg = CreateSolidBrush(RGB(71, 73, 79));
        FillRect(hdc, &rect, bg);
        DeleteObject(bg);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        SetWindowTextW(hFileNameText, current_opened_pdf);
        LayoutCustomerNav(hwnd);



        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE:
        LayoutCustomerNav(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;


    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_LEFT_ARROW:
            PreviousResult();
            break;

        case IDC_RIGHT_ARROW:
            NextResult();
            break;

        case IDC_PRINT_BUTTON:
            OpenPrintWindow(hwnd);
            EnableWindow(g_pdfFrame, FALSE);
            break;
        }
        return 0;

    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}





void RegisterFrameClass(HINSTANCE hInstance)
{
    static bool registered = false;
    if (!registered)
    {
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = FrameWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"FrameClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wc);
        registered = true;
    }
}

void RegisterPictureFrameClass(HINSTANCE hInstance)
{
    static bool registered = false;
    if (!registered)
    {
        WNDCLASS wcChild = { 0 };
        wcChild.lpfnWndProc = PictureFrameProc;
        wcChild.hInstance = g_hInstance;
        wcChild.lpszClassName = L"PDFChild";
        wcChild.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcChild.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wcChild);
        registered = true;
    }
}

void RegisterToolbarClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (!registered)
    {
        WNDCLASS wcChild = { 0 };
        wcChild.lpfnWndProc = ToolbarProc;
        wcChild.hInstance = g_hInstance;
        wcChild.lpszClassName = L"TOOLBAR";
        wcChild.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcChild.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wcChild);
        registered = true;
    }
}

void RegisterLabelClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (!registered)
    {
        WNDCLASS wcChild = { 0 };
        wcChild.lpfnWndProc = LabelProc;
        wcChild.hInstance = g_hInstance;
        wcChild.lpszClassName = L"LABELPROC";
        wcChild.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcChild.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wcChild);
        registered = true;
    }
}





void MakeWindowRounded(HWND hwnd, int width, int height, int radius) {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
    HRGN hRgn = CreateRoundRectRgn(0, 0, width + 1, height + 1, 20, 20);
    SetWindowRgn(hwnd, hRgn, TRUE);

}


LRESULT CALLBACK PictureFrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_APP_REDRAW_PDF:
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        return 0;


    case WM_PAINT:
    {


        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);

        int viewW = client.right;
        int viewH = client.bottom;

        if (!g_pdfBitmap) {
            EndPaint(hwnd, &ps);
            return 0;
        }

        if (scrollY < 0) scrollY = 0;
        if (scrollY > pdfHeight - viewH)
            scrollY = pdfHeight - viewH;

        if (pdfHeight < viewH)
            scrollY = 0;

        BitBlt(
            hdc,
            0, 0,
            viewW, viewH,
            g_pdfMemDC,
            0, scrollY,
            SRCCOPY
        );



        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        scrollY -= delta / 2;


        int newPage = (scrollY + 10) / page_height + 1;
        if (newPage != current_pdf_page && newPage <= total_pages) {
            current_pdf_page = newPage;
            WCHAR pageText[64];
            swprintf_s(pageText, 64, L"Page %d / %d", current_pdf_page, total_pages);
            SetWindowTextW(hPageLabel, pageText);
        }


        InvalidateRect(g_pictureFrame, NULL, TRUE);


        return 0;
    }


    case WM_ERASEBKGND:
        return 1;


    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

}



HWND OpenSearchWindow(HWND hwndParent) {


    if (g_pdfFrame && IsWindow(g_pdfFrame))
    {
        SetForegroundWindow(g_pdfFrame);
        return g_pdfFrame;
    }

    g_CurrentPage = 0;


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

    screenWidth = GetSystemMetrics(SM_CXSCREEN);
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

    g_pdfFrame = hwnd;





    RegisterFrameClass(GetModuleHandle(NULL));


    frame = CreateWindowEx(
        WS_EX_CONTROLPARENT,
        L"FrameClass",
        NULL,
        WS_CHILD | WS_VISIBLE,
        100, 100, 450, 625,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );



    MakeWindowRounded(frame, 800, 600, 20);








    RegisterPictureFrameClass(GetModuleHandle(NULL));




    HWND pictureFrame = CreateWindowEx(
        0,
        L"PDFChild",
        L"PDF Viewer",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        800, 55, 650, 755,
        hwnd,
        NULL,
        g_hInstance,
        NULL
    );


    if (!pictureFrame) {
        MessageBoxW(hwnd, L"Failed to create PDF frame", L"Error", MB_OK);
    }

    g_pictureFrame = pictureFrame;

    RegisterToolbarClass(GetModuleHandle(NULL));


    HWND pictureFrameToolbar = CreateWindowEx(
        0,
        L"TOOLBAR",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        700, 10, 810, 45,
        hwnd,
        NULL,
        g_hInstance,
        NULL
    );

    HWND hToolbar = pictureFrameToolbar;


    //L"BUTTON", L"\u25C0",
    //L"BUTTON", L"\u25B6",

    hLeftArrow = CreateWindowW(
        L"BUTTON", L"<",  
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 25, 25,
        hToolbar, (HMENU)IDC_LEFT_ARROW, g_hInstance, NULL);

    hFileNameText = CreateWindowW(
        L"STATIC", L"",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        40, 0, 200, 25,
        hToolbar, (HMENU)IDC_NAME_TEXT, g_hInstance, NULL);

    hRightArrow = CreateWindowW(
        L"BUTTON", L">", 
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 25, 25,
        hToolbar, (HMENU)IDC_RIGHT_ARROW, g_hInstance, NULL);

    hPrintButton = CreateWindowW(
        L"BUTTON",
        L"Print",
        WS_CHILD | WS_VISIBLE | BS_ICON | BS_OWNERDRAW,
        120, 8, 50, 30,
        hToolbar,
        (HMENU)IDC_PRINT_BUTTON,
        g_hInstance,
        NULL
    );


    //HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));

    //if (!hIcon)
    //{
    //    MessageBox(NULL, L"Failed to load icon", L"Error", MB_OK);
    //}

    //SendMessage(hPrintButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

    hPageLabel = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        5, 12, 100, 18,
        hToolbar,
        NULL,
        g_hInstance,
        NULL
    );


    hSearchCountLabel = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        700, 12, 100, 18,
        hToolbar,
        NULL,
        g_hInstance,
        NULL
    );

    










    hSearchTab = CreateWindowEx(
        WS_EX_CONTROLPARENT,
        WC_TABCONTROL,
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 10, 430, 605,
        frame,
        (HMENU)4001,
        NULL,
        NULL
    );




    LoadTabsFromIni(hSearchTab, INI_SEARCH, SearchTabs, &SearchTabCount);
    SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, 0, INI_SEARCH, IDC_SEARCH_BUTTON, FALSE);

    RenderPageToCache(g_pictureFrame);
    InvalidateRect(g_pictureFrame, NULL, TRUE);






    if (SearchWndProc) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }


    return hwnd;
}









HWND OpenPrintWindow(HWND hwndParent) {



    if (g_PrintFrame && IsWindow(g_PrintFrame))
    {
        SetForegroundWindow(g_PrintFrame);
        return g_PrintFrame;
    }


    const wchar_t CLASS_NAME[] = L"Print Menu";


    static bool registered = false;
    if (!registered) {
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = PrintWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        RegisterClass(&wc);
        registered = true;
    }

    int width = 800;
    int height = 600;

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;





    PRINT_SETTINGS* settings = malloc(sizeof(PRINT_SETTINGS));
    ZeroMemory(settings, sizeof(PRINT_SETTINGS));




    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT,
        CLASS_NAME,
        L"Print",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y,
        width, height,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        settings
    );

    if (!hwnd)
        return NULL;

    g_PrintFrame = hwnd;









    if (PrintWndProc) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }


    return hwnd;

}







void LayoutCustomerNav(HWND hwndParent)
{
    wchar_t name[256];
    GetWindowTextW(hSearchCountLabel, name, 256);

    int arrowWidth = 30;
    int arrowHeight = 25;
    int spacing = 10;

    int textWidth = GetTextPixelWidth(hSearchCountLabel, name);



    wchar_t labelText[256];
    GetWindowTextW(hFileNameText, labelText, 256);
    int textWidth2 = GetTextPixelWidth(hFileNameText, labelText);




    int totalWidth = arrowWidth + spacing +
        textWidth + spacing +
        arrowWidth;

    RECT rc;
    GetClientRect(hwndParent, &rc);

    int startX = (rc.right - totalWidth) / 2;

    int startX2 = rc.right - (totalWidth / 2);

    int y = 10;
    int y2 = 5;
    


    MoveWindow(hFileNameText,
        startX - arrowWidth - spacing,
        y + y2,
        textWidth2,
        arrowHeight,
        TRUE);



    MoveWindow(hLeftArrow,
        startX2 - arrowHeight - spacing - (textWidth / 2) - spacing - spacing - spacing,
        y,
        arrowWidth,
        arrowHeight,
        TRUE);

    MoveWindow(hSearchCountLabel,
        startX2 - arrowHeight - spacing - spacing - spacing - spacing,
        y + y2,
        textWidth,
        arrowHeight,
        TRUE);

    MoveWindow(hRightArrow,
        startX2 + arrowHeight + spacing + spacing,
        y,
        arrowWidth,
        arrowHeight,
        TRUE);



    ShowWindow(hLeftArrow, g_SearchActive ? SW_SHOW : SW_HIDE);
    ShowWindow(hRightArrow, g_SearchActive ? SW_SHOW : SW_HIDE);
    ShowWindow(hFileNameText, g_SearchActive ? SW_SHOW : SW_HIDE);
    ShowWindow(hSearchCountLabel, g_SearchActive ? SW_SHOW : SW_HIDE);
    ShowWindow(hPageLabel, g_SearchActive ? SW_SHOW : SW_HIDE);
    ShowWindow(hPrintButton, g_SearchActive ? SW_SHOW : SW_HIDE);
}












int WINAPI WinMain(

    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
) {






    pdf_ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!pdf_ctx)
    {
        MessageBox(NULL, L"Failed to create MuPDF context", L"Error", MB_OK);
        return 1;
    }

    fz_try(pdf_ctx)
    {
        fz_register_document_handlers(pdf_ctx);
    }
    fz_catch(pdf_ctx)
    {
        MessageBoxA(NULL, fz_caught_message(pdf_ctx), "Error registering handlers", MB_OK);
        fz_drop_context(pdf_ctx);
        return 1;
    }



    doc = NULL;
    total_pages = 0;
    current_opened_pdf = L"No PDF Loaded";








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


    INITCOMMONCONTROLSEX initControls = { 0 };
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
        100, 100, 100, 100,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"Failed to create main window", L"Error", MB_OK);
        return 1;
    }



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

        if (g_customerPopup && IsDialogMessage(g_customerPopup, &msg))
            continue;

        if (g_PrintFrame && IsDialogMessage(g_PrintFrame, &msg))
            continue;


        if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB)
        {
            HWND focused = GetFocus();

            if (!focused)
                focused = hwnd;

            HWND root = NULL;

            if (!root)
                root = hwnd;

            if (g_pdfFrame && IsWindow(g_pdfFrame))
                root = g_pdfFrame;
            else if (hPopupWnd && IsWindow(hPopupWnd))
                root = hPopupWnd;

            if (root)
            {
                BOOL backwards = (GetKeyState(VK_SHIFT) & 0x8000);

                HWND next = GetNextDlgTabItem(root, focused, backwards);

                if (next)
                    SetFocus(next);

                continue;
            }
        }





        if (g_pictureFrame && IsWindow(g_pictureFrame))
        {
            HWND focused = GetForegroundWindow();

            if (g_pictureFrame && IsWindow(g_pictureFrame))
            {
                if (msg.message == WM_KEYDOWN)
                {

                    if (msg.wParam == VK_RIGHT)
                    {
                        NextResult();
                        

                        PostMessage(g_pictureFrame, WM_APP_REDRAW_PDF, 0, 0);
                        continue;
                    }

                    if (msg.wParam == VK_LEFT)
                    {
                        PreviousResult();
                        

                        PostMessage(g_pictureFrame, WM_APP_REDRAW_PDF, 0, 0);
                        continue;
                    }
                }
            }
        }








        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }



    if (doc) fz_drop_document(pdf_ctx, doc);
    if (pdf_ctx) fz_drop_context(pdf_ctx);




    return (int)msg.wParam;
}
