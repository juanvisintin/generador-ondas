#include <vector>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>

const double PI = 3.14159265358979323846;

// Convierte un valor entre -1.0 y 1.0 a un entero de 16 bits
int16_t aEntero16(double x) {
	return static_cast<int16_t>(x * 32767.0);
}

// Escribe un entero de 16 bits sin signo en little-endian (byte chico primero)
void escribirU16(std::ofstream& out, uint16_t valor) {
	char bytes[2];
	bytes[0] = static_cast<char>(valor & 0xFF);
	bytes[1] = static_cast<char>((valor >> 8) & 0xFF);
	out.write(bytes, 2);
}

// Escribe un entero de 32 bits sin signo en little-endian
void escribirU32(std::ofstream& out, uint32_t valor) {
	char bytes[4];
	bytes[0] = static_cast<char>(valor & 0xFF);
	bytes[1] = static_cast<char>((valor >> 8) & 0xFF);
	bytes[2] = static_cast<char>((valor >> 16) & 0xFF);
	bytes[3] = static_cast<char>((valor >> 24) & 0xFF);
	out.write(bytes, 4);
}

// Guarda muestras de 16 bits, mono, en un archivo WAV
void guardarWav(const std::string& ruta,
	const std::vector<int16_t>& muestras,
	uint32_t sampleRate) {

	std::ofstream out(ruta, std::ios::binary);

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

}

int main() {
	
	const int sampleRate = 44100; // muestras por segundo
	const double frecuencia = 440.0; // Hz
	const double duracion = 1.0; // segundos
	const double amplitud = 0.2;

	int totalMuestras = static_cast<int>(sampleRate * duracion);
	std::vector<int16_t> muestras;

	for (int i = 0; i < totalMuestras; ++i) {
		double t = static_cast<double>(i) / sampleRate; // instante en segundos
		double valor = std::sin(2.0 * PI * frecuencia * t); // entre -1.0 y 1.0
		muestras.push_back(aEntero16(valor));
	}

	std::cout << "\nTotal de muestras: " << muestras.size() << "\n";

	//auto minmax = std::minmax_element(muestras.begin(), muestras.end());

	/*std::cout << "Minimo: " << *minmax.first << " | Maximo: " << *minmax.second << "\n";*/
	// no entiendo todavía bien qué es todo esto...

	int16_t min = muestras[0];
	int16_t max = muestras[0];

	for (size_t i = 1; i < muestras.size(); ++i) {
		if (muestras[i] < min) {
			min = muestras[i];
		}
		if (muestras[i] > max) {
			max = muestras[i];
		}
	}

	std::cout << "\nMinimo: " << min << " | Maximo: " << max << "\n\n";

	for (int i = 0; i < 20; ++i) {
		std::cout << "Muestra " << i << ": " << muestras[i] << "\n";
	}

	std::cout << "\nGENERAR UN .wav";
	std::cout << "\nNombre del archivo: ";
	std::string nombre;
	std::getline(std::cin, nombre);
	nombre = nombre + ".wav";

	guardarWav(nombre, muestras, sampleRate);
	std::cout << "\nListo: se genero" << nombre << "\n";

	return 0;
}