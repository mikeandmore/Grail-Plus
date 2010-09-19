/*
 * SizeOf.hpp
 *
 *  Created on: Sep 15, 2010
 *      Author: Peter Goodman
 *     Version: $Id$
 */

#ifndef CFTL_MPL_SIZEOF_HPP_
#define CFTL_MPL_SIZEOF_HPP_

namespace cftl { namespace mpl {

    /// Get the byte size for a particular type. this allows us to say that
    /// specific types have zero size, which happens to be useful for other
    /// mpl classes.
    template <typename T>
    class SizeOf {
    public:
        enum {
            VALUE = sizeof(T)
        };
    };

    template <typename T>
    class SizeOf<const T> {
    public:
        enum {
            VALUE = SizeOf<T>::VALUE
        };
    };

    template <>
    class SizeOf<void> {
    public:
        enum {
            VALUE = 0
        };
    };
}}

#endif /* CFTL_MPL_SIZEOF_HPP_ */