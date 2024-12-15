# Low-Level-Data-Processing-and-Query-Engine

This program demonstrates a basic data processing and querying system. It involves generating data, storing it in a memory-mapped file, and then efficiently querying the data based on specified criteria. The core concepts used include memory mapping, custom memory allocation, a CRC-16 checksum for data integrity, bitwise operations, and binary search.

## Overview

The program performs the following steps:

1.  **Data Generation:**
    *   Generates a sequence of integers and saves them to a text file called `dataset.txt`.
2.  **Memory Mapping:**
    *   Creates a large (1GB) file named `memory_mapped.dat`.
    *   Maps this file into the program's address space using `mmap`.
    *   Reads the integers from dataset.txt, encodes them using the custom encoding scheme, and stores them sequentially in the mapped memory.
3.  **Data Encoding:**
    *   Encodes each integer along with its index and some flags into a 64-bit value using bitwise operations.
    *   A CRC-16 checksum is also calculated and included in the encoded data, which enhances data integrity.
4.  **Querying:**
    *   Defines a `QueryCriteria` struct that specifies the conditions for querying.
    *   Performs a binary search on the memory-mapped file to locate the first data entry that meets the criteria.
    *   The search is optimized for speed, as data is assumed to be in order.
5.  **Result Handling:**
    *   Displays the index of the first matching data entry (if found) and its decoded content.
    *   Prints the time it took to perform the search.
6.  **Cleanup:**
    *   Unmaps the memory using `munmap`.
    *   Closes the file descriptor.

## Key Components

### 1. MemoryAllocator

*   **Purpose:** A custom memory allocator to simulate a simplified buffer for memory management.
*   **Implementation:**
    *   It's initialized with a size and allocates a buffer in the constructor.
    *   `allocate(size_t size)`: Allocates memory from the buffer and keeps track of the allocated size.
    *   `deallocate(void* ptr, size_t size)`: For this simple case, this is an empty implementation, not tracking deallocation details.
    *   **Note:** This allocator is very basic and does not handle freeing individual allocations and can result in memory exhaustion if the allocated memory exceeds the initial size.

### 2. CRC-16 Checksum Calculation (`crc16`)

*   **Purpose:** Calculates a 16-bit Cyclic Redundancy Check (CRC) for error detection.
*   **Implementation:**
    *   Uses the standard CRC-16-CCITT polynomial (0x8408).
    *   Combines the input `value` and `index` with bitwise XOR before calculating the CRC.
*   **Formula:** While the code doesn't explicitly show the polynomial formula, the process follows this logic:
     1. Initialize crc = 0xFFFF;
     2. Combine data = value ^ index;
     3. For each bit in data:
       1. if crc ^ data & 0x0001 == 1: crc = (crc >> 1) ^ 0x8408;
       2. else: crc = crc >> 1;
       3. data = data >> 1;
     4. Final result is ~crc;

### 3. Integer Encoding and Decoding

*   **Purpose:** Encodes multiple pieces of information (value, index, flags, checksum) into a single 64-bit integer.
*   **`encode_integer` Implementation:**
    *   Combines the 16-bit `value`, 16-bit `index`, 16-bit `flags`, and 16-bit checksum into a single 64-bit value using bitwise OR and left bit-shifts (`<<`). The structure is as follows:
    
      ```
      | Checksum (16 bits) | Flags (16 bits) | Index (16 bits) | Value (16 bits) |
      ```
*   **`decode_integer` Implementation:**
    *   Extracts the original `value`, `index`, `flags`, and `checksum` from the encoded 64-bit value, using bitwise AND and right bit-shifts (`>>`).

### 4. Checksum Validation

*   **Purpose:** Verifies the integrity of data by recalculating the CRC and comparing it to the stored checksum.
*   **`validate_checksum` Implementation:**
    *   Calculates the CRC using the `crc16` function and compares it with the provided checksum. It returns true if they are equal and false otherwise.

### 5. Querying

*   **`QueryCriteria` Struct:**
    *   Defines the filtering parameters for the data, including `min_value`, `max_value`, `min_index`, `max_index`, a `flag_mask`, and `checksum_validation`.
