# Multithreaded Sheet Pricing System (C++)

> 🧵 A multithreaded simulation of a welding company handling simultaneous customer requests using producer-consumer synchronization, dynamic programming, and C++ threads.

---

## Project Overview

This project focuses specifically on **multithreading in C++**. It implements an advanced version of the classic **Producer/Consumer problem**, set in the context of a fictional **welding company** (`CWeldingCompany`) that sells and prices **metal sheets** (`COrder`).

The system is designed to handle multiple **concurrent customers** (`CCustomer`) requesting pricing for specific types of material sheets. Each customer submits an order list (`COrderList`) containing requested sheet formats, identified by a `material_id`. The sheets are priced using supplier price lists (`CPriceList`) fetched from available **producers** (`CProducer`).

---

## Problem Specifics

- **Sheets cannot be cut** — they can only be **combined (welded)**.
- Welding is only possible along edges of **equal length** (width or height).
- Example: A 3x4 sheet can be welded with a 3x6 to make a 3x10. Then it can be welded with a 4x9 if their heights match, resulting in a 7x9 sheet.

---

## Thread Design

The program utilizes two main thread types:
1. **Service Threads** (per customer)
2. **Worker Threads** (shared thread pool)

### Service Threads

- Each customer is assigned a **dedicated service thread**.
- This thread:
  - Receives the customer’s order list.
  - Checks if the system already has a cached **merged price list** for the requested material.
  - If not, it concurrently queries all suppliers (producers) — which may respond **synchronously or asynchronously** — and waits for all responses.
  - Merges all price lists into a unified, duplicate-free list, preferring the cheaper sheet in case of duplicates.
  - Fills a **shared buffer** with jobs (requested sheets), annotated with the corresponding customer and request metadata.
  - Notifies worker threads that work is available.

### Worker Threads

- Each thread:
  - Awaits new tasks from the shared buffer.
  - Uses the `seqSolve` method (based on **dynamic programming**) to calculate the **minimum cost** to build the requested sheet from available supplier sheets.
  - Updates the order status.
  - When all sheets in an order are priced, returns the results to the customer.

- `seqSolve` builds the optimal combination of sheets in **cubic time**, storing intermediate results for reuse. Since each thread processes only one sheet at a time, **no locking is needed**, and threads can operate in full parallelism.

---

## Graceful Shutdown

- When a service thread detects that a customer has no more orders, it decrements the active service thread counter.
- Once all service threads are complete, the **last one triggers shutdown** by pushing a number of **dummy jobs** into the buffer equal to the number of worker threads.
- Worker threads detect these dummy jobs and terminate gracefully.
- The main thread joins all service and worker threads during shutdown.

---

## Repository Structure

### `progtest/solution.cpp`

- Contains the full C++ implementation of the multithreaded system.

### `Makefile`

- Use `make` to compile the project.
- Use `./test` to run the program.

### Testing Instructions

To enable **custom testing**:
1. Comment out the `main()` function in `solution.cpp`.
2. Uncomment the `testing.o` target in the `Makefile`.
3. The project will then compile `testing.cpp` along with the necessary dependencies.
4. Inside `testing.cpp`, you can configure:
   - Number of **worker threads**
   - Number of **customers**
   - Use of **synchronous/asynchronous producers**

### `dp_solver_solution.cpp`

- Contains the initial implementation of the **dynamic programming algorithm** used to compute minimum sheet pricing combinations.

---

## Technologies Used

- **C++ Standard Library** – `std::thread`, `std::mutex`, `std::condition_variable`, etc.
- **Multithreading and Synchronization**
- **Dynamic Programming**
- **Custom Buffer and Request Coordination**

---

## Purpose

This project demonstrates:
- Design and synchronization of multithreaded systems in C++
- Dynamic resource allocation and request handling
- Efficient price computation through dynamic programming
- Graceful and parallel task execution

---

## License

This project was developed as a school assignment for educational purposes.
