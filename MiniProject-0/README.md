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

