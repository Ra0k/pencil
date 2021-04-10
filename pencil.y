%{

#include "semantics.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>


extern int yylex();
extern int yyparse();
extern int yylineno;

extern FILE* yyin;

void yyerror(const char* s);

int label_index = 0;
std::string new_label();
std::map<std::string,row> symbol_table;
%}

%union
{
	std::string *name;
	type *expr_type;
	std::string *code;
	expr_attribute *expr_attr;
	labeled_code *lcode;
	std::vector<std::string> *variables;
}
%locations

%token T_PROGRAM
%token T_BEGIN
%token T_END
%token T_INTEGER 
%token T_BOOLEAN
%token T_SKIP
%token T_IF
%token T_ELIF
%token T_THEN
%token T_ELSE
%token T_ENDIF
%token T_WHILE
%token T_DO
%token T_DONE
%token T_READ
%token T_WRITE
%token T_SEMICOLON
%token T_ASSIGN
%token T_OPEN
%token T_CLOSE
%token T_OPEN_BRACES
%token T_CLOSE_BRACES
%token T_TILDE
%token T_COMMA
%token T_BACK_ARROW
%token <name> T_NUM
%token <name> T_FLOAT
%token T_TRUE
%token T_FALSE
%token <name> T_ID

%left T_OR
%left T_AND
%left T_NOT
%left T_EQ
%left T_LESS T_GR
%left T_ADD T_SUB
%left T_MUL T_DIV T_MOD

%type <expr_attr> expression
%type <code> instruction
%type <code> instructions
%type <code> assignment
%type <code> read
%type <code> write
%type <code> branch
%type <lcode> subbranch
%type <code> loop
%type <variables> variable_list

%%
start:
    T_PROGRAM T_OPEN_BRACES instructions T_CLOSE_BRACES
    {
		std::cout << program(symbol_table, *$3);
    }
;

variable_list:
	T_ID
	{
		if(symbol_table.count(*$1) > 0) {
			yyerror("Redeclared variable.");
		}
		symbol_table[*$1] = row(type_integer,yylloc.first_line,new_label());

		std::vector<std::string> *start = new std::vector<std::string>();
		start->push_back(*$1);
		$$ = start;
	}
| 
	T_ID T_COMMA variable_list
	{
		if(symbol_table.count(*$1) > 0) {
			yyerror("Redeclared variable.");
		}
		symbol_table[*$1] = row(type_integer,yylloc.first_line,new_label());

		$3->push_back(*$1);
		$$ = $3;
	}
;

declaration:
    variable_list T_TILDE T_INTEGER
    {
		for(std::string x : *$1) {
			symbol_table[x].var_type = type_integer;
		}
		delete $1;
    }
|
    variable_list T_TILDE T_BOOLEAN
    {
		for(std::string x : *$1) {
			symbol_table[x].var_type = type_boolean;
		}
		delete $1;
    }
;

instructions:
    instruction
    {
		$$ = $1;
    }
|
    instruction instructions
    {
		$$ = concat($1, $2);
		delete $1;
		delete $2;
    }
;

instruction:
	declaration {
		$$ = new std::string();
	}
|
    T_SKIP
    {
		$$ = nop();
    }
|
    assignment
    {
		$$ = $1;
    }
|
    read
    {
		$$ = $1;
    }
|
    write
    {
		$$ = $1;
    }
|
    branch
    {
		$$ = $1;
    }
|
    loop
    {
		$$ = $1;
    }
;

assignment:
    T_ID T_BACK_ARROW expression
    {
        if(symbol_table.count(*$1) < 1) {
			std::stringstream ss;
			ss << "Undeclared variable: " << *$1;
			yyerror(ss.str().c_str());
        }
        if(symbol_table[*$1].var_type != $3->expr_type) {
			yyerror("Type error in assignment.");
        }
        
		$$ = token_assigment(symbol_table[*$1], $3->code);
        
		delete $1;
		delete $3;
    }
;

read:
    T_READ T_OPEN T_ID T_CLOSE
    {
        if(symbol_table.count(*$3) < 1) {
			std::stringstream ss;
			ss << "Undeclared variable: " << *$3;
			yyerror(ss.str().c_str());
        }
        if(symbol_table[*$3].var_type == type_integer) {
			std::string params[] = {};
			std::string read_call = *calling_c_function("_read_unsigned", params, 0);
			$$ = new std::string(
				std::string(read_call
					+ "mov [rel " + symbol_table[*$3].label + "],rax\n"
				)
			);
        } else {
			$$ = new std::string(
				std::string("call _read_unsigned\n")
				+ "mov [rel " + symbol_table[*$3].label + "],al\n"
			);
        }
		delete $3;
    }
;

write:
    T_WRITE T_OPEN expression T_CLOSE
    {
		std::string params[] = {$3->code};
        if($3->expr_type == type_integer) {
			$$ = calling_c_function("_write_unsigned", params, 1);
        } else {
			$$ = calling_c_function("_write_boolean", params, 1);
        }
		delete $3;
    }
