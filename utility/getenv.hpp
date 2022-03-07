#ifndef getenv_hpp_included_
#define getenv_hpp_included_

#include "env.hpp"

#ifndef _CRT_STRINGIZE
#define _CRT_STRINGIZE_(x) #x
#define _CRT_STRINGIZE(x) _CRT_STRINGIZE_(x)
#endif

#pragma message(__FILE__ "(" _CRT_STRINGIZE(__LINE__) ")" ": #include \"utility/getenv.hpp\" is deprecated, include \"env.hpp\" instead")

#endif