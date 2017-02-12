#pragma once
#include "mystring.h"

#include "optional.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DISABLE_COPY_MOVE(cls)                                                                     \
    cls(const cls&) = delete;                                                                      \
    cls(cls&&) = delete;                                                                           \
    cls& operator=(const cls&) = delete;                                                           \
    cls& operator=(cls&&) = delete;

typedef unsigned char byte;

/*-
* Copyright (c) 2012, 2014 Zhihao Yuan.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#ifndef _STDEX_DEFER_H
#define _STDEX_DEFER_H

#include <cryptopp/secblock.h>
#include <utility>

#define DEFER(...)                                                                                 \
    auto STDEX_NAMELNO__(_stdex_defer_, __LINE__) = stdex::make_guard([&] { __VA_ARGS__; });
#define STDEX_NAMELNO__(name, lno) STDEX_CAT__(name, lno)
#define STDEX_CAT__(a, b) a##b

namespace stdex
{

template <typename Func>
struct scope_guard
{
    explicit scope_guard(Func&& on_exit) : on_exit_(std::move(on_exit)) {}

    scope_guard(scope_guard const&) = delete;
    scope_guard& operator=(scope_guard const&) = delete;

    scope_guard(scope_guard&& other) : on_exit_(std::move(other.on_exit_)) {}

    ~scope_guard()
    {
        try
        {
            on_exit_();
        }
        catch (...)
        {
        }
    }

private:
    Func on_exit_;
};

template <typename Func>
scope_guard<Func> make_guard(Func&& f)
{
    return scope_guard<Func>(std::forward<Func>(f));
}
}

#endif

namespace securefs
{
template <class T, size_t N>
constexpr inline size_t array_length(const T (&)[N])
{
    return N;
};

inline constexpr bool is_windows(void)
{
#ifdef WIN32
    return true;
#else
    return false;
#endif
}
using std::experimental::optional;

typedef uint64_t length_type;
typedef uint64_t offset_type;

constexpr uint32_t KEY_LENGTH = 32, ID_LENGTH = 32, BLOCK_SIZE = 4096;

template <class T>
inline std::unique_ptr<T[]> make_unique_array(size_t size)
{
    return std::unique_ptr<T[]>(new T[size]);
}

template <class T>
struct _Unique_if
{
    typedef std::unique_ptr<T> _Single_object;
};

template <class T>
struct _Unique_if<T[]>
{
    typedef std::unique_ptr<T[]> _Unknown_bound;
};

template <class T, size_t N>
struct _Unique_if<T[N]>
{
    typedef void _Known_bound;
};

template <class T, class... Args>
typename _Unique_if<T>::_Single_object make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename _Unique_if<T>::_Unknown_bound make_unique(size_t n)
{
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]());
}

template <class T, class... Args>
typename _Unique_if<T>::_Known_bound make_unique(Args&&...) = delete;

template <class T, size_t Size>
class PODArray
{
private:
    T m_data[Size];

    static_assert(std::is_pod<T>::value, "Only POD types are supported");

public:
    explicit PODArray() { memset(m_data, 0, sizeof(m_data)); }
    explicit PODArray(const T& value) { std::fill(std::begin(m_data), std::end(m_data), value); }
    PODArray(const PODArray& other) { memcpy(m_data, other.m_data, size()); }
    PODArray& operator=(const PODArray& other)
    {
        memmove(m_data, other.m_data, size());
        return *this;
    }
    const T* data() const { return m_data; }
    T* data() { return m_data; }
    static constexpr size_t size() { return Size; };
    bool operator==(const PODArray& other) const
    {
        return memcmp(m_data, other.m_data, size()) == 0;
    }
    bool operator!=(const PODArray& other) const { return !(*this == other); }
};

typedef PODArray<byte, KEY_LENGTH> key_type;
typedef PODArray<byte, ID_LENGTH> id_type;

template <class Iterator, class T>
inline bool is_all_equal(Iterator begin, Iterator end, const T& value)
{
    while (begin != end)
    {
        if (*begin != value)
            return false;
        ++begin;
    }
    return true;
}

inline bool is_all_zeros(const void* data, size_t len)
{
    auto bytes = static_cast<const byte*>(data);
    return is_all_equal(bytes, bytes + len, 0);
}

template <class T>
inline void to_little_endian(T value, void* output) noexcept
{
    typedef typename std::remove_reference<T>::type underlying_type;
    static_assert(std::is_unsigned<underlying_type>::value, "Must be an unsigned integer type");
    auto bytes = static_cast<byte*>(output);
    for (size_t i = 0; i < sizeof(underlying_type); ++i)
    {
        bytes[i] = value >> (8 * i);
    }
}

template <class T>
inline typename std::remove_reference<T>::type from_little_endian(const void* input) noexcept
{
    typedef typename std::remove_reference<T>::type underlying_type;
    static_assert(std::is_unsigned<underlying_type>::value, "Must be an unsigned integer type");
    auto bytes = static_cast<const byte*>(input);
    underlying_type value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        value |= static_cast<underlying_type>(bytes[i]) << (8 * i);
    }
    return value;
}

std::string random_hex_string(size_t size);

void hmac_sha256_calculate(const void* message,
                           size_t msg_len,
                           const void* key,
                           size_t key_len,
                           void* mac,
                           size_t mac_len);

bool hmac_sha256_verify(const void* message,
                        size_t msg_len,
                        const void* key,
                        size_t key_len,
                        const void* mac,
                        size_t mac_len);

// HMAC based key derivation function (https://tools.ietf.org/html/rfc5869)
// This one is not implemented by Crypto++, so we implement it ourselves
void hkdf(const void* key,
          size_t key_len,
          const void* salt,
          size_t salt_len,
          const void* info,
          size_t info_len,
          void* output,
          size_t out_len);

unsigned int pbkdf_hmac_sha256(const void* password,
                               size_t pass_len,
                               const void* salt,
                               size_t salt_len,
                               unsigned int min_iterations,
                               double min_seconds,
                               void* derived,
                               size_t derive_len);

struct id_hash
{
    size_t operator()(const id_type& id) const noexcept
    {
        size_t value;
        memcpy(&value, id.data() + (id.size() - sizeof(size_t)), sizeof(size_t));
        return value;
    }
};

std::unordered_set<id_type, id_hash> find_all_ids(const std::string& basedir);

std::string get_user_input_until_enter();

void respond_to_user_action(
    const std::unordered_map<std::string, std::function<void(void)>>& actionMap);
}
