#include "oscilador.h"

namespace {
	double muestraDeOnda(Onda tipo, double fase) {
		switch (tipo) {
		case Onda::Senoidal:	return std::sin(2.0 * PI * fase);
		case Onda::Cuadrada:	return fase < 0.5 ? 1.0 : -1.0;
		case Onda::Sierra:		return 2.0 * fase - 1.0;
		case Onda::Triangular:	return 1.0 - 4.0 * std::fabs(fase - 0.5);
		}
		return 0.0;
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