#include "envolvente.h"

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