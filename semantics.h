#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <iostream>
#include <map>
#include <vector>
#include <sstream>


typedef enum { type_integer, type_float, type_boolean } type;

struct row
{
	type var_type;
	int def_line;
	std::string label;
	
	row(type t, int l, std::string lab) : var_type(t), def_line(l), label(lab) {}
	row() {}
};

typedef struct expr_attribute
{
	type expr_type;
	std::string code;
	
	expr_attribute(type t, std::string c) : expr_type(t), code(c) {}
} expr_attribute;

typedef struct labeled_code
{
	std::string label;
	std::string code;

	labeled_code(std::string _label, std::string _code) : label(_label), code(_code) {}
} labeled_code;

#endif
