# 🐚 Shankh Shell

> A modern POSIX-inspired shell written in C++.

Shankh Shell is a Unix-like command-line shell built from scratch in C++. It supports command execution, pipelines, job control, variable expansion, command history, and intelligent tab completion while providing a lightweight and educational implementation of core shell concepts.

---

## ✨ Features

### Command Execution

* Execute external programs
* PATH resolution
* Interactive REPL

### Built-in Commands

* `cd`
* `pwd`
* `echo`
* `exit`
* `type`
* `history`
* `jobs`
* `declare`

### Variables

* Shell variables
* Environment variables
* Variable expansion
* Braced expansion

```bash
declare NAME=Shankh
echo $NAME
echo ${NAME}
```

### Pipelines & Redirection

* Single pipelines
* Multi-command pipelines
* Built-ins inside pipelines
* Input redirection (`<`)
* Output redirection (`>`)
* Append redirection (`>>`)

```bash
ls | grep cpp | wc -l
```

### Job Control

* Background execution (`&`)
* Job tracking
* Process reaping
* Job listing

```bash
sleep 30 &
jobs
```

### Productivity Features

* Persistent command history
* History loading on startup
* History saving on exit
* Tab autocompletion
* Command suggestions

---

## 🛠️ Build

### Prerequisites

* C++17 or newer
* CMake 3.15+
* POSIX-compatible environment

### Build Steps

```bash
git clone https://github.com/Nitin-kumar-yadav1307/shankh-shell.git
cd shankh-shell

mkdir build
cd build

cmake ..
cmake --build .
```

---

## 🚀 Example Session

```bash
$ declare PROJECT=Shankh

$ echo $PROJECT
Shankh

$ ls | grep cpp
main.cpp

$ sleep 60 &
[1] Running

$ jobs
[1] sleep 60

$ history
1 declare PROJECT=Shankh
2 echo $PROJECT
3 jobs
```

---

## 🏗️ Project Goals

Shankh Shell was created to explore:

* Process creation (`fork`, `exec`)
* Pipes and IPC
* Terminal interaction
* Shell parsing
* Job control
* Command execution
* Systems programming in C++

---

## 📈 Roadmap

* [ ] Alias support
* [ ] Configuration file support
* [ ] Scripting support
* [ ] Syntax highlighting
* [ ] Plugin system
* [ ] Advanced job control

---

## 🤝 Contributing

Contributions, bug reports, and feature requests are welcome.

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Open a pull request

---

## 📄 License

MIT License

---

## 👨‍💻 Author

**Nitin Kumar Yadav**

Built with C++ and a passion for systems programming.
