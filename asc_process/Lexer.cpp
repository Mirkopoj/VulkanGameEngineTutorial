#include "Lexer.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <glm/ext/scalar_int_sized.hpp>
#include <string>

namespace Lexer {

Config::Config(std::string path) : path(path) {
   std::ifstream ifile(path + "config.txt");
   char key[255];
   char value[255];
   for (std::string line; std::getline(ifile, line);) {
      if (sscanf(line.c_str(), "%[^=]=%[^#]", key, value)) {
         config[key] = value;
      }
   }
}

std::string Config::value(std::string key) {
   return config[key];
}

std::string Config::get_path() {
   return path;
}

PaletDB::PaletDB(const char *path) {
   std::ifstream ifile(path);
   int32_t type;
   uint32_t id;
   char name[255];
   float r;
   float g;
   float b;
   for (std::string line; std::getline(ifile, line);) {
      if (sscanf(line.c_str(), "R%d=%d", &id, &type)) {
         types[type] = id;
      }
      if (sscanf(line.c_str(), "I%d=\"%[^\"]\",%f,%f,%f", &id, name, &r,
                 &g, &b)) {
         layer[id] = Color{.name = name,
                           .color = glm::vec3{r / 255, g / 255, b / 255}};
      }
   }
}

PaletDB::Color PaletDB::color(int32_t type) {
   uint32_t id = types[type];
   return layer[id];
}

Ascf loadf(const char *path) {
   std::ifstream ifile(path);
   int32_t NODATA_value;
   int cellsize;
   uint32_t xn;
   uint32_t yn;
   const int meta_data_lines = 6;
   std::string line;
   for (int i = 0; i < meta_data_lines; ++i) {
      std::getline(ifile, line);
      Line pair = parse(line);
      if (!std::strcmp(pair.name.c_str(), "ncols")) {
         xn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "nrows")) {
         yn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "NODATA_value")) {
         NODATA_value = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "cellsize")) {
         cellsize = pair.value;
         continue;
      }
   }
   std::vector<std::vector<glm::float32>> ret = {};
   for (std::string line; std::getline(ifile, line);) {
      std::vector<std::string> altitudeMapIn = tokenize(line);
      std::vector<glm::float32> aux = {};
      for (int x = 0; x < xn; ++x) {
         int val = std::stof(altitudeMapIn[x]);
         aux.push_back(val);
      }
      ret.push_back(aux);
   }
   return Ascf{
       .cellsize = cellsize,
       .NODATA_value = NODATA_value,
       .body = ret,
   };
}

Asci loadi(const char *path) {
   std::ifstream ifile(path);
   int32_t NODATA_value;
   int cellsize;
   uint32_t xn;
   uint32_t yn;
   const int meta_data_lines = 6;
   std::string line;
   for (int i = 0; i < meta_data_lines; ++i) {
      std::getline(ifile, line);
      Line pair = parse(line);
      if (!std::strcmp(pair.name.c_str(), "ncols")) {
         xn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "nrows")) {
         yn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "NODATA_value")) {
         NODATA_value = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "cellsize")) {
         cellsize = pair.value;
         continue;
      }
   }
   std::vector<std::vector<glm::int32>> ret = {};
   for (std::string line; std::getline(ifile, line);) {
      std::vector<std::string> altitudeMapIn = tokenize(line);
      std::vector<glm::int32> aux = {};
      for (int x = 0; x < xn; ++x) {
         int val = std::stoi(altitudeMapIn[x]);
         aux.push_back(val);
      }
      ret.push_back(aux);
   }
   return Asci{
       .cellsize = cellsize,
       .NODATA_value = NODATA_value,
       .body = ret,
   };
}

Line parse(std::string line) {
   Line ret;
   std::vector<std::string> words = tokenize(line);
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

std::vector<std::string> tokenize(std::string line) {
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

const char *LexicalError::what() const noexcept(true) {
   return msg;
}

}  // namespace Lexer
