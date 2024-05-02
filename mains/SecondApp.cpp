#include <cstdlib>
#include <exception>
#include <iostream>

#include "../apps/second_app.hpp"

int main() {
   lve::SecondApp app{};

   try {
      app.run();
   } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
