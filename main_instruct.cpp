 #include <iostream>
#include <fstream>
#include <vector>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <algorithm>

// Custom Memory Allocator (Simple Implementation)
class MemoryAllocator {
public:
    MemoryAllocator(size_t size) : size_(size), buffer_(new char[size]), allocated_(0) {}
    ~MemoryAllocator() { delete[] buffer_; }

    void* allocate(size_t size) {
        if (allocated_ + size > size_) {
            return nullptr; // Out of memory
        }
        void* ptr = buffer_ + allocated_;
        allocated_ += size;
        return ptr;
    }

    void deallocate(void* ptr, size_t size) {
    } // Simple allocator does not need deallocation tracking

private:
    size_t size_;
    char* buffer_;
    size_t allocated_;
};


// CRC-16 Checksum Calculation (Standard CRC-16-CCITT)
uint16_t crc16(uint16_t value, uint16_t index) {
    uint16_t crc = 0xFFFF;
    uint16_t data = value ^ index; // Combine value and index
    for (int i = 0; i < 16; ++i) {
        if ((crc ^ data) & 0x0001) {
            crc = (crc >> 1) ^ 0x8408;
        } else {
            crc >>= 1;
        }
        data >>= 1;
    }
    return ~crc;
}


// Function to encode the integer into a 64-bit value
uint64_t encode_integer(uint16_t value, uint16_t index, uint16_t flags) {
    uint64_t encoded = 0;
    encoded |= (static_cast<uint64_t>(value) & 0xFFFF);
    encoded |= ((static_cast<uint64_t>(index) & 0xFFFF) << 16);
    encoded |= ((static_cast<uint64_t>(flags) & 0xFFFF) << 32);
	uint16_t checksum = crc16(value, index);
    encoded |= ((static_cast<uint64_t>(checksum) & 0xFFFF) << 48);
    return encoded;
}


// Function to decode the integer
void decode_integer(uint64_t encoded, uint16_t& value, uint16_t& index, uint16_t& flags, uint16_t& checksum) {
    value = static_cast<uint16_t>(encoded & 0xFFFF);
    index = static_cast<uint16_t>((encoded >> 16) & 0xFFFF);
    flags = static_cast<uint16_t>((encoded >> 32) & 0xFFFF);
    checksum = static_cast<uint16_t>((encoded >> 48) & 0xFFFF);
}


// Function to validate the checksum
bool validate_checksum(uint16_t value, uint16_t index, uint16_t checksum) {
    return crc16(value, index) == checksum;
}

// Function for query
struct QueryCriteria {
    int min_value;
    int max_value;
    int min_index;
    int max_index;
    uint16_t flag_mask; // mask to check for presence of flag bits
    bool checksum_validation; // Flag to enable/disable checksum validation

    QueryCriteria() : min_value(0), max_value(65535), min_index(0), max_index(65535), flag_mask(0), checksum_validation(false) {}
};


// Helper function for checking if an encoded integer matches the criteria.
bool matches_criteria(uint64_t encoded, const QueryCriteria& criteria) {
    uint16_t value, index, flags, checksum;
    decode_integer(encoded, value, index, flags, checksum);

    // Check value range
    if (value < criteria.min_value || value > criteria.max_value) {
        return false;
    }

    // Check index range
    if (index < criteria.min_index || index > criteria.max_index) {
        return false;
    }

    // Check flag presence using bitwise AND with the mask
    if((criteria.flag_mask != 0) && ((flags & criteria.flag_mask) != criteria.flag_mask))
    {
    	return false;
    }


    // Checksum validation (optional)
    if (criteria.checksum_validation && !validate_checksum(value, index, checksum)) {
        return false;
    }

    return true;
}


// Binary search in memory-mapped file to find the first matching entry
int binary_search_memory_mapped(uint64_t* data, size_t num_integers, const QueryCriteria& criteria) {
    int left = 0;
    int right = num_integers - 1;
    int result = -1; // Store index of first matching entry

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (matches_criteria(data[mid], criteria)) {
          result = mid;
          right = mid - 1; // Check left half to find first occurence
        } else if (data[mid] > 0) { // if the mid doesn't match and it's not zero, check if mid might be greater (binary search logic)
          left = mid + 1;
        }
        else
          right = mid - 1;
    }
    return result;
}

int main() {
  	// Dataset generation
  	std::ofstream dataFile("dataset.txt");
    if (!dataFile.is_open()) {
        std::cerr << "Error opening dataset file for writing." << std::endl;
        return 1;
    }
    
    const size_t numIntegers = 10000000;
    
    for (size_t i = 0; i < numIntegers; ++i)
        dataFile << (i % 65536) << std::endl;
    dataFile.close();
    
    // --- Memory mapping and data population ---
  	const size_t file_size = 1ULL * 1024 * 1024 * 1024;
    const char* file_path = "memory_mapped.dat";
    
    int fd = open(file_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Increase the file size to 1GB.
    if (ftruncate(fd, file_size) == -1) {
        perror("Error setting file size");
        close(fd);
        return 1;
    }

    void* mmap_ptr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_ptr == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd);
        return 1;
    }


    uint64_t* mapped_data = reinterpret_cast<uint64_t*>(mmap_ptr);
    
    std::ifstream inputFile("dataset.txt");
    if (!inputFile.is_open()) {
      std::cerr << "Error opening input file." << std::endl;
      return 1;
    }

    std::string line;
    size_t integer_index = 0;
    
    while(std::getline(inputFile, line) && integer_index < numIntegers)
    {
        uint16_t value = static_cast<uint16_t>(std::stoi(line));
        uint16_t index = static_cast<uint16_t>(integer_index);
        uint16_t flags = (index % 2 == 0) ? 0x0008 : 0x0000; // Flag 3 set for even index, 0 otherwise
        mapped_data[integer_index] = encode_integer(value, index, flags);
        integer_index++;
    }
    inputFile.close();


    // --- Query Optimization (Example) ---
    QueryCriteria query;
    query.min_value = 100;
    query.max_value = 200;
    query.min_index = 5000;
    query.max_index = 10000;
    query.flag_mask = 0x0008; // Check for flag 3 being set
    query.checksum_validation = true;

    // --- Perform Query and time it ---
    auto start = std::chrono::high_resolution_clock::now();
    
    int firstMatch = binary_search_memory_mapped(mapped_data, numIntegers, query);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    if (firstMatch != -1)
    {
        std::cout << "First matching entry found at index: " << firstMatch << std::endl;
        uint16_t value, index, flags, checksum;
        decode_integer(mapped_data[firstMatch], value, index, flags, checksum);

      	std::cout << "Value: " << value << ", Index: " << index << ", Flags: " << flags << ", Checksum: " << checksum << std::endl;
    } else {
        std::cout << "No matching entry found" << std::endl;
    }

    std::cout << "Query took: " << duration.count() << " seconds" << std::endl;


    // Cleanup
    if (munmap(mmap_ptr, file_size) == -1) {
        perror("Error unmapping memory");
    }
    close(fd);

    return 0;
}