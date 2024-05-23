#pragma once
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
namespace Lexer {

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

}; // namespace Lexer
