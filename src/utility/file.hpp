#include <st/assert_or_throw.hpp>

/** The class represents a file

    Mode:
    r   open and read
    r+  open, read and write
    w   create and write
    w+  create, read and write
    a   open and append to end
    a+  open, read and append to end
*/
class File
{
public:
    File(std::string filename, const char *mode)
    {
        if (filename.at(0) == '~') {
            if (auto home = std::getenv("HOME")) {
                filename.replace(0, 1, home);
            }
        }
        _fp = std::fopen(filename.c_str(), mode);
        st::assert_or_throw(_fp, "Can not open file {}", filename);
    }

    constexpr ~File()
    {
        close();
    }

    constexpr File(File &&other) :
        _fp{other._fp}
    {
        other._fp = nullptr;
    }

    constexpr File(const File &other) = delete;

    constexpr File &operator=(File &&other)
    {
        _fp = other._fp;
        other._fp = nullptr;
        return *this;
    }

    constexpr File &operator=(const File &other) = delete;

    constexpr operator std::FILE *() const
    {
        return _fp;
    }

    constexpr void close()
    {
        if (_fp) {
            std::fclose(_fp);
            _fp = nullptr;
        }
    }

private:
    std::FILE *_fp{};
};
