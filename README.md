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

### Example Calculation the CRC-16-CCITT checksum for a given data sequence.

I will provide an example calculation related to the CRC-16-CCITT checksum calculation, which is a key component in the provided code. CRC (Cyclic Redundancy Check) is a method of detecting accidental changes to raw data, and CRC-16-CCITT is a specific variant of this method.

### Example Calculation: CRC-16-CCITT

**Objective**: Calculate the CRC-16-CCITT checksum for a given data sequence.

**Given Data**: Let's consider the data sequence `0x31 0x32 0x33 0x34`, which corresponds to the ASCII values of '1', '2', '3', '4'.

**Polynomial**: The CRC-16-CCITT uses the polynomial $$ x^{16} + x^{12} + x^{5} + 1 $$, which corresponds to the hexadecimal value `0x1021`. However, in the code provided, the polynomial used in the shift operation is `0x8408`, which is the reflected version of `0x1021` for byte-wise processing.

**Initialization**:
- Initial register value: `0xFFFF` (all bits set to 1).

**Algorithm**:
1. For each byte in the data:
   a. XOR the register with the current data byte.
   b. For each bit in the byte (8 times):
      i. If the high bit of the register is set, shift the register to the right by one and XOR with the polynomial (`0x8408`).
      ii. Otherwise, just shift the register to the right by one.

2. The final register value is the checksum.

### Step-by-Step Calculation

Let's go through the calculation step by step for the data sequence `0x31 0x32 0x33 0x34`.

#### Step 1: Initialize the register
$$ \text{register} = 0xFFFF $$

#### Step 2: Process each byte

**First byte: 0x31**

1. XOR register with data byte:
   $$ 0xFFFF \oplus 0x31 = 0xFFCC $$

2. For each bit in the byte (8 times):

   - Bit 1:
     - High bit of 0xFFCC is 1 (0b1111111111001100)
     - Shift right: 0x7FF9
     - XOR with polynomial 0x8408:
       $$ 0x7FF9 \oplus 0x8408 = 0xF7F1 $$

   - Bit 2:
     - High bit of 0xF7F1 is 1 (0b1111011111110001)
     - Shift right: 0x7BFB
     - XOR with polynomial 0x8408:
       $$ 0x7BFB \oplus 0x8408 = 0xFDF3 $$

   - Bit 3:
     - High bit of 0xFDF3 is 1 (0b1111110111110011)
     - Shift right: 0x7DF9
     - XOR with polynomial 0x8408:
       $$ 0x7DF9 \oplus 0x8408 = 0xF9F1 $$

   - Bit 4:
     - High bit of 0xF9F1 is 1 (0b1111100111110001)
     - Shift right: 0x7CF3
     - XOR with polynomial 0x8408:
       $$ 0x7CF3 \oplus 0x8408 = 0xF8F9 $$

   - Bit 5:
     - High bit of 0xF8F9 is 1 (0b1111100011111001)
     - Shift right: 0x7A7D
     - XOR with polynomial 0x8408:
       $$ 0x7A7D \oplus 0x8408 = 0xFEA5 $$

   - Bit 6:
     - High bit of 0xFEA5 is 1 (0b1111111010100101)
     - Shift right: 0x7F59
     - XOR with polynomial 0x8408:
       $$ 0x7F59 \oplus 0x8408 = 0xF951 $$

   - Bit 7:
     - High bit of 0xF951 is 1 (0b1111100101010001)
     - Shift right: 0x7C93
     - XOR with polynomial 0x8408:
       $$ 0x7C93 \oplus 0x8408 = 0xF89B $$

   - Bit 8:
     - High bit of 0xF89B is 1 (0b1111100011011011)
     - Shift right: 0x7C4D
     - XOR with polynomial 0x8408:
       $$ 0x7C4D \oplus 0x8408 = 0xF845 $$

After processing the first byte, the register is `0xF845`.

**Second byte: 0x32**

1. XOR register with data byte:
   $$ 0xF845 \oplus 0x32 = 0xC077 $$

2. For each bit in the byte (8 times):

   - Bit 1:
     - High bit of 0xC077 is 1 (0b1100000001110111)
     - Shift right: 0x603B
     - XOR with polynomial 0x8408:
       $$ 0x603B \oplus 0x8408 = 0xE433 $$

   - Bit 2:
     - High bit of 0xE433 is 1 (0b1110010000110011)
     - Shift right: 0x7219
     - XOR with polynomial 0x8408:
       $$ 0x7219 \oplus 0x8408 = 0xF611 $$

   - Bit 3:
     - High bit of 0xF611 is 1 (0b1111011000010001)
     - Shift right: 0x7B03
     - XOR with polynomial 0x8408:
       $$ 0x7B03 \oplus 0x8408 = 0xFB0B $$

   - Bit 4:
     - High bit of 0xFB0B is 1 (0b1111101100001011)
     - Shift right: 0x7D85
     - XOR with polynomial 0x8408:
       $$ 0x7D85 \oplus 0x8408 = 0xF98D $$

   - Bit 5:
     - High bit of 0xF98D is 1 (0b1111100110001101)
     - Shift right: 0x7CB9
     - XOR with polynomial 0x8408:
       $$ 0x7CB9 \oplus 0x8408 = 0xF8B1 $$

   - Bit 6:
     - High bit of 0xF8B1 is 1 (0b1111100010110001)
     - Shift right: 0x7A53
     - XOR with polynomial 0x8408:
       $$ 0x7A53 \oplus 0x8408 = 0xFE5B $$

   - Bit 7:
     - High bit of 0xFE5B is 1 (0b1111111001011011)
     - Shift right: 0x7F2D
     - XOR with polynomial 0x8408:
       $$ 0x7F2D \oplus 0x8408 = 0xF925 $$

   - Bit 8:
     - High bit of 0xF925 is 1 (0b1111100100100101)
     - Shift right: 0x7C99
     - XOR with polynomial 0x8408:
       $$ 0x7C99 \oplus 0x8408 = 0xF891 $$

