#pragma once


#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "../mupdf/include/mupdf/fitz.h"


extern int scrollY;
extern int maxScroll;
extern int totalPages;
extern int page_height;



void renderPDF(HDC hdc, HWND hwnd, fz_context* pdf_ctx, fz_document* doc, int current_pdf_page, int total_pages);
