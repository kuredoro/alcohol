#pragma once

#include <string>

namespace ast
{

    struct expression
    {
        void replace(const std::string& var, const expression& withExpr)
        {

        }

        bool operator=(const expression& other) const
        {
            return false;
        }

        std::string to_string() const
        {
            return "<expr>";
        }
    };

};
