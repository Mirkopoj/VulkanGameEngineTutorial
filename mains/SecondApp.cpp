#include <nfd.h>

#include <cstdlib>
#include <exception>
#include <iostream>

#include "../apps/second_app.hpp"
int main(int argc, char* argv[]) {
   NFD_Init();

   lve::SecondApp app{};
   if (argc > 1) {
      app.asyncLoadGameObjects(argv[1]);
   }

   try {
      app.run();
   } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
      return EXIT_FAILURE;
   }

   NFD_Quit();

   return EXIT_SUCCESS;
}
