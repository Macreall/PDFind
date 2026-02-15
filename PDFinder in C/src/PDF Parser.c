#include "PDF Parser.h"

int scrollY = 0;
int maxScroll = 0;
int page_height = 750;


void renderPDF(HDC hdc, HWND hwnd, fz_context* pdf_ctx, fz_document* doc, int current_pdf_page, int total_pages) {

        RECT rect;
        GetClientRect(hwnd, &rect);


        int y_offset = 0;

        for (int i = 0; i < total_pages; i++) {
            fz_page *page = fz_load_page(pdf_ctx, doc, i);
            fz_rect page_box = fz_bound_page(pdf_ctx, page);

            float scale = (float)(rect.right - rect.left) / (page_box.x1 - page_box.x0);

            fz_matrix ctm = fz_scale(scale, scale);
            fz_rect pixel_rect = fz_transform_rect(page_box, ctm);
            fz_irect bbox = fz_round_rect(pixel_rect);

            fz_pixmap *pix = fz_new_pixmap_with_bbox(
                pdf_ctx,
                fz_device_rgb(pdf_ctx),
                bbox,
                NULL,
                1
            );

            fz_clear_pixmap_with_value(pdf_ctx, pix, 255);

            fz_device *dev = fz_new_draw_device(pdf_ctx, ctm, pix);
            fz_run_page(pdf_ctx, page, dev, ctm, NULL);
            fz_drop_device(pdf_ctx, dev);

            int w = fz_pixmap_width(pdf_ctx, pix);
            int h = fz_pixmap_height(pdf_ctx, pix);
            unsigned char *samples = fz_pixmap_samples(pdf_ctx, pix);

            if (i == 0) page_height = h + 20;

            int draw_y = y_offset - scrollY;

            if (draw_y + h > 0 && draw_y < rect.bottom)
            {
                BITMAPINFO bmi = {0};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = w;
                bmi.bmiHeader.biHeight = -h;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                StretchDIBits(
                    hdc,
                    0, draw_y,
                    w, h,
                    0, 0, w, h,
                    samples,
                    &bmi,
                    DIB_RGB_COLORS,
                    SRCCOPY
                );
            }

            y_offset += h + 20;

            fz_drop_pixmap(pdf_ctx, pix);
            fz_drop_page(pdf_ctx, page);
        }

        maxScroll = y_offset - rect.bottom;
        if (maxScroll < 0) maxScroll = 0;




}
