#ifndef _base
#define _base
#include <bits/stdc++.h>

void sintax_error(std::string msg)
{
    std::cerr << "sintax error: " << msg << std::endl;
    exit(1);
}

void compile_error(std::string msg)
{
    std::cerr << "compile error: " << msg << std::endl;
    exit(1);
}

#endif