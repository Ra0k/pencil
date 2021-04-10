#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantics.h"
#include <iostream>
#include <map>
#include <sstream>
#include <algorithm>

std::string program(std::map<std::string,row> symbol_table, std::string insturctions);
std::string entry_point();
std::string header();
std::string* builtinfunctions();
std::string bss_segment(std::map<std::string,row> symbol_table);
std::string text_segment(std::string* text);

std::string* nop();
std::string integer_token_assigment(std::string token_label);
std::string boolean_token_assigment(std::string token_label);
std::string* token_assigment(row& token, std::string exp);
std::string* calling_c_function(std::string name, std::string expressions[], int num);
labeled_code* if_block(std::string label, std::string condition, std::string instructions, labeled_code* subbranch);
labeled_code* else_block(std::string label, std::string instructions);
std::string* if_branch(std::string label, std::string condition, std::string instructions, labeled_code* subbranch);
std::string* while_loop(std::string label, std::string condition, std::string instructions);

expr_attribute* exp_float(std::string num);
expr_attribute* exp_integer(std::string num);
expr_attribute* exp_boolean_true();
expr_attribute* exp_boolean_false();
expr_attribute* exp_variable(row var);
expr_attribute* bin_op_bin_eq_bin(std::string op, expr_attribute* exp1, expr_attribute* exp2, std::string label);
expr_attribute* num_op_num_eq_bin(std::string op, expr_attribute* exp1, expr_attribute* exp2, std::string label);
expr_attribute* num_op_num_eq_num(std::string op, expr_attribute* exp1, expr_attribute* exp2);
expr_attribute* op_bin_eq_bin(std::string op, expr_attribute* exp);

std::string* concat(std::string* a, std::string* b);


std::string program(std::map<std::string,row> symbol_table, std::string insturctions) {
    return std::string(
        header() + 
	    bss_segment(symbol_table) + 
	    text_segment(&insturctions)
    );
}

std::string entry_point() {
    return std::string("_main");
}

std::string header() {
    return std::string(
        "global " + entry_point() + "\n"
		+ *builtinfunctions() +
		"default rel\n"
		"\n"
    );
}

std::string* builtinfunctions() {
    return new std::string(
        "extern _read_unsigned\n"
		"extern _write_unsigned\n"
		"extern _read_boolean\n"
		"extern _write_boolean\n"
    );
}

std::string bss_segment(std::map<std::string,row> symbol_table) {
    std::string bss = "section .bss\n";

    for(std::pair<std::string,row> v : symbol_table) {
        if(v.second.var_type == type_integer) {
            bss += v.second.label + ": resq 1\n";
        } else {
            bss += v.second.label + ": resb 1\n";	
        }
    }

    return std::string(bss);
}

std::string text_segment(std::string* text) {
    return std::string(
        "\n"
		"section .text\n"
		+ entry_point() + ":\n"
		"push rbx\n"
		+ *text + "\n"
		"pop rbx\n"
		"ret\n"
    );
}

std::string* concat(std::string* a, std::string* b) {
    return new std::string(*a + *b);
}

std::string* nop() {
    return new std::string(
        "nop\n"
    );
}

std::string integer_token_assigment(std::string token_label) {
    return std::string(
        "mov [rel " + token_label + "],rax\n"
    );
}

std::string boolean_token_assigment(std::string token_label) {
    return std::string(
        "mov [rel " + token_label + "],al\n"
    );
}

std::string* token_assigment(row& token, std::string exp) {
    if(token.var_type == type_integer) {
        return new std::string(
            exp + integer_token_assigment(token.label)
        );
    } else {
        return new std::string(
            exp + boolean_token_assigment(token.label)
        );
	}
}


