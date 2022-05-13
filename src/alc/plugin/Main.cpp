#include <clang/Frontend/FrontendPluginRegistry.h>
#include "AlcoholPluginAction.hpp"

using namespace clang;

static FrontendPluginRegistry::Add<AlcoholPluginAction>
    X(/*Name=*/"alcohol",
      /*Desc=*/"Find memory leaks and violations using alias calculus theory");
