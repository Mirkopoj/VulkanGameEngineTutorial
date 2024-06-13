#pragma once
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <glm/fwd.hpp>
#include <map>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Lexer {

class Config {
  public:
   Config(const std::filesystem::path &);

   std::string value(std::string key);
   std::filesystem::path get_path();

  private:
   std::filesystem::path path;
   std::map<std::string, std::string> config = {};
};

class PaletDB {
  public:
   typedef struct {
      std::string name;
      glm::vec3 color;
   } Color;

   PaletDB(const std::filesystem::path &path);

   Color color(int32_t type);

  private:
   std::map<int32_t, uint32_t> types = {};
   std::map<uint32_t, Color> layer = {};
};

typedef struct {
   int cellsize;
   int32_t NODATA_value;
   std::vector<std::vector<glm::float32>> body;
} Ascf;

/**
 * @brief Procesa un archivo .asc y devuelve una matriz de float32.
 * @param path al archivo que se quiere procesar.
 * @return Una matriz de ncols x nrow con el cuerpo del .asc.
 * @throws LexicalError Si se encuentra un token no válido.
 */
Ascf loadf(const std::filesystem::path &path);

typedef struct {
   int cellsize;
   int32_t NODATA_value;
   std::vector<std::vector<glm::int32>> body;
} Asci;

/**
 * @brief Procesa un archivo .asc y devuelve una matriz de int32.
 * @param path al archivo que se quiere prcesar.
 * @return Una matriz de ncols x nrow con el cuerpo del .asc.
 * @throws LexicalError Si se encuentra un token no válido.
 */
Asci loadi(const std::filesystem::path &path);

typedef struct {
   std::string name;
   int32_t value;
} Line;

/**
 * @brief Procesa una linea de metadatos del asc.
 * @param line La línea de texto.
 * @return Line separada en su nombre y su valor.
 * @throws LexicalError Si se encuentra un token no válido.
 */
Line parse(std::string);

/**
 * @brief Divide una línea de texto en palabras individuales.
 * @param line La línea de texto.
 * @return Vector de palabras identificadas.
 */
std::vector<std::string> tokenize(std::string line);

class LexicalError : public std::exception {
  public:
   LexicalError() = default;
   ~LexicalError() = default;
   LexicalError(LexicalError &&) = default;
   LexicalError(const LexicalError &) = default;
   const char *what() const noexcept(true) override;

  private:
   const char *msg = "LexicalError: Found an unrecognized token";
};

};  // namespace Lexer