*   **`matches_criteria` Function:**
    *   Decodes the 64-bit integer.
    *   Performs a series of checks to confirm that the decoded data matches the criteria. If any of the condition are not met, returns false.
*   **`binary_search_memory_mapped` Function:**
    *   Performs a binary search on the memory-mapped data.
    *   Uses the `matches_criteria` function to check each element against the query conditions.
    *   If a match is found, updates the `result` index and continues the search in the left half. The search stops when the next left half will cause to exit the loop.

### 6. Memory Mapping (`mmap`)

*   **Purpose:** Enables direct manipulation of a file's content in memory, leading to improved performance for large datasets.
*   **Implementation:**
    *   The file `memory_mapped.dat` is opened, and its size is set to 1GB.
    *   `mmap` is used to map the file into the program's memory.
    *   Data can be directly written to and read from this mapped region, eliminating the need for standard file I/O for access.
*   **Cleanup:** `munmap` is used to release the mapped memory.

## Performance Considerations

*   **Memory Mapping:** Using `mmap` for file access can be significantly faster than standard I/O, as it eliminates system calls for reads and writes.
*   **Binary Search:** This allows for efficient searching of sorted data.
*   **Data Encoding:** Compacting the data into a 64-bit value reduces memory consumption and improves the efficiency of searches, especially when used with memory mapping.

## How to Compile and Run

1.  **Save:** Save the code as a `main.cpp` file.
2.  **Compile:** Use a C++ compiler that supports C++11 or later. For example:
    ```bash
    g++ -std=c++11 main.cpp -o query_engine
    ```
3.  **Run:** Execute the compiled program:
    ```bash
    ./query_engine
    ```


-------------------------

### Example Calculation 

The CRC-16-CCITT algorithm is a widely used error-detecting code that generates a 16-bit checksum. It uses a specific polynomial for its calculations: $$x^{16} + x^{12} + x^{5} + 1$$, which corresponds to the hexadecimal value 0x1021. However, in the provided code, the polynomial is represented as 0x8408, which is the reflected version of 0x1021 for the implementation details.

#### Algorithm Steps

1. **Initialization**:
   - Start with a 16-bit register initialized to 0xFFFF.

2. **Processing Each Byte**:
   - For each byte in the data:
     - XOR the top 8 bits of the register with the current data byte.
     - For each of the 8 bits in the byte:
       - If the least significant bit (LSB) of the register is 1, shift the register right and XOR it with the polynomial (0x8408).
       - If the LSB is 0, just shift the register right.

3. **Final Checksum**:
   - The final value in the register is the checksum.

#### Example Data

Let's calculate the CRC-16-CCITT for the data sequence: 0x31, 0x32, 0x33 (corresponding to ASCII '1', '2', '3').

#### Step-by-Step Calculation

1. **Initialization**:
   $$
   \text{register} = 0xFFFF
   $$

2. **Processing the First Byte (0x31)**:

   - **XOR register with data byte**:
     $$
     0xFFFF \oplus 0x31 = 0xFFC0
     $$

   - **Bitwise Processing**:
     - Bit 7: 0x80 (128)
       - LSB of 0xFFC0 is 0 (register: 0xFFC0 & 0x0001 = 0)
       - Shift right: 0xFFC0 >> 1 = 0x7FF8
     - Bit 6: 0x40 (64)
       - LSB of 0x7FF8 is 0
       - Shift right: 0x7FF8 >> 1 = 0x3FFC
     - Bit 5: 0x20 (32)
       - LSB of 0x3FFC is 0
       - Shift right: 0x3FFC >> 1 = 0x1FFE
     - Bit 4: 0x10 (16)
       - LSB of 0x1FFE is 0
       - Shift right: 0x1FFE >> 1 = 0x0FFE
     - Bit 3: 0x08 (8)
       - LSB of 0x0FFE is 0
       - Shift right: 0x0FFE >> 1 = 0x07FE
     - Bit 2: 0x04 (4)
       - LSB of 0x07FE is 0
       - Shift right: 0x07FE >> 1 = 0x03FE
     - Bit 1: 0x02 (2)
       - LSB of 0x03FE is 0
       - Shift right: 0x03FE >> 1 = 0x01FE
     - Bit 0: 0x01 (1)
       - LSB of 0x01FE is 0
       - Shift right: 0x01FE >> 1 = 0x00FE

   - **Final register after first byte**: 0x00FE

