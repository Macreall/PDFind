import customtkinter as ctk
import os
import checkboxDropdown as cbd
import threading
import getpass


class MyTabView(ctk.CTkTabview):
    def __init__(self, master, main_app, **kwargs):
        super().__init__(master, **kwargs)
        self.app = main_app

        self.add("Job Tickets")
        self.add("Invoices")
        self.add("Parts Receipts")

        self.entries = []



        self.options = ["2024", "2025", "2026", "2027", "2028", "2029", "2030"]
        self.month_options = ["January", "February", "March", "April", "May", "June",
                              "July", "August", "September", "October", "November", "December"]

        self.year_var = ctk.StringVar(value=self.options[0])
        self.chosen_months = []

        # Job Tickets
        self.customer_name_text = ctk.CTkLabel(master=self.tab("Job Tickets"), text='Search Job Tickets', font=("Georgia", 45))
        self.customer_name_text.grid(row=0, column=0, padx=20, pady=20)

        self.job_ticket_entry = ctk.CTkEntry(master=self.tab("Job Tickets"), width=300, placeholder_text="Type here...")
        self.job_ticket_entry.grid(row=1, column=0, padx=20, pady=20)
        self.entries.append(self.job_ticket_entry)

        self.combo_explanation = ctk.CTkLabel(master=self.tab("Job Tickets"), text='Choose Year', font=("Georgia", 14))
        self.combo_explanation.grid(row=3, column=0, padx=0, pady=0)
        self.combobox = ctk.CTkOptionMenu(master=self.tab("Job Tickets"), values=self.options, variable=self.year_var,)
        self.combobox.grid(row=2, column=0, padx=0, pady=0)

        self.job_ticket_explanation = ctk.CTkLabel(master=self.tab("Job Tickets"), text="Search by Customer's Name or PO Number",
                                                   font=("Georgia", 22))
        self.job_ticket_explanation.grid(row=4, column=0, padx=20, pady=20)


        # Invoices
        self.customer_name_text = ctk.CTkLabel(master=self.tab("Invoices"), text='Search Invoices', font=("Georgia", 45))
        self.customer_name_text.grid(row=0, column=0, padx=20, pady=20)

        self.invoice_entry = ctk.CTkEntry(master=self.tab("Invoices"), width=300,
                                                placeholder_text="Type here...")
        self.invoice_entry.grid(row=1, column=0, padx=20, pady=20)
        self.entries.append(self.invoice_entry)

        self.invoices_explanation = ctk.CTkLabel(master=self.tab("Invoices"),
                                                   text="Search by Customer's Name or PO Number",
                                                   font=("Georgia", 22))
        self.invoices_explanation.grid(row=4, column=0, padx=20, pady=20)

        self.combo_explanation2 = ctk.CTkLabel(master=self.tab("Invoices"), text='Choose Year', font=("Georgia", 14))
        self.combo_explanation2.grid(row=3, column=0, padx=0, pady=0)
        self.combobox2 = ctk.CTkOptionMenu(master=self.tab("Invoices"), values=self.options, variable=self.year_var)
        self.combobox2.grid(row=2, column=0, padx=0, pady=0)



        # Parts Receipts
        self.part_number_text = ctk.CTkLabel(master=self.tab("Parts Receipts"), text='Search Part Receipts', font=("Georgia", 45))
        self.part_number_text.grid(row=0, column=0, padx=20, pady=20)

        self.parts_receipts_entry = ctk.CTkEntry(master=self.tab("Parts Receipts"), width=300,
                                              placeholder_text="Type here...")
        self.parts_receipts_entry.grid(row=1, column=0, padx=20, pady=20)
        self.entries.append(self.parts_receipts_entry)

        self.part_receipts_explanation = ctk.CTkLabel(master=self.tab("Parts Receipts"),
                                                   text="Search by Part Number # or Invoice Number #",
                                                   font=("Georgia", 22))
        self.part_receipts_explanation.grid(row=4, column=0, padx=20, pady=20)

        self.combo_explanation3 = ctk.CTkLabel(master=self.tab("Parts Receipts"), text=' Choose Year', font=("Georgia", 14))
        self.combo_explanation3.grid(row=3, column=0, padx=45, pady=0, sticky="w")
        self.combobox3 = ctk.CTkOptionMenu(master=self.tab("Parts Receipts"), values=self.options, variable=self.year_var)
        self.combobox3.grid(row=2, column=0, padx=40, pady=0, sticky="w")

        # self.combo_explanation4 = ctk.CTkLabel(master=self.tab("Parts Receipts"), text='Choose Month', font=("Georgia", 14))
        # self.combo_explanation4.grid(row=3, column=0, padx=40, pady=0, sticky="e")
        # self.combobox4 = ctk.CTkOptionMenu(master=self.tab("Parts Receipts"), values=self.month_options, variable=self.month_var)
        # self.combobox4.grid(row=2, column=0, padx=40, pady=0, sticky="e")


        self.dropdown = cbd.CheckBoxDropdown(
            master=self.tab("Parts Receipts"),
            options=self.month_options,
            command=self.get_values,
            corner_radius=8,
            fg_color="#C5C6C7",
        )

        self.dropdown.grid(row=2, column=0, padx=60, pady=0, sticky="e")

        self.dropdown_explanation = ctk.CTkLabel(master=self.tab("Parts Receipts"), text=' Choose Month', font=("Georgia", 14))
        self.dropdown_explanation.grid(row=3, column=0, padx=60, pady=0, sticky="e")



        for i, entry in enumerate(self.entries):
            entry.bind("<Return>", lambda e, ent=entry: self.search_method(ent))



        self.after(100, self.enlarge_tabs)

    def get_values(self, selected):
        self.chosen_months = selected
        print(self.chosen_months)

    def search_method(self, entry):
        text = entry.get()
        selected_year = self.year_var.get()
        selected_month = self.chosen_months
        username = getpass.getuser()


        # TODO - Will need to update this to a location you can choose. Will also need to fix all of the file locations as they were previously set up.
        folder = f"C:\\Users\\{username}"

        job_path = os.path.join(folder, selected_year, f"Job Tickets {selected_year}")
        invoice_path = os.path.join(folder, selected_year, f"Invoices {selected_year}")
        parts_path = os.path.join(folder, selected_year, f"Parts Receipts {selected_year}")


        try:
            if entry == self.job_ticket_entry:
                if len(entry.get()) < 20:
                    threading.Thread(target=self.app.search_job_tickets, args=(job_path, text), daemon=True).start()
                else:
                    return

            elif entry == self.invoice_entry:
                if len(entry.get()) < 20:
                    threading.Thread(target=self.app.search_invoices, args=(invoice_path, text, selected_year), daemon=True).start()
                else:
                    return

            elif entry == self.parts_receipts_entry:
                if len(entry.get()) < 20:
                    threading.Thread(target=self.app.search_parts, args=(parts_path, text, selected_year, selected_month), daemon=True).start()
                else:
                    return

        except Exception as e:
            print(e)

        entry.delete(0, ctk.END)



    def enlarge_tabs(self):
        segmented = self._segmented_button

        segmented.configure(height=60, selected_hover_color="#FCE205", unselected_hover_color="#FCE205",
                            text_color="black", selected_color='#FCE205')

        for name, btn in segmented._buttons_dict.items():
            btn.configure(font=("Georgia", 20))

        segmented.configure(corner_radius=28)
