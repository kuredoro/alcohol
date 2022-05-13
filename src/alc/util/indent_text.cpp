#include <alc/util/indent_text.hpp>

namespace util
{

std::string indent_text(const std::string& text)
{
    std::string result;
    result.resize(text.size());

    bool lineStart = true;
    for (auto ch : text)
    {
        if (lineStart)
        {
            result += "    ";
        }

        result.push_back(ch);

        lineStart = ch == '\n';
    }

    return result;
}

}
