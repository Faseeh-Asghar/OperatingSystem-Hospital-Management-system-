# Hospital Triage System 🏥

This is our semester project for the Operating Systems Lab. It simulates a hospital triage and admission system using C and core OS concepts.

## Overview
We modeled real hospital ER workflows using standard OS primitives:
- **Processes (`fork` / `exec`)**: Simulates different hospital departments running simultaneously.
- **IPC (Pipes)**: Handles communication between the admissions desk and the central scheduler.
- **Threads & Synchronization (Mutexes)**: Manages concurrent patient arrivals safely without data corruption or race conditions.
- **Priority Scheduling**: Ensures critical patients are allocated beds before stable ones.

## Project Structure
- `src/` - All the C source code and header files.
- `scripts/` - Bash scripts to automate starting and stopping the simulation.
- `Makefile` - Instructions for compiling the project.

## How to Compile & Run
Make sure you are running this in a Linux environment (like WSL or Ubuntu).

1. **Compile the project:**
   ```bash
   make
   ```

2. **Start the simulation:**
   ```bash
   ./scripts/start_hospital.sh
   ```

3. **Clean up compiled files:**
   ```bash
   make clean
   ```
