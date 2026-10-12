#include <vector>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
#endif

const double PI = 3.14159265358979323846;

enum class Onda { Senoidal, Cuadrada, Sierra, Triangular };

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

// "fase" va de 0.0 a 1.0 y representa en qué punto del ciclo estamos
double muestraDeOnda(Onda tipo, double fase) {
	switch (tipo) {
	case Onda::Senoidal:	return std::sin(2.0 * PI * fase);
	case Onda::Cuadrada:	return fase < 0.5 ? 1.0 : -1.0;
	case Onda::Sierra:		return 2.0 * fase - 1.0;
	case Onda::Triangular:	return 1.0 - 4.0 * std::fabs(fase - 0.5);
	}
	return 0.0;
}

struct Envolvente {
	double ataque; // segundos que tarda en subir de 0 a 1
	double decaimiento; // segundos que tarda en bajar de 1 al nivel sostenido
	double sustain; // nivel de sostenido (0.0 a 1.0)
	double liberacion; // segundos que tarda en bajar a 0 al final
};

// Devuelve el volumen (0.0 a 1.0) en el instante t de una nota de "duración" segundos
double valorEnvolvente(double t, double duracion, const Envolvente& e) {
	double inicioLiberacion = duracion - e.liberacion;

	if (t < e.ataque) {
		return t / e.ataque;
	}
	if (t < e.ataque + e.decaimiento) {
		return 1.0 - (1.0 - e.sustain) * (t - e.ataque) / e.decaimiento;
	}
	if (t < inicioLiberacion) {
		return e.sustain;
	}
	double v = e.sustain * (1.0 - (t - inicioLiberacion) / e.liberacion);
	return v < 0.0 ? 0.0 : v;
}

std::vector<double> generarOnda(Onda tipo, double frecuencia, double duracion,
	uint32_t sampleRate, double amplitud, const Envolvente& env) {
	int total = static_cast<int>(sampleRate * duracion);
	std::vector<double> muestras;
	
	double fase = 0.0;
	double incremento = frecuencia / sampleRate; // cuánto avanza la fase en cada muestra
	
	for (int i = 0; i < total; ++i) {
		double t = static_cast<double>(i) / sampleRate;
		double volumen = valorEnvolvente(t, duracion, env);
		muestras.push_back(amplitud * volumen * muestraDeOnda(tipo, fase));
		fase += incremento;
		if (fase >= 1.0) {
			fase -= 1.0;
		}
	}
	return muestras;
}

std::vector<int16_t> convertirA16(const std::vector<double>& muestras) {
	std::vector<int16_t> resultado;
	for (double m : muestras) {
		resultado.push_back(aEntero16(m));
	}
	return resultado;
};

// Convierte una nota como "C4", "F#3" o "Bb5" a su frecuencia en Hz
double frecuenciaDeNota(const std::string& nota) {
	int semitono = 0;
	switch (nota[0]) {
		case 'C': semitono = 0; break;
		case 'D': semitono = 2; break;
		case 'E': semitono = 4; break;
		case 'F': semitono = 5; break;
		case 'G': semitono = 7; break;
		case 'A': semitono = 9; break;
		case 'B': semitono = 11; break;
	}
	size_t pos = 1;
	if (nota[pos] == '#') {
		semitono += 1;
		pos++;
	}
	else if (nota[pos] == 'b') {
		semitono -= 1;
		pos++;
	}

	int octava = std::stoi(nota.substr(pos));
	int midi = (octava + 1) * 12 + semitono;
	return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

// Recibe un texto como "C4 E4 G4 R C5" ("R" = silencio) y genera la melodía
std::vector<double> generarMelodia(const std::string& texto, Onda tipo,
	double duracionNota, uint32_t sampleRate, double amplitud, const Envolvente& env) {

	std::vector<double> melodia;
	std::istringstream entrada(texto);
	std::string nota;
	while (entrada >> nota) {
		std::vector<double> parte;
		if (nota == "R") {
			parte.assign(static_cast<size_t>(sampleRate * duracionNota), 0.0);
		}
		else {
			parte = generarOnda(tipo, frecuenciaDeNota(nota), duracionNota, sampleRate, amplitud, env);
		}
		melodia.insert(melodia.end(), parte.begin(), parte.end());
	}

	return melodia;
}

int main() {
	#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);   // lo que el programa muestra
		SetConsoleCP(CP_UTF8);         // lo que el usuario escribe
	#endif
	 
	const int sampleRate = 44100; // muestras por segundo
	const Envolvente envPorDefecto = { 0.05, 0.1, 0.7, 0.2 };

	while (true) {
		std::cout << "\n=== SINTETIZADOR POR CONSOLA ===\n--------------------------\n";
		std::cout << "\nMenú de opciones\n";
		std::cout << "[1] Generar un tono\n";
		std::cout << "[2] Generar una melodía\n";
		std::cout << "[0] Salir\n";
		std::cout << "[*] Opción --> ";
		int opcion, usarEnvolvente;
		std::cin >> opcion;

		if (opcion == 0) {
			std::cout << "\nAdieu! Bye Bye! Aufwiedersehen!\n\n";
			break;
		}

		if (opcion == 1) {
			double frecuencia, duracion, amplitud;
			std::string ruta;
			int tipo;

			std::cout << "\nFrecuencia (Hz): ";
			std::cin >> frecuencia;
			std::cout << "Duracion: (segundos): ";
			std::cin >> duracion;
			std::cout << "Amplitud: (0.0 a 1.0): ";
			std::cin >> amplitud;
			std::cout << "Forma de onda: (1:Senoidal, 2:Cuadrada, 3:Sierra, 4:Triangular): ";
			std::cin >> tipo;
			std::cout << "Usar envolvente ADSR (1: Si, 0: No): ";
			std::cin >> usarEnvolvente;
			std::cout << "Nombre del archivo (ej: tono.wav): ";
			std::getline(std::cin >> std::ws, ruta);

			Envolvente env = { 0.0, 0.1, 1.0, 0.0 }; // sin envolvente, volumen siempre al máximo
			if (usarEnvolvente == 1) {
				env = { 0.05, 0.1, 0.7, 0.2 }; // ataque, decay, sustain, release
			}

			std::vector<double> onda = generarOnda(static_cast<Onda>(tipo - 1), frecuencia, duracion, sampleRate, amplitud, env);

			guardarWav(ruta, convertirA16(onda), sampleRate);
			std::cout << "\n" << ruta << " generado con éxito\n";
		}

		if (opcion == 2) {
			int tipo;
			double duracionNota, amplitud;
			std::string texto, ruta;

			std::cout << "Forma de onda (1: Senoidal | 2: Cuadrada | 3: Sierra | 4: Rectangular): ";
			std::cin >> tipo;
			std::cout << "Duración de cada nota (segundos): ";
			std::cin >> duracionNota;
			std::cout << "Amplitud (0.0 a 1.0): ";
			std::cin >> amplitud;
			std::cout << "Melodía (ej: C4 E4 G4 R C5): ";
			std::getline(std::cin >> std::ws, texto);
			std::cout << "Nombre del archivo (ej: melodia.wav): ";
			std::cin >> ruta;

			std::vector<double> onda = generarMelodia(texto, static_cast<Onda>(tipo - 1), duracionNota, sampleRate, amplitud, envPorDefecto);
			guardarWav(ruta, convertirA16(onda), sampleRate);
			std::cout << "\n" << ruta << " generado con éxito ♫ ♪\n";
		}
	}

	return 0;
}