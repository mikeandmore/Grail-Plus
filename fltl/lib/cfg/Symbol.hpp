/*
 * Symbol.hpp
 *
 *  Created on: Jan 20, 2011
 *      Author: Peter Goodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 */

#ifndef FLTL_SYMBOL_HPP_
#define FLTL_SYMBOL_HPP_

namespace fltl { namespace lib { namespace cfg {

    /// grammar symbol (non-terminal or terminal)
    template <typename AlphaT>
    class Symbol {
    protected:

        internal_sym_type value;

        friend class CFG<AlphaT>;
        friend class Variable<AlphaT>;
        friend class Production<AlphaT>;
        friend class OpaqueProduction<AlphaT>;
        friend class ProductionBuilder<AlphaT>;
        friend class SymbolString<AlphaT>;

        typedef SymbolString<AlphaT> symbol_string_type;

        explicit Symbol(const cfg::internal_sym_type v) throw()
            : value(v)
        { }

        /// adapted from fmix32, from Murmurhash3, by Austin Appleby.
        // this function is adapted from code that is licensed under the
        // MIT license
        inline internal_sym_type randomize(void) const throw() {
            uint32_t h(static_cast<uint32_t>(value));
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return static_cast<internal_sym_type>(h);
        }

    public:

        typedef Symbol<AlphaT> self_type;

        Symbol(void) throw()
            : value(0)
        { }

        Symbol(const self_type &that) throw()
            : value(that.value)
        { }

        inline self_type &operator=(const self_type &that) throw() {
            value = that.value;
            return *this;
        }

        inline bool operator==(const self_type &that) const throw() {
            return value == that.value;
        }

        inline bool operator!=(const self_type &that) const throw() {
            return value != that.value;
        }

        inline unsigned length(void) const throw() {
            return (0 == value) ? 0U : 1U;
        }

        symbol_string_type operator+(symbol_string_type &that) const throw() {
            return that.prepend_symbol(this);
        }

        symbol_string_type operator+(const self_type &other) const throw() {
            symbol_string_type ret;
            const unsigned this_len = length();
            const unsigned that_len = other.length();
            const unsigned total_len = this_len + that_len;

            if(0 == total_len) {
                return ret;
            }

            ret.symbols = symbol_string_type::allocate(total_len);
            ret.symbols[symbol_string_type::FIRST_SYMBOL].value = value;
            ret.symbols[
                symbol_string_type::FIRST_SYMBOL + this_len
            ].value = other.value;
            return ret;
        }
    };

}}}

#endif /* FLTL_SYMBOL_HPP_ */