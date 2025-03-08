#pragma once
#include <iostream>
#include <cstdio>
#include <ctype.h>
#define CHUNK_SIZE 4096

using namespace std;

class InParser
{
private:
    FILE* fin;
    unique_ptr<char[]> buffer;
    size_t current_pos, chars_in_buffer;

    inline char read_char()
    {
        ++current_pos;
        if (current_pos == CHUNK_SIZE)
        {
            current_pos = 0;
            chars_in_buffer = fread(buffer.get(), 1, CHUNK_SIZE, fin);
        }
        return buffer[current_pos];
    }

public:
    InParser(const string &filename): buffer{make_unique<char[]>(CHUNK_SIZE)}, current_pos(CHUNK_SIZE - 1), chars_in_buffer(0), fin(fopen(filename.c_str(), "r"))
    {
    }

    inline InParser& operator >> (unsigned int& nr)
    {
        char c;
        while (!isdigit(c = read_char()) && current_pos != chars_in_buffer);
        nr = c - '0';
        while (isdigit(c = read_char()) && current_pos != chars_in_buffer)
            nr = 10 * nr + c - '0';
        return *this;
    }

    inline InParser& operator >> (char& chr)
    {
        chr = read_char();
        return *this;
    }
};