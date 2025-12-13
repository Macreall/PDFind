import customtkinter as ctk



class CheckBoxDropdown(ctk.CTkFrame):
    def __init__(self, master, options, command=None, **kwargs):
        super().__init__(master, **kwargs)

        self.options = options
        self.command = command
        self.vars = {opt: ctk.BooleanVar(value=False) for opt in options}

        self.button = ctk.CTkButton(
            self,
            text="Select Months",
            command=self.open_dropdown,
            width=140
        )
        self.button.pack()

        self.dropdown = None

    def open_dropdown(self):
        if self.dropdown and self.dropdown.winfo_exists():
            self.dropdown.destroy()

        self.dropdown = ctk.CTkToplevel(self)
        self.dropdown.overrideredirect(True)

        x = self.winfo_rootx()
        y = self.winfo_rooty() + self.button.winfo_height()
        self.dropdown.geometry(f"120x{len(self.options)*30}+{x}+{y}")

        for opt in self.options:
            cb = ctk.CTkCheckBox(
                self.dropdown,
                text=opt,
                variable=self.vars[opt],
                command=self._on_checkbox,
                fg_color="#bfcbd1"
            )
            cb.pack(anchor="w", padx=10, pady=2)

        self.dropdown.bind("<FocusOut>", lambda e: self.dropdown.destroy())
        self.dropdown.focus()

    def _on_checkbox(self):
        if self.command:
            self.command(self.get_selected())

    def get_selected(self):
        return [opt for opt, var in self.vars.items() if var.get()]
