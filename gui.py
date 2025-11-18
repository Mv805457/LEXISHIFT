import customtkinter as ctk
import ctypes

# -----------------------------------------
# Load DLL
# -----------------------------------------
lexi = ctypes.CDLL("./lexishift.dll")

lexi.createNode.restype = ctypes.c_void_p
lexi.insertTrie.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lexi.getSuggestions.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.POINTER(ctypes.c_char * 100 * 10)
]
lexi.getSuggestions.restype = ctypes.c_int

root = lexi.createNode()

# -----------------------------------------
# Load dictionary
# -----------------------------------------
dict_path = "C:\\Users\\madha\\Downloads\\words_alpha (1).txt"

with open(dict_path, "r") as f:
    for w in f:
        w = w.strip()
        if w.isalpha():
            lexi.insertTrie(root, w.encode())


# -----------------------------------------
# CustomTkinter UI
# -----------------------------------------
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("green")

app = ctk.CTk()
app.title("LexiShift – Python GUI Autocomplete")
app.geometry("700x500")

title = ctk.CTkLabel(app, text="LexiShift Autocomplete", font=("Consolas", 28))
title.pack(pady=20)

entry = ctk.CTkEntry(app, width=500, height=40, font=("Consolas", 20))
entry.pack(pady=10)

suggest_frame = ctk.CTkFrame(app)
suggest_frame.pack(pady=10)

suggest_labels = []
for i in range(10):
    lbl = ctk.CTkLabel(
        suggest_frame,
        text="",
        font=("Consolas", 18),
        anchor="w"
    )
    lbl.pack(fill="x", pady=2)
    suggest_labels.append(lbl)


# -----------------------------------------
# Suggestion click handler
# -----------------------------------------
def fill_suggestion(i):
    # Get preview text (after "1. ")
    full = suggest_labels[i].cget("text")[3:]

    entry.delete(0, "end")
    entry.insert(0, full)



# -----------------------------------------
# Update suggestions
# -----------------------------------------
def update_suggestions(event=None):
    text = entry.get()

    # Extract last word safely
    stripped = text.rstrip()
    if " " in stripped:
        # everything before last word
        prefix = stripped[:stripped.rfind(" ") + 1]
        last_word = stripped[stripped.rfind(" ") + 1:]
    else:
        prefix = ""
        last_word = stripped

    # Clear suggestions if empty
    if last_word == "":
        for lbl in suggest_labels:
            lbl.configure(text="")
        return

    # Prepare buffer
    OutArray = ctypes.c_char * 100
    out = (OutArray * 10)()

    count = lexi.getSuggestions(root, last_word.encode(), out)

    for i in range(10):
        if i < count:
            word = out[i].value.decode()

            # Build preview: prefix + suggestion
            preview = prefix + word

            suggest_labels[i].configure(text=f"{i+1}. {preview}")

            # Bind click
            suggest_labels[i].bind("<Button-1>",
                                   lambda e, idx=i: fill_suggestion(idx))
        else:
            suggest_labels[i].configure(text="")
            suggest_labels[i].unbind("<Button-1>")



entry.bind("<KeyRelease>", update_suggestions)

app.mainloop()
