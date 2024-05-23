#include "Lexer.hpp"

Lexer::Line Lexer::parse(std::string line) {
   Line ret;
   std::vector<std::string> words = Lexer::tokenize(line);
   if (words.size() < 2) {
      LexicalError e;
      throw e;
   }
	ret.name = words[0];
   try {
      ret.value = stoi(words[1]);
   } catch (...) {
      LexicalError e;
      throw e;
   }
   return ret;
}

std::vector<std::string> Lexer::tokenize(std::string line) {
   std::vector<std::string> ret;
   char prev = ' ';
   for (char &character : line) {
      if (!std::isspace(character)) {
         if (std::isspace(prev)) {
            std::string new_token;
            ret.push_back(new_token);
         }
         ret.back().push_back(character);
      }
      prev = character;
   }
   return ret;
}

const char *Lexer::LexicalError::what() const noexcept(true) {
   return msg;
}
