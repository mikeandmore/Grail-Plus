/*
 * NFA_TO_DOT.hpp
 *
 *  Created on: Jan 29, 2012
 *      Author: petergoodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef Grail_Plus_NFA_TO_DOT_HPP_
#define Grail_Plus_NFA_TO_DOT_HPP_


#include <cstdio>

#include "fltl/include/NFA.hpp"

#include "grail/include/io/CommandLineOptions.hpp"
#include "grail/include/io/fread_nfa.hpp"

namespace grail { namespace cli {

    template <typename AlphaT>
    class NFA_TO_DOT {
    public:

        FLTL_NFA_USE_TYPES(fltl::NFA<AlphaT>);

        static const char * const TOOL_NAME;

        static void declare(io::CommandLineOptions &opt, bool in_help) throw() {
            io::option_type in(opt.declare("stdin", io::opt::OPTIONAL, io::opt::NO_VAL));
            if(!in_help) {
                if(in.is_valid()) {
                    opt.declare_max_num_positional(0);
                } else {
                    opt.declare_min_num_positional(1);
                    opt.declare_max_num_positional(1);
                }
            }
        }

        static void help(void) throw() {
            //  "  | |                              |                                             |"
            printf(
                "  %s:\n"
                "    Converts a non-deterministic finite automaton (NFA) into a DOT digraph\n"
                "    describing the structure of the graph representation of the automaton.\n\n"
                "  basic use options for %s:\n"
                "    --stdin                        Read a PDA from stdin. Typing a new\n"
                "                                   line followed by Ctrl-D or Ctrl-Z will\n"
                "                                   close stdin.\n"
                "    <file>                         read in an NFA from <file>.\n\n",
                TOOL_NAME, TOOL_NAME
            );
        }

        static void print(FILE *outfile, nfa_type &nfa) throw() {

            char name_buff[1024] = {'\0'};

            state_type from, to;
            symbol_type condition;
            generator_type transitions(nfa.search(~from, ~condition, ~to));
            generator_type states(nfa.search(~from));

            fprintf(outfile, "digraph {\n");

            // add in a pseudo start state to make the entry arrow into the
            // NFA
            fprintf(outfile,
                "    S [label=\"\" color=\"white\"]\n"
                "    S -> %u\n",
                nfa.get_start_state().number()
            );

            // print out the accept states
            for(; states.match_next(); ) {

                // how should we label the state?
                const char *state_name(nfa.get_name(from));
                if('\0' == state_name[0]) {
                    sprintf(name_buff, "%u", from.number());
                } else {
                    sprintf(name_buff, "%s", state_name);
                }

                if(nfa.is_accept_state(from)) {
                    fprintf(outfile,
                        "    %u [shape=doublecircle label=\"%s\"]\n",
                        from.number(),
                        name_buff
                    );
                } else {
                    fprintf(outfile,
                        "    %u [label=\"%s\"]\n",
                        from.number(),
                        name_buff
                    );
                }
            }

            // print out the transitions
            for(; transitions.match_next(); ) {
                const char *cond_str("&epsilon;");
                if(nfa.epsilon() != condition) {
                    traits_type::unserialize(
                        nfa.get_alpha(condition),
                        cond_str
                    );
                }


                fprintf(outfile, "    %u -> %u [label=<%s>]\n",
                    from.number(),
                    to.number(),
                    cond_str
                );
            }

            fprintf(outfile, "}\n");
        }

        static int main(io::CommandLineOptions &options) throw() {

            // run the tool
            io::option_type file;
            const char *file_name(0);

            FILE *fp(0);

            // take input from stdin or from a file?
            if(options["stdin"].is_valid()) {
                file = options["stdin"];
                fp = stdin;
                file_name = "<stdin>";
            } else {
                file = options[0U];
                file_name = file.value();
                fp = fopen(file_name, "r");
            }

            // couldn't open the file
            if(0 == fp) {
                options.error(
                    "Unable to open file containing non-deterministic finite "
                    "automaton for reading."
                );
                options.note("File specified here:", file);
                return 1;
            }

            nfa_type nfa;
            int ret(0);
            FILE *outfile(stdout);

            if(io::fread(fp, nfa, file_name)) {

                print(outfile, nfa);

            } else {
                ret = 1;
            }

            fclose(fp);

            return ret;
        }
    };

    template <typename AlphaT>
    const char * const NFA_TO_DOT<AlphaT>::TOOL_NAME("nfa-to-dot");
}}

#endif /* Grail_Plus_NFA_TO_DOT_HPP_ */
