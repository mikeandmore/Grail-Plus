/*
 * OpaqueTransition.hpp
 *
 *  Created on: Feb 26, 2011
 *      Author: Peter Goodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 */

#ifndef FLTL_OPAQUETRANSITION_HPP_
#define FLTL_OPAQUETRANSITION_HPP_

namespace fltl { namespace pda {

    /// represents a transition of the PDA
    template <typename AlphaT>
    class OpaqueTransition {
    protected:

        friend class PDA<AlphaT>;
        friend class TransitionGenerator<AlphaT>;

        template <
            typename, typename,
            typename, typename,
            typename, typename
        > friend class PatternGenerator;

        friend class Pattern<AlphaT>;

        template <typename, typename>
        friend class detail::ResetPatternGenerator;

        template <typename, typename, typename>
        friend class detail::FindNextTransition;

        typedef OpaqueTransition<AlphaT> self_type;
        typedef OpaqueState<AlphaT> state_type;
        typedef Symbol<AlphaT> symbol_type;

        Transition<AlphaT> *transition;

        explicit OpaqueTransition(Transition<AlphaT> *trans) throw()
            : transition(trans)
        {
            if(0 != trans) {
                Transition<AlphaT>::hold(trans);
            }
        }

        void assign(Transition<AlphaT> *trans) throw() {

            if(transition != trans) {
                if(0 != transition) {
                    Transition<AlphaT>::release(transition);
                }

                transition = trans;

                if(0 != transition) {
                    Transition<AlphaT>::hold(transition);
                }
            }
        }

    public:

        typedef transition_tag tag_type;

        OpaqueTransition(void) throw()
            : transition(0)
        { }

        OpaqueTransition(const self_type &that) throw()
            : transition(that.transition)
        {
            if(0 != transition) {
                Transition<AlphaT>::hold(transition);
            }
        }

        ~OpaqueTransition(void) throw() {
            if(0 != transition) {
                Transition<AlphaT>::release(transition);
                transition = 0;
            }
        }

        self_type &operator=(const self_type &that) throw() {
            assign(that.transition);
            return *this;
        }

        const state_type &source(void) const throw() {
            assert(0 != transition);
            return transition->source_state;
        }

        const state_type &sink(void) const throw() {
            assert(0 != transition);
            return transition->sink_state;
        }

        const symbol_type &read(void) const throw() {
            assert(0 != transition);
            return transition->sym_read;
        }

        const symbol_type &pop(void) const throw() {
            assert(0 != transition);
            return transition->sym_pop;
        }

        const symbol_type &push(void) const throw() {
            assert(0 != transition);
            return transition->sym_push;
        }

        /// note: not const!
        Unbound<AlphaT,transition_tag> operator~(void) throw() {
            return Unbound<AlphaT,transition_tag>(this);
        }
    };

}}

#endif /* FLTL_OPAQUETRANSITION_HPP_ */