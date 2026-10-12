#include <vector>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <limits>

#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
#endif

const double PI = 3.14159265358979323846;

enum class Onda { Senoidal, Cuadrada, Sierra, Triangular };

// Convierte un valor entre -1.0 y 1.0 a un entero de 16 bits
int16_t aEntero16(double x) {
	if (x > 1.0) x = 1.0;
	if (x < -1.0) x = -1.0;
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

void validarParametros(double frecuencia, double duracion, uint32_t sampleRate, double amplitud, const Envolvente& env) {
	if (frecuencia <= 0.0 || frecuencia >= sampleRate / 2.0) {
		throw std::invalid_argument("La frecuencia debe ser mayor que 0 y menor que " +
			std::to_string(sampleRate / 2) + "Hz (límite de Nyquist)");
	}

	if (duracion <= 0.0) {
		throw std::invalid_argument("La duración debe ser mayor que 0");
	}

	if (amplitud <= 0.0 || amplitud > 1.0) {
		throw std::invalid_argument("La amplitud debe estar entre 0 y 1");
	}

	if (env.ataque < 0.0 || env.decaimiento < 0.0 || env.liberacion < 0.0 || env.sustain < 0.0 || env.sustain > 1.0) {
		throw std::invalid_argument("Envolvente inválida: tiempos negativos o sustain fuera de 0 a 1");
	}

	if (env.ataque + env.decaimiento + env.liberacion > duracion) {
		throw std::invalid_argument("Ataque + decaimiento + liberacion no pueden superar la duración");
	}
}


std::vector<double> generarOnda(Onda tipo, double frecuencia, double duracion,
	uint32_t sampleRate, double amplitud, const Envolvente& env) {
	
	validarParametros(frecuencia, duracion, sampleRate, amplitud, env);

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
	
	if (nota.size() < 2) {
		throw std::invalid_argument("Nota inválida: '" + nota + "'");
	}
	
	int semitono = 0;
	switch (nota[0]) {
		case 'D': semitono = 2; break;
		case 'C': semitono = 0; break;
		case 'E': semitono = 4; break;
		case 'F': semitono = 5; break;
		case 'G': semitono = 7; break;
		case 'A': semitono = 9; break;
		case 'B': semitono = 11; break;
		default:
			throw std::invalid_argument("Nota inválida: '" + nota + "'");
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

	int octava = 0;
	try {
		octava = std::stoi(nota.substr(pos));
	}
	catch (const std::exception&) {
		throw std::invalid_argument("Nota inválida: '" + nota + "' (falta la octava)");
	}

	if (octava < 0 || octava > 8) {
		throw std::invalid_argument("La octava de '" + nota + "'debe estar entre 0 y 8");
	}
	int midi = (octava + 1) * 12 + semitono;
	return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

// Recibe un texto como "C4 E4 G4 R C5" ("R" = silencio) y genera la melodía
std::vector<double> generarMelodia(const std::string& texto, Onda tipo,
	double duracionNota, uint32_t sampleRate, double amplitud, const Envolvente& env) {

	if (duracionNota <= 0.0) {
		throw std::invalid_argument("La duración de cada nota debe ser mayor que 0");
	}

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

	if (melodia.empty()) {
		throw std::invalid_argument("La melodía está vacía");
	}

	return melodia;
}

// Pide un número y no avanza hasta que el usuario escriba uno válido
double leerNumero(const std::string& mensaje) {
	while (true) {
		std::cout << mensaje;
		double valor;
		if (std::cin >> valor) {
			return valor;
		}
		if (std::cin.eof()) {
			throw std::runtime_error("Se cerró la entrada de datos");
		}
		std::cin.clear(); // limpia el estado de error

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // descrarta lo escrito
		std::cout << " Error: entrada inválida. Ingresá un número, no cualquier verdura.\n";
	}
}

// Si el nombre no termina en ".wav", se lo agrega
std::string asegurarExtension(std::string ruta) {
	if (ruta.size() < 4 || ruta.substr(ruta.size() - 4) != ".wav") {
		ruta += ".wav";
	}

	return ruta;
}

Onda leerTipoDeOnda() {
	int tipo = static_cast<int>(leerNumero("Forma de onda\n1: Senoidal | 2: Cuadrada | 3: Sierra | 4: Rectangular --> "));
	if (tipo < 1 || tipo > 4) {
		throw std::invalid_argument("La forma de onda debe ser un número del 1 al 4");
	}

	return static_cast<Onda>(tipo - 1);
}

int main() {
	#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);   // lo que el programa muestra
		SetConsoleCP(CP_UTF8);         // lo que el usuario escribe
	#endif
	 
	const uint32_t sampleRate = 44100; // muestras por segundo
	const Envolvente envPorDefecto = { 0.05, 0.1, 0.7, 0.2 };

	std::cout << "\n=== SINTETIZADOR POR CONSOLA ===\n--------------------------------";

	while (true) {
		std::cout << "\n\nMenú de opciones\n----------------\n";
		std::cout << "[1] Generar un tono\n";
		std::cout << "[2] Generar una melodía\n";
		std::cout << "[0] Salir\n";
		int opcion, usarEnvolvente;

		try {
			int opcion = static_cast<int>(leerNumero("[*] Opción --> "));

			if (opcion == 0) {
				std::cout << "\nAdieu! Bye Bye! Aufwiedersehen!\n\n";
				break;
			}

			if (opcion == 1) {
				std::cout << "\n";
				Onda tipo = leerTipoDeOnda();
				double frecuencia = leerNumero("Frecuencia (Hz) --> ");
				double duracion = leerNumero("Duracion (seg) --> ");
				double amplitud = leerNumero("Amplitud (0.0 a 1.0) --> ");
				int usarEnvolvente = static_cast<int>(leerNumero("Envolvente ADSR (1: Si | 0: No) --> "));
				std::string ruta;
				std::cout << "Nombre del archivo --> ";
				std::cin >> ruta;
				ruta = asegurarExtension(ruta);

				Envolvente env = { 0.0, 0.0, 1.0, 0.0 };
				if (usarEnvolvente == 1) {
					env = envPorDefecto;
				}

				std::vector<double> onda = generarOnda(tipo, frecuencia, duracion, sampleRate, amplitud, env);
				guardarWav(ruta, convertirA16(onda), sampleRate);
				std::cout << "\n" << ruta << " generado con éxito ♫♪";

			}
			else if (opcion == 2) {
				std::cout << "\n";
				Onda tipo = leerTipoDeOnda();
				double duracionNota = leerNumero("Duracion de cada nota (seg) --> ");
				double amplitud = leerNumero("Amplitud (0.0 a 1.0) --> ");
				std::string texto, ruta;
				std::cout << "Melodía --> ";
				std::getline(std::cin >> std::ws, texto);
				std::cout << "Nombre del archivo --> ";
				std::cin >> ruta;
				ruta = asegurarExtension(ruta);

				std::vector<double> onda = generarMelodia(texto, tipo, duracionNota, sampleRate, amplitud, envPorDefecto);
				guardarWav(ruta, convertirA16(onda), sampleRate);
				std::cout << "\n" << ruta << " generado con éxito ♫♪";

			}
			else {
				std::cout << "\nError: opción no válida.\n";
			}
		}
		catch (const std::exception& e) {
			std::cout << "Error: " << e.what() << "\n";
		}
	}
	return 0;
}