3. **Processing the Second Byte (0x32)**:

   - **XOR register with data byte**:
     $$
     0x00FE \oplus 0x32 = 0x33D
     $$

   - **Bitwise Processing**:
     - Bit 7: 0x80 (128)
       - LSB of 0x033D is 1 (register: 0x033D & 0x0001 = 1)
       - Shift right and XOR with 0x8408:
         $$
         (0x033D >> 1) = 0x019E
         $$
         $$
         0x019E \oplus 0x8408 = 0x85A6
         $$
     - Bit 6: 0x40 (64)
       - LSB of 0x85A6 is 0
       - Shift right: 0x85A6 >> 1 = 0x42D3
     - Bit 5: 0x20 (32)
       - LSB of 0x42D3 is 1
       - Shift right and XOR with 0x8408:
         $$
         (0x42D3 >> 1) = 0x2169
         $$
         $$
         0x2169 \oplus 0x8408 = 0x8561
         $$
     - Bit 4: 0x10 (16)
       - LSB of 0x8561 is 1
       - Shift right and XOR with 0x8408:
         $$
         (0x8561 >> 1) = 0x42B0
         $$
         $$
         0x42B0 \oplus 0x8408 = 0x86B8
         $$
     - Bit 3: 0x08 (8)
       - LSB of 0x86B8 is 0
       - Shift right: 0x86B8 >> 1 = 0x435C
     - Bit 2: 0x04 (4)
       - LSB of 0x435C is 0
       - Shift right: 0x435C >> 1 = 0x21AE
     - Bit 1: 0x02 (2)
       - LSB of 0x21AE is 0
       - Shift right: 0x21AE >> 1 = 0x10D7
     - Bit 0: 0x01 (1)
       - LSB of 0x10D7 is 1
       - Shift right and XOR with 0x8408:
         $$
         (0x10D7 >> 1) = 0x086B
         $$
         $$
         0x086B \oplus 0x8408 = 0x8C63
         $$

   - **Final register after second byte**: 0x8C63

4. **Processing the Third Byte (0x33)**:

   - **XOR register with data byte**:
     $$
     0x8C63 \oplus 0x33 = 0x8C50
     $$

   - **Bitwise Processing**:
     - Bit 7: 0x80 (128)
       - LSB of 0x8C50 is 0
       - Shift right: 0x8C50 >> 1 = 0x4628
     - Bit 6: 0x40 (64)
       - LSB of 0x4628 is 0
       - Shift right: 0x4628 >> 1 = 0x2314
     - Bit 5: 0x20 (32)
       - LSB of 0x2314 is 0
       - Shift right: 0x2314 >> 1 = 0x118A
     - Bit 4: 0x10 (16)
       - LSB of 0x118A is 0
       - Shift right: 0x118A >> 1 = 0x08C5
     - Bit 3: 0x08 (8)
       - LSB of 0x08C5 is 1
       - Shift right and XOR with 0x8408:
         $$
         (0x08C5 >> 1) = 0x0462
         $$
         $$
         0x0462 \oplus 0x8408 = 0x886A
         $$
     - Bit 2: 0x04 (4)
       - LSB of 0x886A is 0
       - Shift right: 0x886A >> 1 = 0x4435
     - Bit 1: 0x02 (2)
       - LSB of 0x4435 is 1
       - Shift right and XOR with 0x8408:
         $$
         (0x4435 >> 1) = 0x221A
         $$
         $$
         0x221A \oplus 0x8408 = 0x8612
         $$
     - Bit 0: 0x01 (1)
       - LSB of 0x8612 is 0
       - Shift right: 0x8612 >> 1 = 0x4309

   - **Final register after third byte**: 0x4309

5. **Final Checksum**:
   - The final value in the register is 0x4309. This is the CRC-16-CCITT checksum for the data sequence 0x31, 0x32, 0x33.
