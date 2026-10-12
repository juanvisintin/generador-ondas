#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

// Convierte muestras entre -1.0 y 1.0 a enteros de 16 bits (recorta si se pasan del rango)
std::vector<int16_t> convertirA16(const std::vector<double>& muestras);

// Guarda un WAV mono de 16 bits. Lanza std::runtime_error si no puede escribir el archivo
void guardarWav(const std::string& ruta,
	const std::vector<int16_t>& muestras,
	uint32_t sampleRate);

namespace {
	// Convierte un valor entre -1.0 y 1.0 a un entero de 16 bits
	int16_t aEntero16(double x);

	// Escribe un entero de 16 bits sin signo en little - endian(byte chico primero)
	void escribirU16(std::ofstream& out, uint16_t valor);

	// Escribe un entero de 32 bits sin signo en little-endian
	void escribirU32(std::ofstream& out, uint32_t valor);
}