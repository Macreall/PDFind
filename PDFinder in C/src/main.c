#define UNICODE



#include "../resources.h"
#include "PDF Parser.h"




struct fz_context* pdf_ctx = NULL;
struct fz_document* doc = NULL;
int current_pdf_page = 0;
HWND g_pdfFrame = NULL;
HWND g_pictureFrame = NULL;

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

BOOL g_SearchActive = FALSE;

void LayoutCustomerNav(HWND hwndParent);










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
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SAVE_SETTINGS  1002
#define ID_TRAY_SEARCH_SETTINGS  1003
#define ID_TRAY_SEARCH  1004
#define ID_TRAY_UNDO 1005
#define IDC_COMBOBOX_DATES 101

#define IDC_SAVE_BUTTON 105
#define IDC_SEARCH_BUTTON 106


#define IDC_LEFT_ARROW 107
#define IDC_RIGHT_ARROW 108
#define IDC_NAME_TEXT 109
#define IDC_PRINT_BUTTON 110


HWND hPopupTab = NULL;
HWND hPopupWnd = NULL;
HWND hButton = NULL;

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
    HWND hwnd;

} FIELD_DATA;

typedef struct {
    wchar_t name[64];
    wchar_t type[16];


    wchar_t iniSection[64];
    FIELD_DATA fields[16];
    u_int fieldCount;
    HWND hPage; 

    HWND hButton;
    LPCWSTR iniPath;
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

LPCWSTR INI_SAVE = L"C:\\watchFolder\\save_settings.ini";
LPCWSTR INI_SEARCH = L"C:\\watchFolder\\search_settings.ini";



#define MAX_TABS 32
#define MAX_FIELDS 32
HWND fieldLabels[MAX_FIELDS];
HWND fieldControls[MAX_FIELDS];
int activeFieldCount = 0;

TAB_DATA SearchTabs[MAX_TABS];
TAB_DATA SaveTabs[MAX_TABS];


int running = 1;

int g_CurrentPage = 0;
WCHAR g_CurrentFilename[MAX_PATH];
u_int PAGE_COUNT = 0;

u_int SearchTabCount;
u_int SaveTabCount;


void CreateFieldsFromTab(HWND parent, TAB_DATA* tab, LPCWSTR iniPath);
HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text);
HWND OpenSearchWindow(HWND hwndParent);
LRESULT CALLBACK PictureFrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ExpandTemplate(const wchar_t* input, wchar_t* output, size_t outSize, FIELD_VALUE* fields, int fieldCount);
void LoadNewPDF(const wchar_t* newPath);










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


