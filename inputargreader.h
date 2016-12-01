#ifndef INPUTARGREADER_H
#define INPUTARGREADER_H

#include <iostream>
#include <vector>
#include <string>

namespace Tools {

    template<int COUNT, bool EXIT_ON_MISMATCH = true>
    std::vector<std::string> input_arg_reader(int argc, char**argv)
    {
        if(!((argc - 1) == COUNT))
        {
            std::cout << __FUNCTION__ << " Input error : " << std::endl;
            std::cout << " Input args expected : " << COUNT << std::endl;
            std::cout << " Input args received : " << argc - 1<< std::endl;
            std::cout << " Notice : "
                         " The Programm name is not take into account as"
                         " input argument since it is automatically passed"
                         " when calling this programm" << std::endl;
            if(EXIT_ON_MISMATCH)
            {
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            std::vector<std::string> args;
            for(int i = 0; i < argc; i++)
            {
                args.push_back(argv[i]);
            }
            return args;
        }

    }
}
#endif // INPUTARGREADER_H
