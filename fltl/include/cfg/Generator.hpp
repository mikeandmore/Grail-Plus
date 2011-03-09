/*
 * Generator.hpp
 *
 *  Created on: Jan 27, 2011
 *      Author: Peter Goodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 */

#ifndef FLTL_GENERATOR_HPP_
#define FLTL_GENERATOR_HPP_

namespace fltl { namespace cfg {

    namespace detail {

        template <typename AlphaT>
        bool default_gen_next(Generator<AlphaT> *) throw() {
            return false;
        }

        template <typename AlphaT>
        void default_gen_reset(Generator<AlphaT> *) throw() { }

        template <typename AlphaT>
        void default_gen_free(Generator<AlphaT> *) throw() { }

        template <typename AlphaT>
        class SimpleGenerator {
        public:

            static void
            reset_next_production(Generator<AlphaT> *state) throw() {
                state->free_func(state);
                state->cursor.production = state->cfg->first_production;
                if(0 != state->cursor.production) {
                    Production<AlphaT>::hold(state->cursor.production);
                }
            }

            static Production<AlphaT> *
            find_next_production(Production<AlphaT> *prod) throw() {

                if(0 == prod) {
                    return 0;
                }

                // go look for the next production
                Production<AlphaT> *next_prod(prod->next);
                Variable<AlphaT> *curr_var(prod->var);

                for(;;) {
                    if(0 != next_prod && !next_prod->is_deleted) {
                        goto found_next_prod;
                    }
                    curr_var = curr_var->next;

                    if(0 == curr_var) {
                        next_prod = 0;
                        goto found_next_prod;
                    }

                    next_prod = curr_var->first_production;
                }

            found_next_prod:
                return next_prod;
            }

            /// generate productions
            static bool
            bind_next_production(Generator<AlphaT> *state) throw() {

                // get the binder
                OpaqueProduction<AlphaT> *binder(
                    helper::unsafe_cast<OpaqueProduction<AlphaT> *>(
                        state->binder
                    )
                );

                // update or clear the binding
                Production<AlphaT> *curr_prod(state->cursor.production);
                Production<AlphaT> *prev(0);
                OpaqueProduction<AlphaT> opaque_prod;

                // are we looking at the right production? we might need
                // to move forward if the production that was meant to be
                // the cursor was deleted
                if(0 == curr_prod) {
                    *binder = opaque_prod;
                    return false;
                } else {

                    while(curr_prod->is_deleted) {

                        prev = curr_prod;
                        curr_prod = find_next_production(prev);
                        Production<AlphaT>::release(prev);

                        if(0 != curr_prod) {
                            Production<AlphaT>::hold(curr_prod);
                        } else {
                            *binder = opaque_prod;
                            state->cursor.production = 0;
                            return false;
                        }
                    }
                }

                opaque_prod.assign(curr_prod);
                Production<AlphaT>::release(curr_prod);
                *binder = opaque_prod;

                state->cursor.production = find_next_production(curr_prod);

                if(0 != state->cursor.production) {
                    Production<AlphaT>::hold(state->cursor.production);
                }

                return true;
            }

            static void free_next_production(Generator<AlphaT> *state) throw() {
                if(0 != state->cursor.production) {
                    Production<AlphaT>::release(state->cursor.production);
                    state->cursor.production = 0;
                }
            }

            /// reset the variable generator
            static void reset_next_variable(Generator<AlphaT> *state) throw() {
                Production<AlphaT> *first_prod(state->cfg->first_production);
                if(0 == first_prod) {
                    state->cursor.variable_offset = 1;
                } else {
                    state->cursor.variable_offset = static_cast<unsigned>(
                        first_prod->var->id
                    );
                }
            }

            /// generate variables
            static bool bind_next_variable(Generator<AlphaT> *state) throw() {

                Symbol<AlphaT> *binder(
                    helper::unsafe_cast<Symbol<AlphaT> *>(state->binder)
                );

                unsigned offset(state->cursor.variable_offset);
                Variable<AlphaT> *var(0);

            check_var_offset:

                if(offset >= state->cfg->variable_map.size()) {
                    binder->value = 0;
                    state->cursor.variable_offset = offset;
                    return false;
                }

                var = state->cfg->variable_map.get(offset);

                // bad variable or variable is deleted
                if(0 == var || 0 == var->first_production) {
                    ++offset;
                    goto check_var_offset;
                }

                // bind the variable and point the cursor at the next
                // variable
                binder->value = var->id;
                state->cursor.variable_offset = offset + 1;
                return true;
            }

