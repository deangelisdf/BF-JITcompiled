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

bool lexing_parsing(const char* code,linked_list_token *&token,statistic_token& statistic){
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
                statistic.op_inc_dec++;
                addNext(index,(*i));
                break;
            case '.': 
                statistic.op_putchar++;
                addNext(index,(*i));
                break;
            case ',':
                statistic.op_getchar++;
                addNext(index,(*i));
                break;
            case '[':
                statistic.op_open_loop++;
                addNext(index,(*i));
                tkn_jmp.push_back(dynamic_cast<ld_token_jmp*>(index->next));
                break;
            case ']':
                statistic.op_close_loop++;
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

void freeBFcompiled(BrainFuckCompiled fn,const statistic_token &s){
    munmap((void*)fn,getByteBFCompile(s));
}

void freeListToken(linked_list_token*& token){
    linked_list_token *t;
    while(token != NULL){
        t = token;
        token = token->next;
        delete t;
    }
}

BrainFuckCompiled compile(linked_list_token* token,const statistic_token &s){
    uint8_t *prg = (uint8_t*)mmap(NULL,getByteBFCompile(s), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
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
                *(i_prg++) = (uint8_t)((addr & 0xff000000000000)>>48);
                *(i_prg++) = (uint8_t)((addr & 0xff00000000000000)>>56);
                
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
                //mov    %rdi,%rbx
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x89;
                *(i_prg++) = 0xfb;
                
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
                
                //mov %rbx, %rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x89;
                *(i_prg++) = 0xdf;
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

#define addStatistic(op,sts) switch(op){ \
                             case '+': case'-':case'<':case'>': sts.op_inc_dec++; break; \
                             case '.': sts.op_putchar++; break; \
                             case ',': sts.op_getchar++; break; \
                             case '[': sts.op_open_loop++; break; \
                             case ']': sts.op_close_loop++; break; \
                             }

#define AddNodeO1(src,dst,sts)  if(dst->op == src->op) \
                                    dynamic_cast<linked_list_token_repeat*>(dst)->repeat++; \
                                else{ \
                                    dst->next = new linked_list_token_repeat(); \
                                    dst->next->op = src->op; \
                                    dynamic_cast<linked_list_token_repeat*>(dst->next)->repeat =1; \
                                    dst = dst->next; \
                                    addStatistic(dst->op,sts); \
                                } 

void optimizeListTokenO1(linked_list_token *source,linked_list_token *&dest,statistic_token &statistic){
    vector<ld_token_jmp*> tkn_jmp;
    linked_list_token* index;
    if(source!=NULL){//Add first node in destination list(optimizate list)
        if(source->op != '[' && source->op != ']'){
            dest = new linked_list_token_repeat();
            dest->op = source->op;
            dynamic_cast<linked_list_token_repeat*>(dest)->repeat = 1;
        }else{
            dest = new ld_token_jmp();
            dest->op = source->op;
            dynamic_cast<ld_token_jmp*>(dest)->jmp = dynamic_cast<ld_token_jmp*>(source)->jmp;
        }
        addStatistic(source->op,statistic);
        source = source->next;
    }
    
    index = dest;
    
    while(source != NULL){
        switch(source->op){
            case '<':case '>':case '+':case '-': 
                //AddNodeO1(source,dest,statistic);
                
                if(index->op == source->op) 
                    dynamic_cast<linked_list_token_repeat*>(index)->repeat++; 
                else{ 
                    index->next = new linked_list_token_repeat(); 
                    index->next->op = source->op; 
                    dynamic_cast<linked_list_token_repeat*>(index->next)->repeat =1; 
                    index = index->next; 
                    addStatistic(index->op,statistic); 
                }
                
                break;
            case '.':case ',':
                if(source->op == '.') statistic.op_putchar++;
                else statistic.op_getchar++;
                
                index->next = new linked_list_token_repeat;
                index = index->next;
                index->op = source->op;
                dynamic_cast<linked_list_token_repeat*>(index)->repeat = 1;
                break;
            case '[':
                statistic.op_open_loop++;
                index->next = new ld_token_jmp;
                index = index->next;
                index->op = source->op;
                tkn_jmp.push_back(dynamic_cast<ld_token_jmp*>(index));
                break;
            case ']':
                statistic.op_close_loop++;
                index->next = new ld_token_jmp;
                index = index->next;
                index->op = source->op;
                dynamic_cast<ld_token_jmp*>(index)->jmp = tkn_jmp[tkn_jmp.size()-1];
                tkn_jmp[tkn_jmp.size()-1]->jmp = index;
                tkn_jmp.pop_back();
                break;
        }
        
        source = source->next;
    }
}

BrainFuckCompiled compileO1(linked_list_token* token,const statistic_token &s){
    uint8_t *prg = (uint8_t*)mmap(NULL,getByteBFCompiledO1(s), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
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
                //subq repeat,%rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x83;
                *(i_prg++) = 0xef;
                *(i_prg++) = dynamic_cast<linked_list_token_repeat*>(token)->repeat;
                break;
            case '>':                
                //addq repeat,%rdi
                *(i_prg++) = 0x48;
                *(i_prg++) = 0x83;
                *(i_prg++) = 0xc7;
                *(i_prg++) = dynamic_cast<linked_list_token_repeat*>(token)->repeat;
                break;
            case '+':                
                //addb   $0x1,(%rdi)
                *(i_prg++) = 0x80;
                *(i_prg++) = 0x07;
                *(i_prg++) = dynamic_cast<linked_list_token_repeat*>(token)->repeat;
                break;
            case '-':                
                //subb   $0x1,(%rdi)
                *(i_prg++) = 0x80;
                *(i_prg++) = 0x2f;
                *(i_prg++) = dynamic_cast<linked_list_token_repeat*>(token)->repeat;
                break;
            case '.': //putchar(*ptr)
                while(dynamic_cast<linked_list_token_repeat*>(token)->repeat>0){
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
                    *(i_prg++) = (uint8_t)((addr & 0xff000000000000)>>48);
                    *(i_prg++) = (uint8_t)((addr & 0xff00000000000000)>>56);

                    //callq *%rax
                    *(i_prg++) = 0xff;
                    *(i_prg++) = 0xd0;

                    //mov %rbx, %rdi
                    *(i_prg++) = 0x48;
                    *(i_prg++) = 0x89;
                    *(i_prg++) = 0xdf;
                    dynamic_cast<linked_list_token_repeat*>(token)->repeat--;
                }
                break;
            case ',':
                while(dynamic_cast<linked_list_token_repeat*>(token)->repeat>0){
                    //*ptr =getchar();
                    //mov    %rdi,%rbx
                    *(i_prg++) = 0x48;
                    *(i_prg++) = 0x89;
                    *(i_prg++) = 0xfb;
                    
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
                    
                    //mov %rbx, %rdi
                    *(i_prg++) = 0x48;
                    *(i_prg++) = 0x89;
                    *(i_prg++) = 0xdf;
                    dynamic_cast<linked_list_token_repeat*>(token)->repeat--;
                }
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