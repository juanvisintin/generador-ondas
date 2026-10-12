#include <vector>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <algorithm>

const double PI = 3.14159265358979323846;

// Convierte un valor entre -1.0 y 1.0 a un entero de 16 bits
int16_t aEntero16(double x) {
	return static_cast<int16_t>(x * 32767.0);
}

int main() {
	
	const int sampleRate = 44100; // muestras por segundo
	const double frecuencia = 440.0; // Hz
	const double duracion = 1.0; // segundos

	int totalMuestras = static_cast<int>(sampleRate * duracion);
	std::vector<double> muestras;

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

	return 0;
}