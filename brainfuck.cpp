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
#include "brainfuck.h"
#include <string>
#include <vector>
#include <unistd.h>

#include <sys/mman.h>
using namespace std;

void addNext(linked_list_token *&father, char op){
    linked_list_token* temp;
    if(op != '[' && op != ']') 
        temp = new linked_list_token;
    else{
        temp = new ld_token_jmp;
        ((ld_token_jmp*)(temp))->jmp = NULL;
    }
    temp->op = op;
    temp->next = NULL;
    if(father != NULL)        
        father->next = temp;
    else
        father = temp;
}

inline void ltrim(string &s)
{
    register string::iterator i;
    register string::iterator end = s.end();
    for (i = s.begin(); i != end; i++)
        if (!isspace(*i))
            break; 
    s.erase(s.begin(), i);
}
inline void ftrim(string &s)
{
    register string::reverse_iterator i;
    register string::reverse_iterator rend = s.rend();
    for (i = s.rbegin(); i != rend; i++)
        if (!isspace(*i)) 
            break;
    s.erase(i.base(), s.end());
}

bool lexing_parsing(const char* code,linked_list_token *&token){
    string code_str = code;
    ltrim(code_str);
    ftrim(code_str);
    
    auto end=code_str.end();
    linked_list_token* index = NULL;
    vector<ld_token_jmp*> tkn_jmp;
    bool first_time = true,valide_char = true;
    for(auto i=code_str.begin();i!=end;i++){
        switch((*i)){//if char isn't part of lexical of BrainFu..K is ignored
            case '<': case '>':
            case '+': case '-':
            case '.': case ',':
                addNext(index,(*i));
                break;
            case '[':
                addNext(index,(*i));
                tkn_jmp.push_back(dynamic_cast<ld_token_jmp*>(index->next));
                break;
            case ']':
                if(tkn_jmp.size()>0){
                    addNext(index,(*i));
                    dynamic_cast<ld_token_jmp*>(index->next)->jmp = tkn_jmp[tkn_jmp.size()-1];
                    tkn_jmp[tkn_jmp.size()-1]->jmp = index->next;
                    tkn_jmp.pop_back();
                }
                else
                    return false;
                break;
            default:
                valide_char = false;
                break;
        }
        if(valide_char){
            if(first_time){
                first_time = false;
                token = index;
            }
            else
                index = index->next;
        }
        else
            valide_char = true;
    }
    return true;
}

void interpeter(linked_list_token* token,char* ptr){
    char* first_address = ptr;
    char* last_address = ptr +MAX_DIM_BUFFER;
    bool next_ok = true;
    while(token != NULL){
        switch(token->op){
            case '<':
                if((ptr-1) < first_address) ptr = last_address-1;
                else --ptr;
                break;
            case '>':
                if((ptr+1) == last_address) ptr = first_address;
                else ++ptr;
                break;
            case '+':
                ++(*ptr);
                break;
            case '-':
                --(*ptr);
                break;
            case '.':
                putchar(*ptr);
                break;
            case ',':
                *ptr =getchar();
                break;
            case '[':
                if((*ptr) == 0)
                    token = dynamic_cast<ld_token_jmp*>(token)->jmp;
                break;
            case ']':
                token = dynamic_cast<ld_token_jmp*>(token)->jmp;
                next_ok = false;
                break;
        }
        if(next_ok)
            token = token->next;
        else
            next_ok = true;
    }
}

