#include <vector>
#include <cmath>
#include <iostream>

const double PI = 3.14159265358979323846;

int main() {
	const int sampleRate = 44100; // muestras por segundo
	const double frecuencia = 440.0; // Hz
	const double duracion = 1.0; // segundos

	int totalMuestras = static_cast<int>(sampleRate * duracion);
	std::vector<double> muestras;

	for (int i = 0; i < totalMuestras; ++i) {
		double t = static_cast<double>(i) / sampleRate; // instante en segundos
		double valor = std::sin(2.0 * PI * frecuencia * t); // entre -1.0 y 1.0
		muestras.push_back(valor);
	}

	std::cout << "Total de muestras: " << muestras.size() << "\n";
	for (int i = 0; i < 100; ++i) {
		std::cout << "Muestra " << i << ": " << muestras[i] << "\n";
	}

	return 0;
}