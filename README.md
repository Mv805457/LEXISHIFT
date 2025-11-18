# 🚀 LexiShift — Smart Autocomplete System

LexiShift is a high-performance **Trie-based autocomplete and word-suggestion engine** written in C.  
It includes three interfaces:

- **C Command-Line Autocomplete Tool**  
- **Raylib GUI (C)** – Modern graphical typing interface  
- **Python GUI (Tkinter)** – Uses a C-based DLL for suggestions  

LexiShift provides real-time suggestions, prefix search, multi-word completion, and a modular structure suitable for text editors, AI assistants, and search bars.

---

## 🔥 Features

- ⚡ **Fast Trie-based autocomplete** (O(length of word))  
- 📝 Real-time suggestions while typing  
- 🧠 Multi-word prediction  
- 🗂 Custom dictionary loading (words_alpha.txt)  
- 🎨 Raylib GUI with smooth UI and suggestion cards  
- 🐍 Python GUI using ctypes + DLL  
- ⌨ ALT-Key shortcuts for navigation & selection  
- ➕ Add/delete/update dictionary words  
- 📦 Launcher menu to choose CLI / Raylib / Python mode  
- 🟩 Matrix-style intro animation  

---

## 🧠 Tech Stack

### Core Engine
- **C (Trie, autocomplete, UI logic)**  

### GUI
- **Raylib (C Graphics)**  
- **Python + Tkinter**  
- **DLL (C compiled library)**  

### Tools
- GCC (w64devkit)  
- Git & GitHub  

---

## 🛠 Installation & Setup

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/Mv805457/LEXISHIFT.git
cd LEXISHIFT
2️⃣ Build the C Console Version
bash
Copy code
gcc src/main.c src/input.c src/matrix_intro.c src/ui.c src/trie.c -Iinclude -o lexishift.exe
3️⃣ Build the Raylib GUI Version
bash
Copy code
"C:\raylib\w64devkit\bin\gcc" -Iinclude -I"C:\raylib\raylib\include" src/lexishift_gui.c src/trie.c -L"C:\raylib\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm -o lexishift_gui.exe
4️⃣ Run the Python GUI
bash
Copy code
python gui.py
Make sure lexishift.dll is in the same folder.

💻 How to Use
▶ CLI Version
Type a prefix → instant suggestions

Press TAB to cycle suggestions

Press ALT + 0–9 to pick a suggestion

Shortcut keys:

ALT+A → Move to start

ALT+B → Move to end

ALT+C → Select first word

ALT+D → Select last word

ALT+Backspace → Clear entire sentence

🎨 Raylib GUI Version
Modern GUI with input box

Suggestions appear in card-style blocks

Click on a suggestion or use ALT+0–9

Clean launcher menu

🐍 Python GUI Version
Tkinter-based clean UI

Uses C DLL for autocomplete

Supports multi-word replacement

📁 Project Structure
graphql
Copy code
LEXISHIFT/
├── include/              # Header files  
├── src/                  # Core C source  
│   ├── trie.c            # Trie implementation  
│   ├── input.c           # Input + prefix detection  
│   ├── lexishift_gui.c   # Raylib GUI  
│   ├── launcher.c        # Mode launcher  
│   ├── matrix_intro.c    # Matrix intro animation  
│   └── ui.c              # CLI UI helpers  
├── gui.py                # Python GUI  
├── lexishift.dll         # DLL for Python  
├── lexishift.exe         # CLI executable  
├── lexishift_gui.exe     # Raylib UI executable  
└── README.md  
👥 Contributors
Name	Role
Madhav Vinod	Core Trie engine, DLL, Python GUI, debugging, GitHub
Nikhil Sai	Trie logic, prefix algorithm, performance tuning
Harshith	Raylib GUI development, launcher & UI design

🙏 Acknowledgements
Raylib – Simple and powerful graphics library

Tkinter – Python GUI framework

words_alpha.txt – Dictionary dataset

Faculty for guidance and support

⭐ Future Improvements
Word frequency ranking

Sentence context prediction

Grammar correction

Cloud dictionary updates

📜 License
This project is for educational use.
