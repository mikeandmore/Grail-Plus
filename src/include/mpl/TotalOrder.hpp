/*
 * TotalOrder.hpp
 *
 *  Created on: Sep 14, 2010
 *      Author: Peter Goodman
 *     Version: $Id$
 */

#ifndef CFTL_MPL_TOTALORDER_HPP_
#define CFTL_MPL_TOTALORDER_HPP_

#include "src/include/trait/OperatorTag.hpp";

namespace cftl { namespace mpl {

    /// Less-than-or-equal operator for some type. Defines a total ordering
    /// for elements of that type.
    template <typename T>
    class TotalOrder {
    public:

        typedef trait::BinaryOperatorTag operator_tag_t;

        bool operator()(const T &left, const T &right) const {
            return left <= right;
        }
    };

    template <typename T>
    class TotalOrder<T &> {
    public:

        typedef trait::BinaryOperatorTag operator_tag_t;

        bool operator()(const T &left, const T &right) const {
            return left <= right;
        }
    };
}}

#endif /* CFTL_MPL_TOTALORDER_HPP_ */