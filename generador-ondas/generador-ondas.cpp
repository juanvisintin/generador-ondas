#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "envolvente.h"
#include "notas.h"
#include "oscilador.h"
#include "wav.h"

#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
#endif

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