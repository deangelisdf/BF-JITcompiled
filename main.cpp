/*
 * Copyright (C) 2018 deangelis
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
 * File:   main.cpp
 * Author: deangelis
 *
 * Created on 22 settembre 2018, 19.53
 */

#include "brainfuck.h"
void test(const char* code,char *ptr){
    printf("\nTEST:\n%s\n\n",code);
    linked_list_token* token;
    BrainFuckCompiled fn = NULL;
    clear_memory(ptr);
    if(lexing_parsing(code,token)){
        printf("Interpeter Result:\n");
        interpeter(token,ptr);
        printf("<-\n");
        fn = compile(token);
    }
    else{
        printf("error");
    }
    clear_memory(ptr);
    if(fn!=NULL){
        printf("JIT Result:\n");
        fn(ptr);
        printf("<-\n");
    }
}

int main(int argc, char** argv) {
    const char* code = "+[-[<<[+[--->]-[<<<]]]>>>-]>-.---.>..>.<<<<-.<+.>>>>>.>.<<.<-."; //hello world
    //code: i forgot where i got it
    const char* code2 = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";//Hello World!\n
    //code2: ref https://esolangs.org/wiki/Brainfuck#Memory_and_wrapping
    char ptr[MAX_DIM_BUFFER] ={0};
    
    test(code,ptr);
    test(code2,ptr);
    
    return 0;
}