BrainFuckCompiled compile(linked_list_token* token){
    uint8_t *prg = (uint8_t*)mmap(NULL,1024, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    uint8_t inc_ptr = 0;
    uint8_t* i_prg = prg;
    uint8_t* calc_prg_jmp;//serve per calcolare il salto
    uint64_t addr;//used to call function or pass arguments
    
    vector<uint8_t*> pointer_square;
    
    //Prolog
    *(i_prg++) = 0x55;// push rbp
    *(i_prg++) = 0x53;//push %rdx
    // mov	rbp, rsp
    *(i_prg++) = 0x48;
    *(i_prg++) = 0x89;
    *(i_prg++) = 0xe5;
    //mov    %rdi,%rbx
    *(i_prg++) = 0x48;
    *(i_prg++) = 0x89;
    *(i_prg++) = 0xfb;
    
    while(token != NULL){
       switch(token->op){
            case '<':
                //dec %rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0xff;
                *(i_prg++) = 0xcf;
                break;
            case '>':                
                //inc %rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0xff;
                *(i_prg++) = 0xc7;
                break;
            case '+':                
                //addb   $0x1,(%rdi)
                *(i_prg++) = 0x80;
                *(i_prg++) = 0x07;
                *(i_prg++) = 0x01;
                
                break;
            case '-':                
                //subb   $0x1,(%rdi)
                *(i_prg++) = 0x80;
                *(i_prg++) = 0x2f;
                *(i_prg++) = 0x01;
                break;
            case '.': //putchar(*ptr)
                //mov    %rdi,%rbx
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x89;
                *(i_prg++) = 0xfb;
                           
                //mov    %rdi,%edi
                *(i_prg++) = 0x8b;
                *(i_prg++) = 0x3f;
                //move putchar in rax
                //movabs $putchar,%rax
                *(i_prg++) = 0x48;
                *(i_prg++) = 0xb8;
                addr = reinterpret_cast<uint64_t>(putchar);
                *(i_prg++) = (uint8_t)(addr & 0xff);
                *(i_prg++) = (uint8_t)((addr & 0xff00)>>8);
                *(i_prg++) = (uint8_t)((addr & 0xff0000)>>16);
                *(i_prg++) = (uint8_t)((addr & 0xff000000)>>24);
                *(i_prg++) = (uint8_t)((addr & 0xff00000000)>>32);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>40);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>48);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>56);
                
                //callq *%rax
                *(i_prg++) = 0xff;
                *(i_prg++) = 0xd0;
                
                //mov %rbx, %rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x89;
                *(i_prg++) = 0xdf;
                
                break;
            case ',':
                //*ptr =getchar();
                //movabs $putchar,%rax
                *(i_prg++) = 0x48;
                *(i_prg++) = 0xb8;
                addr = reinterpret_cast<uint64_t>(getchar);
                *(i_prg++) = (uint8_t)(addr & 0xff);
                *(i_prg++) = (uint8_t)((addr & 0xff00)>>8);
                *(i_prg++) = (uint8_t)((addr & 0xff0000)>>16);
                *(i_prg++) = (uint8_t)((addr & 0xff000000)>>24);
                *(i_prg++) = (uint8_t)((addr & 0xff00000000)>>32);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>40);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>48);
                *(i_prg++) = (uint8_t)((addr & 0xff0000000000)>>56);
                
                //callq *%rax
                *(i_prg++) = 0xff;
                *(i_prg++) = 0xd0;
                
                break;
            case '[':
                pointer_square.push_back(i_prg);//save address than after change
                
                //cmpb 0x00,(%rbi)
                *(i_prg++) = 0x80;
                *(i_prg++) = 0x3f;
                *(i_prg++) = 0x00;
                
                //JE rel16/32  0F 84
                *(i_prg++) = 0x0f;
                *(i_prg++) = 0x84;
                *(i_prg++) = 0x00;
                *(i_prg++) = 0x00;
                *(i_prg++) = 0x00;
                *(i_prg++) = 0x00;
                break;
            case ']':
                calc_prg_jmp = (uint8_t*)(reinterpret_cast<uint64_t>(pointer_square.back()) - reinterpret_cast<uint64_t>(i_prg) - 5);
                addr = reinterpret_cast<uint64_t>(calc_prg_jmp);
                //jmp '['  jmp con indirizzamento relativo a quello attuale
                *(i_prg++) = 0xe9;
                *(i_prg++) = (uint8_t)(addr & 0xff);
                *(i_prg++) = (uint8_t)((addr & 0xff00)>>8);
                *(i_prg++) = (uint8_t)((addr & 0xff0000)>>16);
                *(i_prg++) = (uint8_t)((addr & 0xff000000)>>24);
                
                //change je ']'
                calc_prg_jmp = pointer_square.back()+1;
                switch(*calc_prg_jmp){
                    case 0xbf:
                        calc_prg_jmp+=8;//istr del contronto(6) + opcode di je(2)
                        break;
                    case 0x7f:
                        calc_prg_jmp+=5;
                        break;
                    case 0x3f:
                        calc_prg_jmp+=4;
                        break;
                }
                addr = reinterpret_cast<uint64_t>(i_prg) - reinterpret_cast<uint64_t>(calc_prg_jmp+4);
                *(calc_prg_jmp++) = (uint8_t)(addr & 0xff);
                *(calc_prg_jmp++) = (uint8_t)((addr & 0xff00)>>8);
                *(calc_prg_jmp++) = (uint8_t)((addr & 0xff0000)>>16);
                *(calc_prg_jmp++) = (uint8_t)((addr & 0xff000000)>>24);
                pointer_square.pop_back();
                break;
        }
        token = token->next;
    }
    
    *(i_prg++) = 0x5b; //pop rbx
    *(i_prg++) = 0x5d; //pop rbp
    *(i_prg++) = 0xc3; //ret
    
    return (BrainFuckCompiled)prg;
}

void clear_memory(char ptr[MAX_DIM_BUFFER]){
    for(int i=0;i<MAX_DIM_BUFFER;i++)
        ptr[i] = 0;
}