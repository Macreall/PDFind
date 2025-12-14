#define SDL_MAIN_HANDLED
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#define STB_IMAGE_IMPLEMENTATION

#include "resources/SDL2/include/SDL.h"
#include "GL/gl.h"
#include "resources/nuklear.h"
#include "resources/nuklear_sdl_gl2.h"
#include <stdio.h>
#include "resources/stb_image.h"


void draw_background(GLuint bg_tex, int window_w, int window_h)
{
    glViewport(0, 0, window_w, window_h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, window_w, window_h, 0, -1, 1); // top-left = (0,0)

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bg_tex);

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(1,0); glVertex2f(window_w,0);
    glTexCoord2f(1,1); glVertex2f(window_w,window_h);
    glTexCoord2f(0,1); glVertex2f(0,window_h);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix(); // MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow("PDFinder",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1400, 800,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GLContext gl_context = SDL_GL_CreateContext(win);

    /* --- INIT NUKLEAR --- */
    struct nk_context* ctx = nk_sdl_init(win);

    /* --- SETUP FONT ATLAS --- */
    struct nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font* font = nk_font_atlas_add_default(atlas, 14, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(ctx, &font->handle);

    struct nk_color background = nk_rgb(28, 48, 62);
    struct nk_color textColor = nk_rgb(255,255,255);


    int running = 1;

    // check box bool
    int active = 0;

    // radio button bool
    int selected = 0;

    // Slider value
    float value = 0.5f;

    // Progress bar value
    float progress = 0.3f;

    // Entry box char
    char buffer[32] = "";

    // Combo Box
    int selectedComboBox = 0;

    // Tabs
    int active_tab = 0; // 0 = Tab1, 1 = Tab2


    // Background Image
    int bg_tex;
    int w, h, n;
    unsigned char* data = stbi_load("../assets/images/gray_background.jpg", &w, &h, &n, 4);
    if (!data) {
        printf("Failed to load image\n");
        return -1;
    }
    glGenTextures(1, &bg_tex);
    glBindTexture(GL_TEXTURE_2D, bg_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);







    while (running) {
        float nkWinSize = 600;

        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            nk_sdl_handle_event(&evt);
            if (evt.type == SDL_QUIT) running = 0;
            if (evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                running = 0;
            }
        }
        nk_input_end(ctx);

        draw_background(bg_tex, 1400, 800);   // ← draws your PNG/JPG


        // if (nk_begin(ctx, "PDFinder", nk_rect(1000, 50, 200, 250),
        //     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_CURSOR_RESIZE_TOP_LEFT_DOWN_RIGHT)) {
        //     printf("here");
        // }




        /* ---- GUI ---- */
        if (nk_begin(ctx, "Left Side", nk_rect(50, 50, nkWinSize, 700),
        NK_WINDOW_MOVABLE | NK_CURSOR_RESIZE_TOP_LEFT_DOWN_RIGHT))
        {
            // // Layout for row and column
            // nk_layout_row_static(ctx, 40, 120, 1);
            //
            // // Labels
            // nk_label(ctx, "Hello World", NK_TEXT_LEFT);
            //
            // // Checkbox
            // nk_checkbox_label(ctx, "Check Me", &active);
            //
            // // Buttons need if statements to do something
            // if (nk_button_label(ctx, "Click Me")) {
            //     printf("Button Pressed!\n");
            // }
            //
            // nk_layout_row_static(ctx, 30, 400, 1);
            //
            // // Radio Buttons
            // nk_option_label(ctx, "Option 1", selected == 0);
            // nk_option_label(ctx, "Option 2", selected == 1);
            //
            // // Sliders
            // nk_slider_float(ctx, 0.0f, &value, 1.0f, 0.01f);

            // Progress bar
            // nk_progress(ctx, (int)(progress*100), 100, NK_MODIFIABLE);




            nk_layout_row_static(ctx, 60, 120, 3);


            ctx->style.text.color = nk_rgb(255,255,255);

            ctx->style.button.text_normal = nk_rgb(0,0,0);
            ctx->style.button.text_hover = nk_rgb(0,0,0);



            ctx->style.tab.padding.x = 0;
            ctx->style.tab.padding.y = 5;
            ctx->style.tab.rounding = 900.f;







            // Tabs
            nk_layout_row_static(ctx, 30, nkWinSize / 4, 3); // row for 2 buttons
            if (nk_button_label(ctx, "Job Tickets")) active_tab = 0;
            if (nk_button_label(ctx, "Invoices")) active_tab = 1;
            if (nk_button_label(ctx, "Parts Receipts")) active_tab = 2;


            // --- Tab content ---
            nk_layout_row_dynamic(ctx, 300, 1); // dynamic area for content
            if (active_tab == 0) {
                if (nk_group_begin(ctx, "Tab1", NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_label(ctx, "This is Tab 1 content", NK_TEXT_LEFT);
                    nk_checkbox_label(ctx, "Check me", &active);

                    // Entry
                    nk_layout_row_static(ctx, 30, 220, 1); // row for 2 buttons
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, buffer, sizeof(buffer)-1, nk_filter_default);

                    nk_group_end(ctx);
                }
            } else if (active_tab == 1) {
                if (nk_group_begin(ctx, "Tab2", NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_label(ctx, "This is Tab 2 content", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 0.0f, &value, 1.0f, 0.01f);

                    // Entry
                    nk_layout_row_static(ctx, 30, 220, 1); // row for 2 buttons
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, buffer, sizeof(buffer)-1, nk_filter_default);

                    nk_group_end(ctx);
                }
            } else if (active_tab == 2) {
                if (nk_group_begin(ctx, "Tab3", NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_label(ctx, "This is Tab 3 content", NK_TEXT_LEFT);
                    nk_label(ctx, "This is how we can create 3 tabs", NK_TEXT_LEFT);
                    nk_slider_float(ctx, 0.0f, &value, 1.0f, 0.01f);

                    // Entry
                    nk_layout_row_static(ctx, 30, 220, 1); // row for 2 buttons
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, buffer, sizeof(buffer)-1, nk_filter_default);

                    nk_group_end(ctx);
                }
            }



            // Combo Boxes
            // const char* items[] = {"One", "Two", "Three"};
            // nk_combo_begin_label(ctx, items[selectedComboBox], nk_vec2(200,200));
            // for (int i=0;i<3;i++) {
            //     if (nk_menu_item_label(ctx, items[i], NK_TEXT_LEFT)) selectedComboBox=i;
            // }
            // nk_combo_end(ctx);

        }
        nk_end(ctx);




        if (nk_begin(ctx, "Right side",
            nk_rect(800, 50, 400, 700),
            NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
        {
            nk_button_label(ctx, "Right side");
        }
        nk_end(ctx);





        /* DRAW */
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(win);
    }

    nk_sdl_shutdown();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
