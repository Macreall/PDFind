import customtkinter as ctk
import fitz
from PIL import Image, ImageTk
from customtkinter import CTkImage
import pdfplumber
import os
from difflib import SequenceMatcher
import subprocess


import tabView as tb


class App:
    def __init__(self):
        # Setup
        self.running = True
        self.root = ctk.CTk()
        self.root.geometry("1200x800")
        self.root.title("PDFind")

        # Key Bindings
        self.root.bind("<Escape>", lambda e: self.root.destroy())
        self.root.bind("<Right>", self.next_page)
        self.root.bind("<Left>", self.prev_page)


        bg_image = ctk.CTkImage(
            light_image=Image.open("gray_background.jpg"),
            size=(2000, 1800)
        )

        bg_label = ctk.CTkLabel(self.root, image=bg_image, text="")
        bg_label.place(x=0, y=0, relwidth=1, relheight=1)




        # Tab Frame
        self.ctk_tab_frame = ctk.CTkFrame(self.root, width=800, height=800, fg_color="#C5C6C7", bg_color="#7B7B7C",
                                          border_width=2, border_color="#7B7B7C")
        self.ctk_tab_frame.grid(row=0, column=0, padx=20, pady=20)

        # Tabs for Viewing Options
        self.tab_view = tb.MyTabView(self.ctk_tab_frame, main_app=self, width=600, height=800, fg_color="#bfcbd1")
        self.tab_view.grid(row=0, column=0, padx=10, pady=20)

        # Page Viewer Frame
        self.ctk_frame = ctk.CTkFrame(self.root, width=800, height=800, fg_color="#bfcbd1", bg_color="#7B7B7C",
                                      border_width=2, border_color="#7B7B7C")
        self.ctk_frame.grid(row=0, column=2, padx=80, pady=20)
        self.ctk_frame.grid_remove()

        # Document File Opening
        self.ctk_label = ctk.CTkLabel(self.ctk_frame, text="")
        self.ctk_label.pack(fill="both", expand=True, padx=90, pady=20)

        # Print Button
        self.print_button = ctk.CTkButton(self.ctk_frame, text="Print", command=self.print_file,
                                          text_color='black', font=("Georgia", 14), hover_color="#80c904",
                                          border_color='black', border_width=4, fg_color="#FCE205", corner_radius=0)
        self.print_button.place(relx=0.42, rely=0.001)

        # Page Buttons
        self.next_page_button = ctk.CTkButton(self.ctk_frame, text='-->', command=self.next_page, width=25)
        self.next_page_button.place(relx=0.93, rely=0.5)

        self.previous_page_button = ctk.CTkButton(self.ctk_frame, text='<--', command=self.prev_page, width=25)
        self.previous_page_button.place(relx=0.04, rely=0.5)

        self.matched_pages = []
        self.current_index = 0

        self.open_label = ctk.CTkLabel(self.ctk_frame,
                                       text="Open \n File Path\n ---------",
                                       text_color="black",
                                       cursor="hand2",
                                       font=("Georgia", 14),
                                       fg_color="transparent")
        self.open_label.bind("<Button-1>", lambda e: self.open_file_location())
        self.open_label.place(relx=0.012, rely=0.03)

        self.add_hover_effect(self.open_label)

        self.open_label2 = ctk.CTkLabel(self.ctk_frame,
                                       text="Open \n Paperwork\n ---------",
                                       text_color="black",
                                       cursor="hand2",
                                       font=("Georgia", 14))
        self.open_label2.bind("<Button-1>", lambda e: self.open_file())
        self.open_label2.place(relx=0.012, rely=0.13)
        self.add_hover_effect(self.open_label2)

        # Error Label
        self.error_label = ctk.CTkLabel(self.ctk_tab_frame, text='', font=("Georgia", 24), fg_color="#bfcbd1", text_color='red')
        self.error_label.place(relx=0.1, rely=0.6)

        self.current_file_path = ""




    def add_hover_effect(self, label):
        def on_enter(e):
            label.configure(
                fg_color="#12cfd1",
                corner_radius=4,
                padx=2,
                pady=1
            )

        def on_leave(e):
            label.configure(
                fg_color="transparent",
                padx=0,
                pady=0
            )

        label.bind("<Enter>", on_enter)
        label.bind("<Leave>", on_leave)

    def open_file(self):
        if not self.current_file_path:
            return

        os.startfile(self.current_file_path)

    def open_file_location(self):
        if not self.current_file_path:
            return

        subprocess.Popen(rf'explorer /select,"{self.current_file_path}"')

    def show_page(self, index):
        page = self.matched_pages[index]
        self.current_index = index
        self.ctk_label.configure(image=page["ctk"])
        self.ctk_label.image = page["ctk"]

        self.ctk_frame.grid()

    def next_page(self, event=None):
        if self.matched_pages:
            if self.current_index < len(self.matched_pages) - 1:
                self.current_index += 1
                self.show_page(self.current_index)
        else:
            return

    def prev_page(self, event=None):
        if self.matched_pages:
            if self.current_index > 0:
                self.current_index -= 1
                self.show_page(self.current_index)

        else:
            return

    def print_file(self):
        page = self.matched_pages[self.current_index]
        pil_image = page["pil"]

        temp_path = "temp_print_image.png"
        pil_image.save(temp_path)

        os.startfile(temp_path, "print")


    def search_job_tickets(self, folder, customer_info):
        self.matched_pages = []

        for filename in os.listdir(folder):
            if len(self.matched_pages) >= 20:
                break
            filepath = os.path.join(folder, filename)
            if not os.path.isfile(filepath):
                continue


            try:
                if customer_info.lower() in filename.lower():
                    self.current_file_path = filepath
                    doc = fitz.open(filepath)
                    mp_page = doc[0]
                    pix = mp_page.get_pixmap(matrix=fitz.Matrix(1.0, 1.0))
                    img = Image.frombytes("RGB", (pix.width, pix.height), pix.samples)
                    ctk_img = CTkImage(light_image=img, size=(pix.width, pix.height))

                    self.matched_pages.append({
                        "ctk": ctk_img,
                        "pil": img
                    })
                    if self.matched_pages:
                        self.show_page(0)



            except Exception as e:
                print(f"Error opening {filepath}: {e}")


        if self.matched_pages:
            self.error_label.configure(text="")
        else:
            self.error_label.configure(text=f'Error, unable to find file:\n - "{customer_info}"')


    def search_invoices(self, folder, customer_info, selected_year):
        self.matched_pages = []

        months = [
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        ]
        deposit_numbers = [5,4,3,2,1]

        text_file = os.path.join(folder, f"{selected_year} Paid Customers.txt")

        deposit_numbers = [1,2,3,4,5]


        try:

            with open(text_file, "r") as f:
                text_lines = f.readlines()
        except Exception as e:
            print("Could not open text file:", e)
            self.error_label.configure(text=f'Error, could not find "{selected_year} Paid Customers.txt"')
            return

        customer_info_clean = customer_info.lower().strip()
        current_month = None
        matched_month = None
        matched_date = None
        current_date = None


        for line in text_lines:
            if len(self.matched_pages) >= 20:
                break
            line_strip = line.strip()

            for m in months:
                if line_strip.endswith(f"{m}:"):
                    current_month = m
                    break

            if line_strip.endswith(":") and "/" in line_strip:
                current_date = line_strip.replace(":", "")
                continue
            #
            # if line_strip.startswith("Deposit"):
            #     matched_month = current_month
            #     matched_date = line_strip.replace(":", "")
            #     break
            #
            # if line_strip.startswith("Deposit Summary 2"):
            #     matched_month = current_month
            #     matched_date = line_strip.replace(":", "")
            #
            # if line_strip.startswith("Deposit Summary 3"):
            #     matched_month = current_month
            #     matched_date = line_strip.replace(":", "")
            #
            # if line_strip.startswith("Deposit Summary 4"):
            #     matched_month = current_month
            #     matched_date = line_strip.replace(":", "")



            if customer_info_clean in line_strip.lower():
                matched_month = current_month
                matched_date = current_date
                break

        if not matched_date:
            self.error_label.configure(text="Customer not found.")
            return


        cleaned_date = matched_date.replace("/", "-")
        print(matched_date)

        self.current_file_path = os.path.join(folder, matched_month, cleaned_date)
        self.current_file_path = f"{self.current_file_path}.pdf"
        print(self.current_file_path)



        try:
            with pdfplumber.open(self.current_file_path) as pdf:
                for page_number in range(len(pdf.pages)):
                    if len(self.matched_pages) >= 20:
                        break

                    page = pdf.pages[page_number]
                    width, height = page.width, page.height
                    top = height * 0.5
                    bottom = height * 0.75

                    region = page.crop((0, top, width, bottom))
                    text = region.extract_text() or ""
                    text_clean = text.replace(" ", "")

                    doc = fitz.open(self.current_file_path)
                    mp_page = doc[page_number]
                    pix = mp_page.get_pixmap(matrix=fitz.Matrix(1.0, 1.0))
                    img = Image.frombytes("RGB", (pix.width, pix.height), pix.samples)
                    ctk_img = CTkImage(light_image=img, size=(pix.width, pix.height))

                    self.matched_pages.append({
                        "ctk": ctk_img,
                        "pil": img
                    })
                    if self.matched_pages:
                        self.show_page(0)



        except Exception as e:
            print(f"Error opening {self.current_file_path}:", e)
            self.error_label.configure(text=f'Error, could not find file \n - {cleaned_date}.pdf at \n - {selected_year}\\Invoices\\{current_month}')
            return


        if self.matched_pages:
            self.error_label.configure(text="")
            self.show_page(0)
        else:
            self.error_label.configure(text=f'Error, unable to find file:\n - "{customer_info}"')




    def search_parts(self, folder, part_number, selected_year, selected_month):
        self.matched_pages = []

        part_number_clean = part_number.replace(" ", "")
        for root, dirs, files in os.walk(folder):
            if selected_month and not any(month in root for month in selected_month):
                continue

            for filename in files:
                if not filename.lower().endswith(".pdf"):
                    continue

                filepath = os.path.join(root, filename)

                try:
                    with pdfplumber.open(filepath) as pdf:
                        for page_number in range(len(pdf.pages)):
                            if len(self.matched_pages) >= 20:
                                break


                            page = pdf.pages[page_number]
                            width, height = page.width, page.height
                            right = width * 0.5
                            top = height * 0.5
                            bottom = height * 0.75

                            region = page.crop((0, top, right, bottom))
                            text = region.extract_text() or ""
                            text_clean = text.replace(" ", "")


                            best_ratio = 0
                            for i in range(len(text_clean) - len(part_number_clean) + 1):
                                snippet = text_clean[i:i + len(part_number_clean)]
                                ratio = SequenceMatcher(None, part_number_clean, snippet).ratio()
                                best_ratio = max(best_ratio, ratio)

                            if best_ratio >= 0.75:
                                self.current_file_path = filepath
                                doc = fitz.open(filepath)
                                mp_page = doc[page_number]
                                pix = mp_page.get_pixmap(matrix=fitz.Matrix(1.0, 1.0))
                                img = Image.frombytes("RGB", (pix.width, pix.height), pix.samples)
                                ctk_img = CTkImage(light_image=img, size=(pix.width, pix.height))

                                self.matched_pages.append({
                                    "ctk": ctk_img,
                                    "pil": img
                                })
                                if self.matched_pages:
                                    self.show_page(0)

                except Exception as e:
                    print(f"Error opening {filepath}: {e}")
                    self.error_label.configure(text=f'Error opening {filepath}"')

        if self.matched_pages:
            self.error_label.configure(text="")
        else:
            self.error_label.configure(text=f'Error, unable to find\n - "{part_number}"')



        return False



    def update(self):
        self.root.after(16, self.update)

        if not self.running:
            self.root.destroy()
            return

    def run(self):
        try:
            self.update()
            self.root.mainloop()
        except KeyboardInterrupt:
            print("Closing Safely...")


if __name__ == '__main__':
    app = App()
    app.run()
