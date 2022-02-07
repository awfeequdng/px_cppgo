#include "syntax/source.hh"

#include "common/types.hh"
#include "common/utf8/rune.hh"
#include "common/hex_formatter.hh"

#include <vector>
#include <string>
#include <iostream>
#include <tuple>
#include <fstream>

#define panic(a)    { std::cout << (a) << std::endl; exit(0); } while(0)

namespace syntax
{
// #include "io"
// #include "unicode/utf8"

// characters below RuneSelf are represented as themselves in a single byte
#define RuneSelf 0x80
#define sentinel RuneSelf

#define linebase 1
#define colbase 1

int64_t nextSize(int64_t size) {
    const static int min = 4 << 10;
    const static int max = 1 << 20;
    if (size < min) {
        return min;
    }
    if (size <= max) {
        return size << 1;
    }
    return size + max;
}


    void source::init(std::string file, err_handler errh)  {
        _ifs.open(file);
        if (!_ifs.good()) {
            std::cout << "open file: " << file << " failed" << std::endl;
            exit(0);
        }
        _errh = errh;
        _buf.resize(nextSize(0), 0);
        _buf[0] = sentinel;
        _b = -1;
        _r = 0;
        _e = 0;
        _line = 0;
        _col = 0;
        _ch = ' ';
        _chw = 0;
        std::cout << "init buf: " << std::endl
            << common::HexFormatter::dump_to_string(_buf.data(), 10)
            << std::endl;
    }
    std::pair<int, int> source::pos() {
        return {linebase + _line, colbase + _col};
    }

    void source::error(std::string msg) {
        auto [line, col] = pos();
        if (_errh) {
            _errh(line, col, msg);
        }
        std::cout << msg << std::endl;
        exit(0);
    }

    void source::start() { _b = _r - _chw; }
    void source::stop() { _b = -1; }

    std::string source::segment() { return _buf.substr(_b, _r - _chw - _b); }

    void source::rewind() {
        if (_b < 0) {
            panic("no active segment");
        }
        _col -= uint(_r - _b);
        _r = _b;
        nextch();
    }

    void source::nextch() {
    redo:
        _col += uint(_chw);
        if (_ch == '\n') {
            _line++;
            _col = 0;
        }

        {
            _ch = common::utf8::rune_t((unsigned char)_buf[_r]);
            if (_ch < sentinel) {
                _r++;
                _chw = 1;
                if (_ch == '\0') {
                    error("invalid NUL character");
                    goto redo;
                }
                return;
            }
        }
// maximum number of bytes of a UTF-8 encoded Unicode character.
#define UTFMax 4
// FullRune reports whether the bytes in p begin with a full UTF-8 encoding of a rune.
// An invalid encoding is considered a full Rune since it will convert as a width-1 error rune.
        auto fr = _buf.substr(_r, _e - _r);
        while((_e - _r < UTFMax) &&
              !common::utf8::full_rune(fr) &&
              _ifs.good()) {
            fill();
        }
        if (_r == _e) {
            if (!_ifs.eof()) {
                error("I/O error: ");
            }
            _ch = common::utf8::rune_eof;
            _chw = 0;
            return;
        }
        auto dr = _buf.substr(_r, _e - _r);
        std::tie(_ch, _chw) = common::utf8::decode_rune(dr);
        _r += _chw;
        if (_ch == common::utf8::rune_invalid && _chw == 1) {
            error("invalid UTF-8 encoding");
            goto redo;
        }
    // #define BOM 0xfeff
        if (_ch == common::utf8::rune_bom) {
            if (_line > 0 || _col > 0) {
                error("invalid BOM in the middle of the file");
            }
            goto redo;
        }
    }

    void source::fill() {
        auto bb = _r;
        if (_b >= 0) {
            bb = _b;
            _b = 0;
        }

        auto content = _buf.substr(bb, _e - bb);
        if (content.size() * 2 > _buf.size()) {
            _buf.resize(nextSize(_buf.size()));
            std::copy(content.begin(), content.end(), _buf.begin());
        } else if (_b > 0) {
            std::copy(content.begin(), content.end(), _buf.begin());
        }
        _r -= bb;
        _e -= bb;
        {
            auto i = 0;
            for (; i < 10; i++) {
                 // -1 to leave space for sentinel
                _ifs.read(&_buf.front() + _e, _buf.size() - 1 - _e);
                int n = _ifs.gcount();

                if (n < 0) {
                    panic("negative read");
                }
                if (n > 0 || !_ifs.eof()) {
                    _e += n;
                    _buf[_e] = sentinel;
                    return;
                }
            }
        }
        _buf[_e] = sentinel;
    }

} // namespace syntax
