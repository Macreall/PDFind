#include "PDF Parser.h"



void renderPDF(HDC hdc, HWND hwnd, fz_context* pdf_ctx, fz_document* doc) {

    RECT rect;
        GetClientRect(hwnd, &rect);

        if (pdf_ctx && doc)

        {
            fz_page *page = fz_load_page(pdf_ctx, doc, 0);
            
            fz_rect page_box = fz_bound_page(pdf_ctx, page);
            
            float xscale = (float)(rect.right - rect.left) / (page_box.x1 - page_box.x0);
            float yscale = (float)(rect.bottom - rect.top) / (page_box.y1 - page_box.y0);
            float scale = xscale < yscale ? xscale : yscale;
            
            float dpi_scale = 1.1f;
            fz_matrix ctm = fz_scale(scale * dpi_scale, scale * dpi_scale);

            fz_rect pixel_rect = fz_transform_rect(page_box, ctm);
            fz_irect bbox = fz_round_rect(pixel_rect);

            fz_pixmap *pix = fz_new_pixmap_with_bbox(pdf_ctx,
                                                      fz_device_rgb(pdf_ctx),
                                                      bbox,
                                                      NULL, 
                                                      1);
            fz_clear_pixmap_with_value(pdf_ctx, pix, 0);
            
            fz_device *dev = fz_new_draw_device(pdf_ctx, ctm, pix);
            fz_run_page(pdf_ctx, page, dev, ctm, NULL);

            fz_drop_device(pdf_ctx, dev);
            
            int w = fz_pixmap_width(pdf_ctx, pix);
            int h = fz_pixmap_height(pdf_ctx, pix);
            unsigned char *samples = fz_pixmap_samples(pdf_ctx, pix);

            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = w;
            bmi.bmiHeader.biHeight = -h;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            SetStretchBltMode(hdc, HALFTONE);
            SetBrushOrgEx(hdc, 0, 0, NULL);

            StretchDIBits(hdc,
                0, 0, rect.right - rect.left, rect.bottom - rect.top,
                0, 0, w, h,
                samples, &bmi, DIB_RGB_COLORS, SRCCOPY);

            fz_drop_pixmap(pdf_ctx, pix);
            fz_drop_page(pdf_ctx, page);
        }

}
