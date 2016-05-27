/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2016
 */

/*
 * UTF is ISO/IEC 10646
 * UTF-8 is described in RFC3629, https://tools.ietf.org/html/rfc3629
 *
 * I'm really not sure to qhat extent it's necessary to explicity deal with
 * UTF-8 in a library.  It doesn't seem to be that onerous.
 *
 * Some hex to binary mappings
 *   0xF 1111
 *   0xE 1110
 *   0xD 1101
 *   0xC 1100
 *   0xB 1011
 *   0xA 1010
 *   0x8 1000
 *   0x9 1001
 *
 * Some hex masks that follow from the above
 * [ 87654321 - Hex  - UTF8 Meaning ]
 *   0xxxxxxx - 0x00 - ASCII, one byte
 *   10xxxxxx - 0x80 - Byte 2, 3 or 4 of a UTF8 sequence
 *   110xxxxx - 0xC0 - Byte 1 of two byte sequence
 *   1110xxxx - 0xE0 - Byte 1 of three byte sequence
 *   11110xxx - 0xF0 - Byte 1 of four byte sequence
 * 
 */

#include "ind.h"
#include "var.h"

using namespace libube;
using namespace std;

/**
 * Returns the number of bytes constituting the valid UTF-8 code-point pointed
 * to by iStr, or 0 if it's not UTF-8.
 */
int utf8len(const char* iStr)
{
    // Just one byte; the most likely case
    if ((iStr[0] & 0x80) == 0x00) // 0xxxxxxx
        return 1;

    // Otherwise there are 3 other length cases
    int len = 0;
    if ((iStr[0] & 0xE0) == 0xC0) // 110xxxxx
        len = 2;
    else if ((iStr[0] & 0xF0) == 0xE0) // 1110xxxx
        len = 3;
    else if ((iStr[0] & 0xF8) == 0xF0) // 11110xxx
        len = 4;
    if (len == 0)
        return 0;

    // The subsequent bytes have a given form
    for (int i=1; i<len; i++)
        if ((iStr[i] & 0xC0) != 0x80) // 10xxxxxx
            return 0;

    // It's UTF-8
    return len;
}

/**
 * Return the length of a string in code-points
 */
ind var::len()
{
    if (!atype<char>())
        return size();
    int clen = 0;
    int code = 0;
    char* s = ptr<char>();
    while (clen < size())
    {
        int l = utf8len(s+clen);
        if (l == 0)
            return -1;
        clen += l;
        code += 1;
    }
    if (clen != size())
        return -1;
    return code;
}
