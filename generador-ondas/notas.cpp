#include "notas.h"

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