After processing the second byte, the register is `0xF891`.

**Third byte: 0x33**

1. XOR register with data byte:
   $$ 0xF891 \oplus 0x33 = 0xC0A2 $$

2. For each bit in the byte (8 times):

   - Bit 1:
     - High bit of 0xC0A2 is 1 (0b1100000010100010)
     - Shift right: 0x6059
     - XOR with polynomial 0x8408:
       $$ 0x6059 \oplus 0x8408 = 0xE451 $$

   - Bit 2:
     - High bit of 0xE451 is 1 (0b1110010001010001)
     - Shift right: 0x7223
     - XOR with polynomial 0x8408:
       $$ 0x7223 \oplus 0x8408 = 0xF62B $$

   - Bit 3:
     - High bit of 0xF62B is 1 (0b1111011000101011)
     - Shift right: 0x7B1D
     - XOR with polynomial 0x8408:
       $$ 0x7B1D \oplus 0x8408 = 0xFB15 $$

   - Bit 4:
     - High bit of 0xFB15 is 1 (0b1111101100010101) 
     - Shift right: 0x7D8A
     - XOR with polynomial 0x8408:
       $$ 0x7D8A \oplus 0x8408 = 0xF982 $$

   - Bit 5:
     - High bit of 0xF982 is 1 (0b1111100110000010)
     - Shift right: 0x7C99
     - XOR with polynomial 0x8408:
       $$ 0x7C99 \oplus 0x8408 = 0xF891 $$

   - Bit 6:
     - High bit of 0xF891 is 1 (0b1111100010010001)
     - Shift right: 0x7A43
     - XOR with polynomial 0x8408:
       $$ 0x7A43 \oplus 0x8408 = 0xFE4B $$

   - Bit 7:
     - High bit of 0xFE4B is 1 (0b1111111001001011)
     - Shift right: 0x7F25
     - XOR with polynomial 0x8408:
       $$ 0x7F25 \oplus 0x8408 = 0xF92D $$

   - Bit 8:
     - High bit of 0xF92D is 1 (0b1111100100101101)
     - Shift right: 0x7C99
     - XOR with polynomial 0x8408:
       $$ 0x7C99 \oplus 0x8408 = 0xF891 $$

After processing the third byte, the register is `0xF891`.

**Fourth byte: 0x34**

1. XOR register with data byte:
   $$ 0xF891 \oplus 0x34 = 0xC0A5 $$

2. For each bit in the byte (8 times):

   - Bit 1:
     - High bit of 0xC0A5 is 1 (0b1100000010100101)
     - Shift right: 0x6059
     - XOR with polynomial 0x8408:
       $$ 0x6059 \oplus 0x8408 = 0xE451 $$

   - Bit 2:
     - High bit of 0xE451 is 1 (0b1110010001010001)
     - Shift right: 0x7223
     - XOR with polynomial 0x8408:
       $$ 0x7223 \oplus 0x8408 = 0xF62B $$

   - Bit 3:
     - High bit of 0xF62B is 1 (0b1111011000101011)
     - Shift right: 0x7B1D
     - XOR with polynomial 0x8408:
       $$ 0x7B1D \oplus 0x8408 = 0xFB15 $$

   - Bit 4:
     - High bit of 0xFB15 is 1 (0b1111101100010101)
     - Shift right: 0x7D8A
     - XOR with polynomial 0x8408:
       $$ 0x7D8A \oplus 0x8408 = 0xF982 $$

   - Bit 5:
     - High bit of 0xF982 is 1 (0b1111100110000010)
     - Shift right: 0x7C99
     - XOR with polynomial 0x8408:
       $$ 0x7C99 \oplus 0x8408 = 0xF891 $$

   - Bit 6:
     - High bit of 0xF891 is 1 (0b1111100010010001)
     - Shift right: 0x7A43
     - XOR with polynomial 0x8408:
       $$ 0x7A43 \oplus 0x8408 = 0xFE4B $$

   - Bit 7:
     - High bit of 0xFE4B is 1 (0b1111111001001011)
     - Shift right: 0x7F25
     - XOR with polynomial 0x8408:
       $$ 0x7F25 \oplus 0x8408 = 0xF92D $$

   - Bit 8:
     - High bit of 0xF92D is 1 (0b1111100100101101)
     - Shift right: 0x7C99 
     - XOR with polynomial 0x8408:
       $$0x7C99 \oplus 0x8408 = 0xF891$$

After processing the fourth byte, the register is `0xF891`.

#### Step 3: Final Checksum

The final register value is `0xF891`. This is the CRC-16-CCITT checksum for the data sequence `0x31 0x32 0x33 0x34`.

### Explanation

The CRC-16-CCITT algorithm works by treating the data and the polynomial as binary polynomials and performing polynomial division over GF(2). The remainder of this division is the checksum. In practice, this is implemented using bitwise operations for efficiency.

Each byte of the data is processed one at a time. For each byte, the register is XORed with the data byte, and then for each bit in the byte, the register is shifted and conditionally XORed with the polynomial if the high bit is set. This process effectively performs the polynomial division step by step.

The final register value, after all bytes have been processed, is the checksum. This checksum can then be stored or transmitted with the data to verify its integrity later.

This example demonstrates the detailed steps involved in computing the CRC-16-CCITT checksum for a simple data sequence, providing insight into the bit-level operations that ensure data integrity in various applications.
