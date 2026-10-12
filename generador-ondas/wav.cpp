#include "wav.h"

std::vector<int16_t> convertirA16(const std::vector<double>& muestras) {
	std::vector<int16_t> resultado;
	for (double m : muestras) {
		resultado.push_back(aEntero16(m));
	}
	return resultado;
};

void guardarWav(const std::string& ruta,
	const std::vector<int16_t>& muestras,
	uint32_t sampleRate) {

	std::ofstream out(ruta, std::ios::binary);
	if (!out) {
		throw std::runtime_error("No se pudo crear el archivo " + ruta);
	}

	const uint16_t canales = 1;
	const uint16_t bitsPorMuestra = 16;
	const uint16_t blockAlign = static_cast<uint16_t>(canales * bitsPorMuestra / 8); // bytes por frame
	const uint32_t byteRate = sampleRate * blockAlign; // bytes por segundo
	const uint32_t tamanoDatos = static_cast<uint32_t>(muestras.size() * 2); // 2 bytes por muestra

	// Chunk RIFF
	out.write("RIFF", 4);
	escribirU32(out, 32 + tamanoDatos);
	out.write("WAVE", 4);

	// Chunk "fmt "
	out.write("fmt ", 4);
	escribirU32(out, 16);
	escribirU16(out, 1);
	escribirU16(out, canales);
	escribirU32(out, sampleRate);
	escribirU32(out, byteRate);
	escribirU16(out, blockAlign);
	escribirU16(out, bitsPorMuestra);

	// Chunk "data"
	out.write("data", 4);
	escribirU32(out, tamanoDatos);
	for (int16_t m : muestras) {
		escribirU16(out, static_cast<uint16_t>(m));
	}

	if (!out) {
		throw std::runtime_error("Error al escribir el archivo " + ruta);
	}
}

namespace {
	int16_t aEntero16(double x) {
		if (x > 1.0) x = 1.0;
		if (x < -1.0) x = -1.0;
		return static_cast<int16_t>(x * 32767.0);
	}

	void escribirU16(std::ofstream& out, uint16_t valor) {
		char bytes[2];
		bytes[0] = static_cast<char>(valor & 0xFF);
		bytes[1] = static_cast<char>((valor >> 8) & 0xFF);
		out.write(bytes, 2);
	}

	void escribirU32(std::ofstream& out, uint32_t valor) {
		char bytes[4];
		bytes[0] = static_cast<char>(valor & 0xFF);
		bytes[1] = static_cast<char>((valor >> 8) & 0xFF);
		bytes[2] = static_cast<char>((valor >> 16) & 0xFF);
		bytes[3] = static_cast<char>((valor >> 24) & 0xFF);
		out.write(bytes, 4);
	}
}