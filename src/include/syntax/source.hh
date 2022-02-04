#pragma once
#include "common/types.hh"
#include "common/utf8/rune.hh"
#include "common/hex_formatter.hh"

#include <vector>
#include <string>
#include <iostream>
#include <tuple>
#include <fstream>

template <typename T>
std::vector<T> slice(std::vector<T>& v, std::size_t low, std::size_t high = -1) {
    std::vector<T> vec;
    if (high == -1) {
        std::copy(v.begin() + low, v.end(), std::back_inserter(vec));
    } else {
        std::copy(v.begin() + low, v.begin() + high, std::back_inserter(vec));
    }
    return vec;
}

namespace syntax
{

int64_t nextSize(int64_t size);

typedef void (*err_handler)(uint line, uint col, std::string msg);

// The source buffer is accessed using three indices b (begin),
// r (read), and e (end):
//
// - If b >= 0, it points to the beginning of a segment of most
//   recently read characters (typically a Go literal).
//
// - r points to the byte immediately following the most recently
//   read character ch, which starts at r-chw.
//
// - e points to the byte immediately following the last byte that
//   was read into the buffer.
//
// The buffer content is terminated at buf[e] with the sentinel
// character utf8.RuneSelf. This makes it possible to test for
// the common case of ASCII characters with a single 'if' (see
// nextch method).
//
//                +------ content in use -------+
//                v                             v
// buf [...read...|...segment...|ch|...unread...|s|...free...]
//                ^             ^  ^            ^
//                |             |  |            |
//                b         r-chw  r            e
//
// Invariant: -1 <= b < r <= e < len(buf) && buf[e] == sentinel
struct source {
    std::ifstream _ifs;
    err_handler _errh{};
    std::string _buf;
    int64_t _b;
    int64_t _r;
    int64_t _e;
    int _line;
    int _col;
    common::utf8::rune_t _ch;
    int _chw;

    void init(std::string file, err_handler errh);
    std::pair<int, int> pos();

    void error(std::string msg);

    void start();
    void stop();

    // std::string segment() { return slice(buf, b, r - chw); }
    std::string segment();

    void rewind();

    void nextch();

    void fill();
};

} // namespace syntax
