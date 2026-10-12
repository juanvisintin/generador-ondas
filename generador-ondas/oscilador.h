#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>
#include "envolvente.h"



enum class Onda { Senoidal, Cuadrada, Sierra, Triangular };

// Genera las muestras (entre -1.0 y 1.0) de una nota.
std::vector<double> generarOnda(Onda tipo, double frecuencia, double duracion,
	uint32_t sampleRate, double amplitud, const Envolvente& env);

namespace {
	const double PI = 3.14159265358979323846;

	// "fase" va de 0.0 a 1.0 y representa en qué punto del ciclo estamos
	double muestraDeOnda(Onda tipo, double fase);

	void validarParametros(double frecuencia, double duracion,
		uint32_t sampleRate, double amplitud,
		const Envolvente& env);
}