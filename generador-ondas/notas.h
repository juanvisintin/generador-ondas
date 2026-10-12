#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include "envolvente.h"
#include "oscilador.h"

// Convierte una nota como "C4", "F#3" o "Bb5" a su frecuencia en Hz
double frecuenciaDeNota(const std::string& nota);

// Recibe un texto como "C4 E4 G4 R C5" ("R" = silencio) y genera la melodía
std::vector<double> generarMelodia(const std::string& texto, Onda tipo,
	double duracionNota, uint32_t sampleRate, double amplitud, const Envolvente& env);