            /// reset the terminal generator
            static void reset_next_terminal(Generator<AlphaT> *state) throw() {
                state->cursor.terminal_offset = 1U;
            }

            /// generate terminals
            static bool bind_next_terminal(Generator<AlphaT> *state) throw() {
                Symbol<AlphaT> *binder(
                    helper::unsafe_cast<Symbol<AlphaT> *>(state->binder)
                );

                unsigned &offset(state->cursor.terminal_offset);

                ++offset;

                if(offset > state->cfg->terminal_map.size()) {
                    binder->value = 0;
                    return false;
                }

                // bind the variable
                binder->value = -1 * static_cast<internal_sym_type>(
                    offset - 1
                );

                return true;
            }
        };

        /// template for complex patterns
        template <typename AlphaT, typename PatternBuilderT>
        class PatternGenerator {
        public:

            static bool bind_next_pattern(Generator<AlphaT> *state) throw() {

                Production<AlphaT> *curr_prod(state->cursor.production);
                Production<AlphaT> *next_prod(0);
                Production<AlphaT> *prev(0);

                if(0 == curr_prod) {
                    return false;
                } else {

                    while(curr_prod->is_deleted) {

                        prev = curr_prod;
                        curr_prod = SimpleGenerator<AlphaT>::find_next_production(
                            prev
                        );

                        Production<AlphaT>::release(prev);

                        if(0 != curr_prod) {
                            Production<AlphaT>::hold(curr_prod);
                        } else {
                            state->cursor.production = 0;
                            return false;
                        }
                    }
                }

                OpaqueProduction<AlphaT> opaque_prod;

                do {
                next_iteration:
                    opaque_prod.assign(curr_prod);
                    Production<AlphaT>::release(curr_prod);

                    // go find the next production to bind
                    next_prod = SimpleGenerator<AlphaT>::find_next_production(
                        curr_prod
                    );

                    // match and bind the pattern
                    if(PatternBuilderT::static_match(state->pattern, opaque_prod)) {

                        // we can't go futher than here
                        if(1 == PatternBuilderT::IS_BOUND_TO_VAR
                        && (0 == curr_prod->next || curr_prod->next->is_deleted)) {
                            next_prod = 0;
                        }

                        break;

                    } else {

                        opaque_prod.assign(0);

                        // we're at the end of some variable's list of
                        // productions
                        if(1 == PatternBuilderT::IS_BOUND_TO_VAR
                        && (0 == curr_prod->next || curr_prod->next->is_deleted)) {
                            curr_prod = 0;
                            next_prod = 0;
                            break;

                        // go try the next one
                        } else {
                            curr_prod = next_prod;
                            next_prod = 0;

                            if(0 != curr_prod) {
                                Production<AlphaT>::hold(curr_prod);
                                goto next_iteration;
                            }
                        }
                    }

                } while(0 != curr_prod);

                state->cursor.production = next_prod;

                if(0 == curr_prod) {
                    return false;
                }

                if(0 != next_prod) {
                    Production<AlphaT>::hold(next_prod);
                }

                // bind the production
                if(0 != state->binder) {

                    OpaqueProduction<AlphaT> *binder(
                        helper::unsafe_cast<OpaqueProduction<AlphaT> *>(
                            state->binder
                        )
                    );

                    *binder = opaque_prod;
                }

                return true;
            }

            static void reset_next_pattern(Generator<AlphaT> *state) throw() {
                state->free_func(state);
                if(1 == PatternBuilderT::IS_BOUND_TO_VAR) {
                    state->cursor.production = state->cfg->variable_map.get(
                        static_cast<unsigned>(
                            state->pattern->var->value
                        )
                    )->first_production;
                } else {
                    state->cursor.production = state->cfg->first_production;
                }

                if(0 != state->cursor.production) {
                    Production<AlphaT>::hold(state->cursor.production);
                }
            }
        };
    }

