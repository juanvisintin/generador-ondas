#pragma once

struct Envolvente {
	double ataque; // segundos que tarda en subir de 0 a 1
	double decaimiento; // segundos que tarda en bajar de 1 al nivel sostenido
	double sustain; // nivel de sostenido (0.0 a 1.0)
	double liberacion; // segundos que tarda en bajar a 0 al final
};

// Devuelve el volumen (0.0 a 1.0) en el instante t de una nota de "duración" segundos
double valorEnvolvente(double t, double duracion, const Envolvente& e);