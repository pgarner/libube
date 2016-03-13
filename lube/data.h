/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, March 2016
 */

#ifndef DATA_H
#define DATA_H

namespace libube
{
    /**
     * Generic data descriptor
     *
     * The descriptor has four entries corresponding to the four fields of a
     * vCard or jCard.  They also work for XML, where TYPE is not needed.
     */
    enum {
        NAME,
        ATTR,
        TYPE,
        DATA
    };
}

#endif // DATA_H