    template <typename AlphaT>
    class Generator {
    private:

        friend class CFG<AlphaT>;
        friend class detail::SimpleGenerator<AlphaT>;

        template <typename, typename>
        friend class detail::PatternGenerator;

        typedef Generator<AlphaT> self_type;
        typedef typename CFG<AlphaT>::production_type production_type;

        typedef void (reset_gen_type)(self_type *);
        typedef bool (bind_next_type)(self_type *);
        typedef void (free_func_type)(self_type *);

        /// CFG from which we are generating stuff
        CFG<AlphaT> *cfg;

        union {

            /// the last variable bound by the generator
            unsigned variable_offset;

            /// the last production bound by the generator
            Production<AlphaT> *production;

            /// offset into the terminals into which we're looking
            unsigned terminal_offset;

        } cursor;

        /// pointer to some sort of type to which we are binding results
        void *binder;
        detail::PatternData<AlphaT> *pattern;

        /// the binder function, does the variable binding and tells us if
        /// we can keep going
        bind_next_type *binder_func;

        /// reset the generator
        reset_gen_type *reset_func;

        /// free the generator
        free_func_type *free_func;

        /// have we started?
        bool has_been_used;

        Generator(
            CFG<AlphaT> *_cfg,
            void *_binder,
            detail::PatternData<AlphaT> *_pattern,
            bind_next_type *_binder_func,
            reset_gen_type *_reset_func,
            free_func_type *_free_func
        ) throw()
            : cfg(_cfg)
            , binder(_binder)
            , pattern(_pattern)
            , binder_func(_binder_func)
            , reset_func(_reset_func)
            , free_func(_free_func)
            , has_been_used(false)
        {
            memset(&cursor, 0, sizeof cursor);

            if(0 != pattern) {
                detail::PatternData<AlphaT>::incref(pattern);
            }
        }

    public:

        Generator(void) throw()
            : cfg(0)
            , binder(0)
            , pattern(0)
            , binder_func(&detail::default_gen_next)
            , reset_func(&detail::default_gen_reset)
            , free_func(&detail::default_gen_free)
            , has_been_used(false)
        {
            memset(&cursor, 0, sizeof cursor);
        }

        /// copy constructor for public use
        Generator(const self_type &that) throw()
            : cfg(that.cfg)
            , binder(that.binder)
            , pattern(that.pattern)
            , binder_func(that.binder_func)
            , reset_func(that.reset_func)
            , free_func(that.free_func)
            , has_been_used(false)
        {
            memcpy(&cursor, &(that.cursor), sizeof cursor);

            if(0 != pattern) {
                detail::PatternData<AlphaT>::incref(pattern);
            }
        }

        ~Generator(void) throw() {
            if(0 != pattern) {
                detail::PatternData<AlphaT>::decref(pattern);
            }
            free_func(this);
        }

        self_type &operator=(self_type &that) throw() {
            assert(
                0 == cfg &&
                "Illegal assignment to an initialized generator."
            );

            if(0 != pattern) {
                detail::PatternData<AlphaT>::decref(pattern);
            }
            free_func(this);

            cfg = that.cfg;
            memcpy(&cursor, &(that.cursor), sizeof cursor);
            binder = that.binder;
            pattern = that.pattern;
            binder_func = that.binder_func;
            reset_func = that.reset_func;
            free_func = that.free_func;
            has_been_used = false;

            if(0 != pattern) {
                detail::PatternData<AlphaT>::incref(pattern);
            }

            return *this;
        }

        /// rewind the generator to its initial conditions
        inline void rewind(void) throw() {
            reset_func(this);
        }

        /// try to generate the next object(s) according to the binder
        /// conditions and then bind the binder objects. returns true if
        /// we were able to find a successful binding, false otherwise.
        inline bool match_next(void) throw() {
            if(!has_been_used) {
                has_been_used = true;
                reset_func(this);
            }
            return binder_func(this);
        }
    };
}}

#endif /* FLTL_GENERATOR_HPP_ */