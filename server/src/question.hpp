#pragma once

#include <string>
#include <vector>

namespace Kahoot
{
    struct Question
    {
        // Question's content.
        std::string question;
        // All answers.
        std::vector<std::string> answers;
        // Index of correct answer in `answers` vector.
        int correct_answer;
    };
}