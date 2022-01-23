#pragma once

#include <vector>

#include "game.hpp"
#include "question.hpp"

class GameBuilder
{
public:
    GameBuilder(int socket_fd);
    void add_question(Question question);
    Game build();

private:
    Question retrieve_question();

private:
    std::vector<Question> m_questions;
    int m_socket_fd;
};