# Water Park Simulation

> A concurrent C simulator of a water park where visitors move through attractions, queues, and support areas while a monitor process tracks the system state in real time.

## 📚 Academic Context

| | |
|---|---|
| **Degree** | BSc in Computer Science @ University of Madeira |
| **Course** | Operating Systems |
| **Year** | 2023/24 |

## 🛠️ Technologies & Concepts

- C
- GCC and Make
- POSIX threads (`pthread`)
- Mutexes and semaphores
- UNIX domain sockets
- Process creation with `fork`
- Configuration-driven simulation
- Concurrency control and synchronization
- Queue and capacity management
- Runtime monitoring and report generation

## 🏗️ Architecture / Approach

The project is split into two C programs. `simulador.c` is responsible for reading `simulador.conf`, initializing the park configuration, creating one thread per visitor, managing attraction queues and capacities with semaphores, and sending simulation events through a UNIX stream socket. `monitor.c` acts as the observer process: it opens the socket, receives events, updates counters and derived statistics, prints the current state to the terminal, and writes the final summary to `Resumo_Execucao.txt`.

The shared types, constants, zone identifiers, action codes, and function prototypes are centralized in `header.h`. The simulation model includes the park entrance, square, swimming, diving, slides, restaurant, changing rooms, and infirmary, with configurable waiting times, capacities, probabilities of injury, abandonment, zone changes, and recovery. The build is defined in `Makefile`, which produces the `simulador` and `monitor` executables and includes cleanup targets for generated files.

## 🚀 How to Run

1. Open the project folder in a terminal.
2. Compile the programs with `make`.
3. Open two terminals in the same project directory.
4. In the first terminal, start the monitor with `./monitor`.
5. In the second terminal, start the simulator with `./simulador simulador.conf`.
6. Wait for the simulation to finish and check the final report written to `Resumo_Execucao.txt`.
7. Use `make clean` to remove object files and executables, or `make fullClean` to also remove generated `.txt` files.

## 📝 Notes

- The simulation behavior is driven entirely by `simulador.conf`, so queue sizes, attraction times, probabilities, and total simulation time can be adjusted without changing the C source.
- The monitor keeps a live execution view and also computes final statistics such as abandonment rates, zone utilization rates, and average times spent in the park and attractions.
