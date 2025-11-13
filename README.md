# 🧩 Crossword Puzzle Game

### **DSA Mini Project – 3rd Semester**

A **fully interactive console-based Crossword Puzzle Game** built for my
**Data Structures and Algorithms (DSA) Mini Project – 3rd Semester.**

The game features crossword solving, a graph-based word connectivity system, and a persistent leaderboard — all implemented using core DSA concepts.

---

# 📸 Screenshots
input
<img width="1806" height="1047" alt="Screenshot 2025-11-13 115501" src="https://github.com/user-attachments/assets/6be83bb6-53ac-49a5-a036-19370a180a8b" />
output
<img width="1819" height="1059" alt="Screenshot 2025-11-13 115530" src="https://github.com/user-attachments/assets/6553bf7e-d440-49f6-933d-cd4e5e3fc50d" />



# ⭐ Features

### ✔️ **Crossword Puzzle Engine**

* Multiple puzzles organized in a puzzle bank
* Random puzzle selection
* Real-time crossword grid rendering using ASCII graphics
* Shows solved letters only
* Reveal full solution
* Percentage progress display

### ✔️ **Word Guessing System**

* Each clue has a unique ID
* Command:

```
guess <id> <word>
```

* Instant validation
* Updates crossword grid

---

### ✔️ **Graph-Based Word Connectivity (Non-Linear DS)**

Crossword words are represented as a **graph**:

* Word → Vertex
* Intersection → Edge
* Edges stored using **linked-list adjacency lists**

Command:

```
graph
```

Shows:

* Connectivity status
* Degree of each node
* List of neighbors
* DFS traversal results

---

### ✔️ **Leaderboard System (Persistent CSV Storage)**

* Stores:

  * Player name
  * Words solved
  * Time taken
  * Timestamp
* Sorted dynamically using custom comparator
* Saved to `leaderboard.csv`
* Persistent across restarts

Commands:

```
leaderboard
leaderboard <n>
leaderboard all
submit <name>
```

---

# 🧠 Data Structures Used

### 🟦 **1. Dynamic Arrays**

Used for:

* Word list
* Puzzle list
* Leaderboard entries
* Grid characters

### 🟩 **2. 2D Array (Matrix)**

Used for storing the crossword board:

```
char **cells;
```

### 🟨 **3. Linked List (Linear DS)**

Used in graph adjacency list:

```c
typedef struct GraphNode {
    size_t index;
    struct GraphNode *next;
} GraphNode;
```

### 🟥 **4. Graph (Non-linear DS)**

Adjacency list representation:

```c
typedef struct {
    size_t num_vertices;
    GraphNode **adj;
} Graph;
```

### 🟧 **5. File Handling + CSV Parsing**

Used to load/save leaderboard entries.

### 🟪 **6. Sorting Algorithms**

Leaderboard sorted using:

```
qsort()
```

With custom comparator ranking players.

---

# 📂 File Structure

```
📁 Crossword-DSA-Project
│── main.c
│── crossword.c
│── crossword.h
│── graph.c
│── graph.h
│── linked_list.c
│── linked_list.h
│── leaderboard.c
│── leaderboard.h
│── leaderboard.csv
│── README.md
```

---

# ⌨ Commands

```
help               Show all commands
clues              List all clues
guess <id> <word>  Guess a word
show               Display crossword with solved letters
reveal             Reveal full crossword
progress           Show completion percentage
graph              Show connectivity graph
leaderboard        Show top 10 leaderboard entries
leaderboard <n>    Show top n
leaderboard all    Show all entries
lb                 Shortcut for leaderboard
submit <name>      Submit score to leaderboard
next / skip        Load a new random puzzle
quit               Exit game
```

---

# ▶️ How to Compile & Run

### **Compile:**

```bash
gcc -std=c11 -Wall -Wextra main.c crossword.c linked_list.c graph.c leaderboard.c -o crossword
```

### **Run:**

```bash
./crossword
```

Windows:

```
.\crossword.exe
```

---

# 🎯 Purpose of the Project

This project was created as my:

> **📘 Data Structures & Algorithms (DSA) Mini Project – 3rd Semester**

It demonstrates practical applications of:

* Graphs
* Linked Lists
* Dynamic Arrays
* Sorting
* File Handling
* String Manipulation
* Recursion (DFS)
* Modular code design

---

# 🚀 Future Improvements

* GUI version (SDL/Qt/GTK)
* Timer UI
* Color-coded terminal output
* Multi-level difficulty
* Online scoreboard integration
* Puzzle editor

---

# 🏁 Conclusion

This Crossword Puzzle Game showcases how multiple **DSA concepts** can be combined to build a fully interactive and functional application.
It features a puzzle engine, leaderboard, graph connectivity, dynamic memory allocation, and modular C programming — making it a well-rounded project for understanding data structures in real applications.