void PrintCurrentPDF()
{
    MessageBox(NULL, L"Printing...", L"Print", MB_OK);


    if (!current_opened_pdf_full) {
        MessageBox(NULL, L"No PDF loaded", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.lpVerb = L"print";
    sei.lpFile = current_opened_pdf_full; 
    sei.nShow = SW_HIDE;

    if (!ShellExecuteEx(&sei)) {
        DWORD err = GetLastError();
        wchar_t buf[256];
        swprintf_s(buf, 256, L"Failed to print PDF!\nError %lu", err);
        MessageBox(NULL, buf, L"Error", MB_OK | MB_ICONERROR);
    }
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
    current_opened_pdf_full = _wcsdup(fullPath);
    current_opened_pdf = PathFindFileNameW(newPath);



    RenderPageToCache(g_pictureFrame);

    PostMessage(g_pictureFrame, WM_APP_REDRAW_PDF, 0, 0);

    if (g_toolbar)
        InvalidateRect(g_toolbar, NULL, TRUE);

    WCHAR pageText[64];
    swprintf_s(pageText, 64, L"Page   %d / %d", current_pdf_page + 1, total_pages);
    SetWindowTextW(hPageLabel, pageText);
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

void AddFound(FOUND_LIST* list, const wchar_t* path)
{
    if (list->count >= MAX_RESULTS)
        return;

    wcscpy_s(list->items[list->count].fullPath, MAX_PATH, path);
    list->count++;
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



void SearchFromIniPaths(int currentPage, FOUND_LIST* results, TAB_DATA* tabs)
{
    TAB_DATA* tab = &tabs[currentPage];

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

        SearchFolder(folder, tab, results);
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
    }
    activeFieldCount = 0;
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

bool LoadButtonData(const wchar_t* tabSection, BUTTON_DATA* b, LPCWSTR iniPath)
{
    if (!b) return false;

    b->x = GetPrivateProfileIntW(tabSection, L"Button.X", -1, iniPath);
    b->y = GetPrivateProfileIntW(tabSection, L"Button.Y", -1, iniPath);

    if (b->x < 0 || b->y < 0)
        return false;

    b->width = GetPrivateProfileIntW(tabSection, L"Button.Width", 80, iniPath);
    b->height = GetPrivateProfileIntW(tabSection, L"Button.Height", 25, iniPath);

    GetPrivateProfileStringW(
        tabSection,
        L"Button.Text",
        L"Save",
        b->text,
        64,
        iniPath
    );

    wcscpy_s(b->name, 64, L"SaveButton");
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

    tab->fieldCount =
        GetPrivateProfileIntW(section, L"FieldCount", 0, iniPath);



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





void CreateFieldsFromTab(HWND parent, TAB_DATA* tab, LPCWSTR iniPath)
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


        activeFieldCount++;
    }
}


void CreateButtons(HWND parent, int pageIndex, LPCWSTR iniPath, UINT btnID, TAB_DATA* tabs) {
    TAB_DATA* tab = &tabs[pageIndex];

    if (tab->hButton && IsWindow(tab->hButton)) {
        DestroyWindow(tab->hButton);
        tab->hButton = NULL;
    }

    BUTTON_DATA btn;
    if (!LoadButtonData(tab->iniSection, &btn, iniPath))
        return;

    RECT rc;
    GetClientRect(parent, &rc);
    TabCtrl_AdjustRect(parent, FALSE, &rc);

    int drawX = rc.left + btn.x;
    int drawY = rc.top + btn.y;

    tab->hButton = CreateWindowW(
        L"BUTTON",
        btn.text,
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        drawX, drawY,
        btn.width, btn.height,
        parent,
        (HMENU)(INT_PTR)btnID,
        g_hInstance,
        NULL
    );

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






void SaveFile(int currentPage) {
    if (currentPage < 0 || currentPage >= SaveTabCount) return;

    TAB_DATA* tab = &SaveTabs[currentPage];

    for (int i = 0; i < tab->fieldCount; i++) {
        FIELD_DATA* f = &tab->fields[i];
        if (f->controlType == FIELD_COMBO) {
            int sel = (int)SendMessage(f->hControl, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR) {
                SendMessage(f->hControl, CB_GETLBTEXT, sel, (LPARAM)f->lastComboValue);

                int fieldIndex = (int)(f - &SaveTabs[g_CurrentPage].fields[0]);
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
    GetPrivateProfileStringW(tab->iniSection, L"SavedFileName", L"", filePattern, 256, INI_SAVE);
    if (filePattern[0] == 0) {
        MessageBox(NULL, L"No SavedFileName defined!", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return;
    }
    ExpandTemplate(filePattern, finalFile, 256, fieldValues, fieldCount);
    SanitizeFilename(finalFile);

    for (int pnum = 1; pnum <= 10; pnum++) {
        wchar_t pathKey[32], folderTemplate[512], expandedFolder[512];
        swprintf_s(pathKey, 32, L"Path%d", pnum);
        GetPrivateProfileStringW(tab->iniSection, pathKey, L"", folderTemplate, 512, INI_SAVE);
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
            SaveLastPath(oldFile, fullPath, INI_SAVE);
            MessageBox(NULL, L"File Saved Successfully!", L"Success", MB_OK | MB_TOPMOST);
        }
    }

    DeleteFileW(oldFile);
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




void SetPage(
    HWND hTab,
    HWND hwndParent,
    TAB_DATA* tabs,
    int pageCount,
    int newPage,
    LPCWSTR iniPath,
    int buttonId
)
{
    if (newPage < 0 || newPage >= pageCount)
        return;

    TabCtrl_SetCurSel(hTab, newPage);

    DestroyActiveFields();

    CreateFieldsFromTab(hwndParent, &tabs[newPage], iniPath);
    CreateButtons(hwndParent, newPage, iniPath, buttonId, tabs);

    TAB_DATA* tab = &tabs[newPage];

    for (int i = 0; i < tab->fieldCount; i++)
        PopulateControlData(&tab->fields[i], iniPath);
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

            if (undoresult -= IDNO)
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

            SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, newPage, INI_SEARCH, IDC_SEARCH_BUTTON);
        }
        break;

    }


    case WM_KEYDOWN:
    {
        if (wParam == VK_TAB)
        {
            HWND current = GetFocus();

            HWND root = GetAncestor(current, GA_ROOT);
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

        switch (id)
        {
        case IDC_PRINT_BUTTON:
            PrintCurrentPDF();
            break;

        case IDC_SEARCH_BUTTON:
        {
            if (id == IDC_SEARCH_BUTTON)
            {

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

                return 0;
            }
            break;
        }

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
        hPopupWnd = NULL;
        break;


    case WM_KEYDOWN:
    {
        if (wParam == VK_TAB)
        {
            HWND current = GetFocus();

            HWND root = GetAncestor(current, GA_ROOT);

            BOOL backwards = (GetKeyState(VK_SHIFT) & 0x8000);

            HWND next = GetNextDlgTabItem(root, current, backwards);

            if (next)
                SetFocus(next);

            return 0;
        }

    }


    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->hwndFrom == hPopupTab && pnmh->code == TCN_SELCHANGE)
        {
            int newPage = TabCtrl_GetCurSel(hPopupTab);
            g_CurrentPage = newPage;

            SetPage(hPopupTab, hPopupWnd, SaveTabs, SaveTabCount, newPage, INI_SAVE, IDC_SAVE_BUTTON);
        }
    }
    break;
    case WM_COMMAND:
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

        case IDC_SAVE_BUTTON:


           


            SaveFile(currentPage);


            DestroyWindow(hwnd);


            break;
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


HWND OpenPopupWindow(HWND hwndParent, LPCWSTR text) {

    if (hPopupWnd && IsWindow(hPopupWnd))
        return hPopupWnd;


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
    SetPage(hPopupTab, hPopupWnd, SaveTabs, SaveTabCount, 0, INI_SAVE, IDC_SAVE_BUTTON);





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

            SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, newPage, INI_SEARCH, IDC_SEARCH_BUTTON);
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
        //RECT centerRect = rect;
        //DrawText(hdc, current_opened_pdf, -1, &centerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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
        }
        
        return 0;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
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
        NULL,
        WS_CHILD | WS_VISIBLE | BS_ICON | BS_OWNERDRAW,
        120, 8, 30, 30,
        hToolbar,
        (HMENU)IDC_PRINT_BUTTON,
        g_hInstance,
        NULL
    );

    
   



    HICON hPrintIcon = (HICON)LoadImageW(
        NULL,
        MAKEINTRESOURCEW(OIC_PRINT),
        IMAGE_ICON,
        24,
        24,
        LR_SHARED | LR_DEFAULTSIZE
    );

    SendMessageW(hPrintButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPrintIcon);







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
    SetPage(hSearchTab, frame, SearchTabs, SearchTabCount, 0, INI_SEARCH, IDC_SEARCH_BUTTON);

    RenderPageToCache(g_pictureFrame);
    InvalidateRect(g_pictureFrame, NULL, TRUE);






    if (SearchWndProc) {
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

        if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB)
        {
            HWND focused = GetFocus();

            HWND root = NULL;

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
