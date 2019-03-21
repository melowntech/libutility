/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
  Imported to utility library from websocketpp by
    Vaclav Blazek <vaclav.blazek@citationtech.net>
  Changes:
    * moved to namespce utility::base64
    * removed base64_ prefix from function names

    ******
    base64.hpp is a repackaging of the base64.cpp and base64.h files into a
    single headersuitable for use as a header only library. This conversion was
    done by Peter Thorson (webmaster@zaphoyd.com) in 2012. All modifications to
    the code are redistributed under the same license as the original, which is
    listed below.
    ******

   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#ifndef shared_utility_base64_hpp_included_
#define shared_utility_base64_hpp_included_

#include <string>

namespace utility { namespace base64 { namespace detail {

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

inline bool is_base64(unsigned char c) {
    return (c == 43 || // +
           (c >= 47 && c <= 57) || // /-9
           (c >= 65 && c <= 90) || // A-Z
           (c >= 97 && c <= 122)); // a-z
}

class Emittor
{
public:
    Emittor(unsigned int wrap)
        : wrap_(wrap), lineSize_(0)
    {}

    inline void emit(unsigned char c) {
        ret_.push_back(base64_chars[c]);

        ++lineSize_;
        if (wrap_ && (lineSize_ >= wrap_)) {
            ret_.push_back('\n');
            lineSize_ = 0;
        }
    }

    inline void end(unsigned int count) {
        while (count--) {
            ret_.push_back('=');
            ++lineSize_;
            if (wrap_ && (lineSize_ >= wrap_)) {
                ret_.push_back('\n');
                lineSize_ = 0;
            }
        }
    }

    const std::string& get() const { return ret_; }

private:
    std::string ret_;
    unsigned int wrap_;
    unsigned int lineSize_;
};

} // namespace detail

inline std::string encode(unsigned char const* bytes_to_encode
                          , unsigned int in_len
                          , unsigned int wrap = 0)
{
    detail::Emittor ret(wrap);

    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                              ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                              ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i <4) ; i++) {
                ret.emit(char_array_4[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                          ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                          ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret.emit(char_array_4[j]);
        }

        ret.end(3 - i);
    }

    return ret.get();
}

inline std::string encode(const std::string &data, unsigned int wrap = 0)
{
    return encode(reinterpret_cast<const unsigned char *>
                  (data.data()), data.size(), wrap);
}

inline std::string decode(std::string::const_iterator it
                          , const std::string::const_iterator &end)
{
    int i = 0;
    int j = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    for (; it != end; ++it) {
        auto c(*it);
        if (c == '=') { break; }
        if ((c == '\n') || (c == '\r')) { continue; }

        if (!detail::is_base64(c)) {
            // fail on invalid char
            break;
        }
        char_array_4[i++] = c;
        if (i ==4) {
            for (i = 0; i <4; i++) {
                char_array_4[i] = static_cast<unsigned char>
                    (detail::base64_chars.find(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j <4; j++) {
            char_array_4[j] = static_cast<unsigned char>
                (detail::base64_chars.find(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) {
            ret += static_cast<std::string::value_type>(char_array_3[j]);
        }
    }

    return ret;
}

inline std::string decode(std::string const& encoded_string) {
    return decode(encoded_string.begin(), encoded_string.end());
}

} } // namespace utility::base64

#endif // shared_utility_base64_hpp_included_
