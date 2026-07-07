#pragma once
#include <DataFrame.hpp>
#include <fstream>
#include <sstream>
#include <string_view>
#include <variant>
#include <charconv>
#include <stdexcept>
#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class MmapFile {
    private:
        int fd = -1;
        size_t size = 0;
        const char* data = nullptr;

    public:
        MmapFile(const std::string& filepath) {
            fd = open(filepath.c_str(), O_RDONLY);
            if (fd == -1) throw std::runtime_error("Failed to open file: " + filepath);

            struct stat sb;
            if (fstat(fd, &sb) == -1) {
                close(fd);
                throw std::runtime_error("Failed to get file size");
            }
            size = sb.st_size;

            if (size > 0) {
                data = static_cast<const char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
                if (data == MAP_FAILED) {
                    close(fd);
                    throw std::runtime_error("Failed to mmap file");
                }
            }
        }

        ~MmapFile() {
            if (data && data != MAP_FAILED) munmap(const_cast<char*>(data), size);
            if (fd != -1) close(fd);
        }

        std::string_view get_view() const { return {data, size}; }
        size_t get_size() const { return size; }
};

class Reader{
    private:
        static std::vector<std::string> parse_line(const std::string& line, char delimiter = ',');
        static std::vector<std::string_view> ssplit(std::string_view str, char delim = ',');
        static DataType infer_type(std::string_view ); 
        static DataType promote_type(DataType , DataType );
    public:
        Reader() = default;

        //Readers
        static DataFrame read_csv(const std::string& );
        // static DataFrame read_excel(const std::string& );
        // static DataFrame read_json(const std::string& );
        // static DataFrame read_xml(const std::string& );

        //Writers
        static void to_csv(const DataFrame& ,const std::string& );
};  