#include "game_builder.hpp"

GameBuilder::GameBuilder(int socket_fd) : m_socket_fd(socket_fd), m_questions({}) {}

void GameBuilder::add_question(Question question) {
    m_questions.emplace_back(question);
}

Game GameBuilder::build()
{
    // TODO
}

Question GameBuilder::retrieve_question() {
    // We're waiting for one of signals: NQN, ANS, FGC or AGC.
    
}