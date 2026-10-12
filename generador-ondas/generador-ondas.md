# generador-ondas

Sintetizador básico en **C++17 sin librerías externas**. Genera ondas senoidales, cuadradas, de sierra y triangulares, les aplica una envolvente ADSR, toca melodías escritas como texto (`C4 E4 G4 R C5`) y exporta el resultado a archivos **WAV** (mono, PCM de 16 bits) que se pueden abrir en cualquier reproductor o DAW.

El archivo WAV se escribe byte por byte a mano, sin ninguna librería de audio.

## Características

- **Cuatro formas de onda:** senoidal, cuadrada, de sierra y triangular.
- **Envolvente ADSR** (ataque, decaimiento, sostenido y liberación) para evitar los "clicks" al empezar y terminar una nota.
- **Melodías a partir de texto:** notas con sostenidos y bemoles (`C4`, `F#3`, `Bb5`) y silencios (`R`).
- **Exportación a WAV** mono, 44100 Hz, 16 bits.
- **Validación de datos:** rechaza frecuencias fuera del límite de Nyquist, amplitudes inválidas, notas inexistentes y archivos que no se pueden crear, sin cerrarse.

## Ejemplo de uso

```
=== GENERADOR DE ONDAS ===
--------------------------

Menú de opciones
----------------
[1] Generar un tono
[2] Generar una melodía
[0] Salir
[*] Opción --> 1

Forma de onda
1: Senoidal | 2: Cuadrada | 3: Sierra | 4: Rectangular --> 1
Frecuencia (Hz) --> 440
Duracion (seg) --> 2
Amplitud (0.0 a 1.0) --> 0.3
Envolvente ADSR (1: Si | 0: No) --> 1
Nombre del archivo --> tono1

tono1.wav generado con éxito ♫♪

Menú de opciones
----------------
[1] Generar un tono
[2] Generar una melodía
[0] Salir
[*] Opción --> 0

Adieu! Bye Bye! Aufwiedersehen!
```

**Formato de las melodías:** notas separadas por espacios. La letra va de `A` a `G`, `#` sube un semitono y `b` lo baja, y el número es la octava (`C4` es el Do central, `A4` es 440 Hz). `R` es un silencio.

## Requisitos

- Un compilador con soporte de **C++17** (g++, clang o MSVC).
- No hay dependencias externas.

## Compilar y ejecutar

**Con g++:**

bash

```bash
g++ -std=c++17 -Wall -Wextra -o generador main.cpp wav.cpp envolvente.cpp oscilador.cpp notas.cpp
./generador          # en Windows: .\generador.exe
```

**Con Visual Studio:** abrí el proyecto, compilá con `Ctrl+Shift+B` y ejecutá con `Ctrl+F5`. Los `.wav` se guardan en la carpeta del proyecto.

## Estructura del proyecto

```
generador-ondas/
├── main.cpp          menú y entrada de datos
├── wav.h / .cpp         conversión a 16 bits y escritura del archivo WAV
├── oscilador.h / .cpp   formas de onda
├── envolvente.h / .cpp  envolvente ADSR
└── notas.h / .cpp       conversión de notas a frecuencia y armado de melodías
```

Cada módulo tiene una sola responsabilidad. Las funciones auxiliares que no se usan desde afuera (por ejemplo, la escritura de enteros en little-endian) quedan privadas dentro de su `.cpp`.

## Cómo funciona

### Generación de la onda

Cada forma de onda es una fórmula que recibe la **fase**, un número de 0 a 1 que indica en qué punto del ciclo está la onda. En cada muestra la fase avanza `frecuencia / sampleRate`, y al pasar de 1 vuelve a empezar.

| Forma      | Fórmula (fase de 0 a 1)   |
| ---------- | ------------------------- |
| Senoidal   | `sin(2π · fase)`          |
| Cuadrada   | `fase < 0.5 ? 1 : -1`     |
| Sierra     | `2 · fase − 1`            |
| Triangular | `1 − 4 · abs(fase − 0.5)` |

El resultado se multiplica por la amplitud y por el valor de la envolvente en ese instante.

### Archivo WAV

El archivo es un contenedor RIFF formado por *chunks*. Se escribe un encabezado de 44 bytes y después las muestras:

| Posición | Tamaño | Contenido                                   |
| :------- | :----- | ------------------------------------------- |
| 0        | 4      | `RIFF`                                      |
| 4        | 4      | tamaño del resto del archivo (`36 + datos`) |
| 8        | 4      | `WAVE`                                      |
| 12       | 4      | `fmt `                                      |
| 16       | 4      | tamaño del chunk `fmt ` (16)                |
| 20       | 2      | formato (1 = PCM)                           |
| 22       | 2      | canales (1)                                 |
| 24       | 4      | frecuencia de muestreo (44100)              |
| 28       | 4      | bytes por segundo                           |
| 32       | 2      | bytes por frame                             |
| 34       | 2      | bits por muestra (16)                       |
| 36       | 4      | `data`                                      |
| 40       | 4      | tamaño de las muestras en bytes             |
| 44       | …      | muestras (`int16_t`, little-endian)         |

Los enteros de más de un byte se escriben en **little-endian** (el byte menos significativo primero), armándolos byte a byte con desplazamientos de bits, sin depender de la arquitectura de la máquina.

### Notas y frecuencias

Cada nota se convierte a su número MIDI (`C4 = 60`, `A4 = 69`) y de ahí a su frecuencia:

```
frecuencia = 440 · 2 ^ ((midi − 69) / 12)
```

## Decisiones de diseño

- **Mono y 16 bits fijos.** Es el caso más simple que genera audio válido en cualquier reproductor, y permite concentrarse en el formato y en la síntesis.
- **Validaciones con excepciones**, atrapadas en `main`, para que un dato incorrecto muestre un mensaje y vuelva al menú en lugar de cerrar el programa.
- **Se recorta (\*clamp\*) en lugar de desbordar** cuando una muestra supera el rango de 16 bits.

## Limitaciones conocidas

- Las ondas **cuadrada y de sierra** se generan de forma directa, así que en frecuencias altas aparece **aliasing** (componentes que superan el límite de Nyquist y se "doblan" hacia abajo). Se podría reducir con técnicas como PolyBLEP.
- Solo genera audio mono de 16 bits.
- No hay reproducción en tiempo real: el programa guarda archivos y se escuchan con otro programa.

## Qué aprendí

- Cómo se organiza un archivo WAV (chunks y encabezado) y cómo escribirlo byte por byte, incluyendo el orden little-endian.
- Cómo se pasa de un valor continuo (`double` entre −1 y 1) a un entero de 16 bits, y qué pasa cuando se desborda el rango.
- Cómo se construyen las formas de onda básicas a partir de la fase, y qué es el aliasing.
- Cómo funciona una envolvente ADSR y por qué evita los clicks.
- Por qué conviene escribir primero el camino feliz y agregar las validaciones después, y cómo usar excepciones en C++.
- Cómo dividir un programa en módulos con `.h` y `.cpp`.

## Autor

**Juan Pablo Visintin** · Estudiante de Ingeniería en Sistemas de Información (UTN FRC) · [LinkedIn](https://www.linkedin.com/in/juan-pablo-visintin-64799a240/) · [GitHub](https://github.com/juanvisintin)