;

subbranch:
	//empty 
	{
		$$ = new labeled_code("", "");
	}
|
	T_ELIF expression T_OPEN_BRACES instructions T_CLOSE_BRACES subbranch
	{
		if($2->expr_type != type_boolean) {
			yyerror("Condition of if-instruction is not boolean.");
		}

		std::string label = new_label();
		$$ = if_block(label, $2->code, *$4, $6);

		delete $2;
		delete $4;
		delete $6;
	}
| 
	T_ELSE T_OPEN_BRACES instructions T_CLOSE_BRACES
	{
		std::string label = new_label();
		$$ = else_block(label, *$3);

		delete $3;
	}
;


branch:
    T_IF expression T_OPEN_BRACES instructions T_CLOSE_BRACES subbranch
    {
		if($2->expr_type != type_boolean) {
			yyerror("Condition of if-instruction is not boolean.");
		}
		
		std::string label = new_label();
		$$ = if_branch(label, $2->code, *$4, $6);

		delete $2;
		delete $4;
		delete $6;
    }
;

loop:
    T_WHILE expression T_OPEN_BRACES instructions T_CLOSE_BRACES
    {
		if($2->expr_type != type_boolean) {
			yyerror("Condition of while-loop is not boolean.");
		}
		std::string label = new_label();
		$$ = while_loop(label, $2->code, *$4);

		delete $2;
		delete $4;
    }
;

expression:
    T_NUM
    {
        $$ = exp_integer(*$1);
		delete $1;
    }
|
    T_TRUE
    {
        $$ = exp_boolean_true();
    }
|
    T_FALSE
    {
        $$ = exp_boolean_false();
    }
|
    T_ID
    {
        if(symbol_table.count(*$1) == 0) {
			std::stringstream ss;
			ss << "Undeclared variable: " << *$1;
			yyerror(ss.str().c_str());
        }
		$$ = exp_variable(symbol_table[*$1]);
		delete $1;
    }
|
    expression T_ADD expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of '+' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of '+' is not integer.");
		}
		$$ = num_op_num_eq_num("+", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_SUB expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of '-' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of '-' is not integer.");
		}
		$$ = num_op_num_eq_num("-", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_MUL expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of '*' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of '*' is not integer.");
		}
		$$ = num_op_num_eq_num("*", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_DIV expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of 'div' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of 'div' is not integer.");
		}
		$$ = num_op_num_eq_num("/", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_MOD expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of 'mod' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of 'mod' is not integer.");
		}
		$$ = num_op_num_eq_num("mod", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_LESS expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of '<' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of '<' is not integer.");
		}
		std::string label = new_label();
		$$ = num_op_num_eq_bin("<", $1, $3, label);
		delete $1;
		delete $3;
    }
|
    expression T_GR expression
    {
		if($1->expr_type != type_integer) {
			yyerror("Left operand of '>' is not integer.");
		}
		if($3->expr_type != type_integer) {
			yyerror("Right operand of '>' is not integer.");
		}
		std::string label = new_label();
		$$ = num_op_num_eq_bin(">", $1, $3, label);
		delete $1;
		delete $3;
    }
|
    expression T_EQ expression
    {
		if($1->expr_type != $3->expr_type) {
			yyerror("Left and right operands of '=' have different types.");
		}
		std::string label = new_label();
		std::string reg_a, reg_b;
		if($1->expr_type == type_integer) {
		    $$ = num_op_num_eq_bin("=", $1, $3, label);
		} else {
		    $$ = bin_op_bin_eq_bin("=", $1, $3, label);
		}

		delete $1;
		delete $3;
    }
|
    expression T_AND expression
    {
		if($1->expr_type != type_boolean) {
			yyerror("Left operand of 'and' is not boolean.");
		}
		if($3->expr_type != type_boolean) {
			yyerror("Right operand of 'and' is not boolean.");
		}
		$$ = bin_op_bin_eq_bin("and", $1, $3);
		delete $1;
		delete $3;
    }
|
    expression T_OR expression
    {
		if($1->expr_type != type_boolean) {
			yyerror("Left operand of 'or' is not boolean.");
		}
		if($3->expr_type != type_boolean) {
			yyerror("Right operand of 'or' is not boolean.");
		}
		$$ = bin_op_bin_eq_bin("or", $1, $3);
		delete $1;
		delete $3;
    }
|
    T_NOT expression
    {
		if($2->expr_type != type_boolean) {
			yyerror("Operand of 'not' is not boolean.");
		}
		$$ = op_bin_eq_bin("not", $2);
		delete $2;
    }
|
    T_OPEN expression T_CLOSE
    {
        $$ = $2;
    }
;

%%

int main() {
	yyin = stdin;

	do {
		yyparse();
	} while(!feof(yyin));

	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s at line: %d\n", s, yylineno);
	exit(1);
}

std::string new_label() {
	std::stringstream ss;
	ss << "label" << label_index++;
	return ss.str();
}