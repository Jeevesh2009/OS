# Mini Project 0 

## Part 1: File Management

### 📌 Features
- Creates a new file `newfile.txt` with **0644 permissions** on startup.  
- If the file already exists, clears its contents (fresh start).  
- Supports three commands:
  - **INPUT** → takes user input and appends it to the file (newline at end).  
  - **PRINT** → displays entire file content.  
  - **STOP** → exits program.  
- Handles multi-word input (e.g., *Hello World!*).  
- Error handling for failed system calls.  

---

### 📖 Documentation (System Calls Used)

#### 1. `open()`
Used to create/open files.  
```c
int fd = open("newfile.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
```

* **O\_CREAT** → create file if it doesn’t exist  
* **O\_WRONLY** → open in write-only mode  
* **O\_TRUNC** → clear contents if file already exists  
* **0644** → file permissions = `rw-r--r--` (owner can read/write, others read only)

---

#### 2. `write()`

```c
write(fd, input, strlen(input));
write(fd, "\n", 1);
```

* **First arg** = file descriptor  
* **Second arg** = data buffer  
* **Third arg** = number of bytes to write

---

#### 3. `read()`

```c
char buffer[100];
int bytesRead = read(fd, buffer, sizeof(buffer) - 1);
buffer[bytesRead] = '\0';
```

* Reads up to `sizeof(buffer)-1` bytes  
* Returns number of bytes read (`0 = EOF`)  
* Buffer must be **null-terminated** manually for safe printing

---

#### 4. `close()`

```c
close(fd);
```

Always close file descriptors after use.

---

## Part 2: Process Creation and Management

### Overview
This project explores **process creation and management** using Unix system calls.  
We implemented three tasks to demonstrate:
- Process memory behavior with `fork()`
- Replacing process image using `exec()`
- Orphan process adoption by `init/systemd`

The program is fully modular and uses **system calls** (`open`, `read`, `write`, `close`, `fork`, `exec`, `wait`) with proper error handling.

---

### Tasks

#### **Task 1 – Variable Inheritance**
- Parent sets `x = 25` and forks.
- Child receives a **copy** of `x`.
- Both parent and child modify `x` independently.

**Expected Behavior:**  
- Both processes print different values of `x` after modification → *copy-on-write memory*.

---

#### **Task 2 – Child with `exec()`**
- Parent forks a child.
- Child calls `exec()` and becomes a new program (writer mode).
- Writer creates `newfile.txt` and writes its **parent PID** into the file.
- Parent waits for child and then displays the file content.

**Expected Behavior:**  
- `newfile.txt` should contain the **original parent PID**.  
- Confirms that **`exec()` replaces process image but keeps PID**.

---

#### **Task 3 – Orphan Process**
- Parent forks child and immediately exits.
- Child continues execution and prints its **new parent PID** (`getppid()`).
- The child also reads and prints what was recorded in `newfile.txt` during Task 2.

**Expected Behavior:**  
- Child’s new parent PID should be `1` (or `systemd/init` PID).  
- Confirms that orphans are adopted by `init/systemd`.

---

## Sample Output (from test run)

```
=== Mini Project 0: Part 2 (Process Creation & Management) ===

===== Task 1: Variable Inheritance =====
[parent 151540] initial x = 25
[child 151541] received x = 25
[child 151541] modified x = 35
[parent 151540] modified x = 20 (independent of child)
Observation: parent & child have separate copies (copy-on-write).

===== Task 2: Child exec() writes parent PID to file =====
[writer] Wrote to newfile.txt: Parent PID recorded by exec'd child: 151540
[parent 151540] writer child exited with code 0
Contents of newfile.txt after Task-2:
Parent PID recorded by exec'd child: 151540

===== Task 3: Orphan Process Observation =====
[parent 151540] exiting immediately to orphan the child 151543
[child 151543] post-orphan getppid() = 1642 (likely 1: init/systemd)
[child 151543] Previously recorded in newfile.txt:
Parent PID recorded by exec'd child: 151540
```

---

### Observations
1. **Fork:** Child gets an independent copy of variables (copy-on-write).
2. **Exec:** Child can replace its memory but retains the same PID. Parent-child relation persists.
3. **Orphan:** When parent exits, child is adopted by `init`/`systemd` → new `ppid`.

---

### Documentation

#### How to Compile & Run
```bash
gcc processManagement.c
```

### System Calls Used
- `open`, `write`, `read`, `close` → low-level file management
- `fork` → process creation
- `exec` → replace process image
- `waitpid` → parent waits for child
- `getpid`, `getppid` → retrieve process IDs
- `exit`, `_exit` → process termination

---
