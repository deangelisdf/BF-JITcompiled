/*
 * Copyright (C) 2018 deangelis domenico francesco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   brainfuck.h
 * Author: deangelis domenico francesco
 *
 * Created on 27 settembre 2018, 12.57
 */

#ifndef BRAINFUCK_H
#define BRAINFUCK_H

#include <cstdlib>
#include <cstdio>
using namespace std;

#define MAX_DIM_BUFFER 1024

struct linked_list_token{
    char op;
    linked_list_token* next;
    virtual ~linked_list_token(){}
    linked_list_token():op(0),next(NULL){}
};

struct ld_token_jmp : linked_list_token{
    linked_list_token* jmp;
    virtual ~ld_token_jmp(){}
    ld_token_jmp():linked_list_token(),jmp(NULL){}
};

struct statistic_token{
    unsigned short op_inc_dec;
    unsigned short op_open_loop,op_close_loop;
    unsigned short op_putchar;
    unsigned short op_getchar;
    statistic_token():op_inc_dec(0),op_open_loop(0),op_close_loop(0),op_putchar(0),op_getchar(0){}
};

#define getByteBFCompile(statistic) (11 + statistic.op_inc_dec*3 + statistic.op_open_loop*9 + statistic.op_close_loop*5 + statistic.op_putchar*20)

typedef int (*BrainFuckCompiled)(char*);

/*
 * @brief: Add specific node ad token's graph 
 * @param[father]: node to add son
 * @param[op]: son's operation, if op is a jump operation then the son is a ld_token_jmp
 */
void addNext(linked_list_token *&father, char op);

/*
 * @brief: controll and packing code in a list
 * @param[code]: string of brainfuck code
 * @param[token]: is return
 */
bool lexing_parsing(const char* code,linked_list_token *&token,statistic_token &statistic);

/*
 * @brief: interpeter of brainfuck
 * @param[token]: brainfuck code in token
 * @param[ptr]: memory area
 */
void interpeter(linked_list_token* token,char* ptr);

/*
 * @brief: Just-In-Time Compiler for BrainFuck
 * @param[token]: brainfuck code in token
 * @param[statistic]: statistic for dimension mmap
 * @return: function pointer of compiled code
 */
BrainFuckCompiled compile(linked_list_token* token,const statistic_token&);

void clear_memory(char ptr[MAX_DIM_BUFFER]);

void freeBFcompiled(BrainFuckCompiled,const statistic_token &s);

#endif /* BRAINFUCK_H */

