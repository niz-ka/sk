#pragma once

#include <string>
#include <vector>

struct Question
{
    // Question's content.
    std::string question;
    // All answers.
    std::vector<std::string> answers;
    // Index of correct answer in `answers` vector.
    int correct_answer;
};