std::string* calling_c_function(std::string name, std::string expressions[], int num) {
    std::string priority[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int num_register_parameters = 6;
    int num_stack_parameters = std::max(0, num - num_register_parameters);

    std::string var = "";
    for(int i = num-1; i > num_register_parameters - 1; i--) {
        var += expressions[i] + "\n" 
            "push rax\n";
        
    }
    for(int i = std::min(num_register_parameters - 1, num-1); i >= 0; i--) {
        var += expressions[i] + "\n" 
            "mov " + priority[i] + ", rax\n";
    }
    
    return new std::string(
        "push rcx\n"
        "push rdx\n"
        "push rsi\n"
        "push rdi\n"
        "push r8\n"
        "push r9\n"
        + var + 
        "call " + name + "\n"
        "add rsp, " + std::to_string(8 * num_stack_parameters) + "\n"
        "pop r9\n"
        "pop r8\n"
        "pop rdi\n"
        "pop rsi\n"
        "pop rdx\n"
        "pop rcx\n"
    );
}

labeled_code* if_block(std::string label, std::string condition, std::string instructions, labeled_code* subbranch) {
    std::string end_label = subbranch->label;
    if(end_label == "") {
        end_label = label + "_end";
    }
    
    std::string code = 
        condition + 
        "cmp al,1\n"
        "jne near " + label + "\n" + 
        instructions + 
        "jmp near " + end_label + "\n" + 
        label + ":\n" + subbranch->code;

    return new labeled_code(end_label, code);
}

labeled_code* else_block(std::string label, std::string instructions) {
    label = label + "_end";
    
    std::string code = 
        instructions + 
        label + ":\n";

    return new labeled_code(label, code);
}

std::string* if_branch(std::string label, std::string condition, std::string instructions, labeled_code* subbranch) {
    labeled_code* r = if_block(label, condition, instructions, subbranch);
    std::string* code = new std::string(
        r->code +
        r->label + ":\n"
    );
    delete r;

    return code;
}

std::string* while_loop(std::string label, std::string condition, std::string instructions) {
    return new std::string(
		label + "_begin:\n" +
        condition +
        "cmp al,1\n"
        "jne near " + label + "_end\n" + 
        instructions + 
        "jmp " + label + "_begin\n" + 
        label + "_end:\n"
    );
}

expr_attribute* exp_integer(std::string num) {
    return new expr_attribute(type_integer, "mov rax," + num + "\n");
}

expr_attribute* exp_float(std::string num) {
    return new expr_attribute(type_float, "mov rax," + num + "\n");
}

expr_attribute* exp_boolean_true() {
    return new expr_attribute(type_boolean, "mov al,1\n");
}

expr_attribute* exp_boolean_false() {
    return new expr_attribute(type_boolean, "mov al,0\n");
}

expr_attribute* exp_variable(row var) {
    if(var.var_type == type_integer) {
        return new expr_attribute(
            type_integer,
            std::string("mov rax,[rel " + var.label + "]\n")
        );
    }else {
        return new expr_attribute(
            type_boolean,
            std::string("mov al,[rel " + var.label + "]\n")
        );
    }
}

expr_attribute* num_op_num_eq_num(std::string op, expr_attribute* exp1, expr_attribute* exp2) {
    if(op == "+") {
        return new expr_attribute(type_integer,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
		    "add rax,rbx\n"
        );
    }else if (op == "-") {
        return new expr_attribute(type_integer,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
		    "sub rax,rbx\n"
        );
    }else if (op == "*") {
        return new expr_attribute(type_integer,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
		    "mul rbx\n"
        );
    }else if (op == "/") {
        return new expr_attribute(type_integer,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "push rdx\n"
            "mov rdx,0\n"
		    "div rbx\n"
            "pop rdx"
        );
    }else if (op == "mod") {
        return new expr_attribute(type_integer,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "push rdx\n"
            "mov rdx,0\n"
		    "div rbx\n"
            "mov rax,rdx\n"
		    "pop rdx\n"
        );
    }
}

expr_attribute* num_op_num_eq_bin(std::string op, expr_attribute* exp1, expr_attribute* exp2, std::string label) {
    if(op == "<") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "jl " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == "<=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "jle " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == ">") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "jg " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == ">=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "jge " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == "=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "je " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == "!=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp rax,rbx\n"
            "jne " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }
}

expr_attribute* bin_op_bin_eq_bin(std::string op, expr_attribute* exp1, expr_attribute* exp2, std::string label="") {
    if(op == "=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp al,bl\n"
            "je " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if(op == "!=") {
        return new expr_attribute(type_boolean,
            exp2->code + 
		    "push rax\n" +
		    exp1->code +
		    "pop rbx\n"
            "cmp al,bl\n"
            "jne " + label + "_yes\n"
            "mov al,0\n"
            "jmp " + label + "_end\n" +
            label + "_yes:\n"
            "mov al,1\n" +
		    label + "_end:\n"
        );
    }else if (op == "and") {
        return new expr_attribute(type_boolean,
		    exp2->code + 
            "push rax\n" + 
		    exp1->code +
		    "pop rbx\n"
		    "and al,bl\n"
		);
    }else if (op == "or") {
        return new expr_attribute(type_boolean,
		    exp2->code + 
            "push rax\n" + 
		    exp1->code +
		    "pop rbx\n"
		    "or al,bl\n"
		);
    }else if (op == "xor") {
        return new expr_attribute(type_boolean,
		    exp2->code + 
            "push rax\n" + 
		    exp1->code +
		    "pop rbx\n"
		    "xor al,bl\n"
		);
    }
}

expr_attribute* op_bin_eq_bin(std::string op, expr_attribute* exp) {
    if(op == "not") {
        return new expr_attribute(type_boolean,
            exp->code + 
		    "xor al,1\n"
        );
    }
}

#endif
