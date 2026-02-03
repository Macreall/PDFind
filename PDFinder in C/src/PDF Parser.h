#pragma once


#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include "../mupdf/include/mupdf/fitz.h"

void renderPDF(HDC hdc, HWND hwnd, fz_context* pdf_ctx, fz_document* doc);
