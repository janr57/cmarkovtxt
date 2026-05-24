#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Comprueba si el Code Point Unicode es una vocal occidental
int es_vocal_unicode(uint32_t cp) {
    if (cp == 'a' || cp == 'e' || cp == 'i' || cp == 'o' || cp == 'u' ||
        cp == 'A' || cp == 'E' || cp == 'I' || cp == 'O' || cp == 'U') return 1;

    switch (cp) {
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x00C4: case 0x00C5: case 0x00C6:
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x00E4: case 0x00E5: case 0x00E6:
        case 0x00C8: case 0x00C9: case 0x00CA: case 0x00CB:
        case 0x00E8: case 0x00E9: case 0x00EA: case 0x00EB:
        case 0x00CC: case 0x00CD: case 0x00CE: case 0x00CF:
        case 0x00EC: case 0x00ED: case 0x00EE: case 0x00EF:
        case 0x00D2: case 0x00D3: case 0x00D4: case 0x00D5: case 0x00D6: case 0x00D8:
        case 0x00F2: case 0x00F3: case 0x00F4: case 0x00F5: case 0x00F6: case 0x00F8:
        case 0x00D9: case 0x00DA: case 0x00DB: case 0x00DC:
        case 0x00F9: case 0x00FA: case 0x00FB: case 0x00FC:
            return 1;
    }
    return 0;
}

// Comprueba si el Code Point Unicode es cualquier letra del alfabeto occidental
int es_letra_unicode(uint32_t cp) {
    if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z')) return 1;
    if (cp >= 0x00C0 && cp <= 0x00FF) {
        if (cp == 0x00D7 || cp == 0x00F7) return 0; // Excluye los signos × y ÷
        return 1;
    }
    if (cp >= 0x0100 && cp <= 0x017F) return 1;
    return 0;
}

void mostrar_ayuda(const char *nombre_programa) {
    printf("Uso: %s [OPCIONES] <ruta_del_archivo>\n\n", nombre_programa);
    printf("Opciones:\n");
    printf("  --help            Muestra este menú de ayuda.\n");
    printf("  --barra_progreso  Muestra la barra de avance en la terminal.\n");
}

int procesar_archivo(const char *ruta, int mostrar_barra) {
    printf("Abriendo archivo: '%s'\n", ruta);

    FILE *file = fopen(ruta, "rb"); 
    if (!file) {
        printf("Error: No se pudo abrir el archivo '%s'. Verifique la ruta.\n", ruta);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long total_bytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("Tamaño detectado: %ld bytes.\n", total_bytes);

    if (total_bytes == 0) {
        printf("El archivo está vacío.\n");
        fclose(file);
        return 0;
    }

    size_t capacidad = total_bytes; 
    size_t total_elementos = 0;
    char *resultado_final = malloc(capacidad * sizeof(char));

    long bytes_leidos = 0;
    int ultimo_porcentaje = -1;
    int c;

    // LEER BYTES CRUDOS UTILIZANDO MÁSCARAS DE BITS UTF-8 ESTÁNDAR
    while ((c = fgetc(file)) != EOF) {
        bytes_leidos++;
        
        uint32_t code_point = 0;
        int bytes_extra = 0;
        unsigned char b = (unsigned char)c; // Forzamos a que sea un byte sin signo (0-255)

        // Determinar cuántos bytes ocupa el carácter según los bits iniciales
        if (b <= 0x7F) {
            code_point = b; // ASCII estándar (1 byte)
            bytes_extra = 0;
        } else if ((b & 0xE0) == 0xC0) {
            code_point = b & 0x1F; // Inicio de carácter de 2 bytes (á, é, ñ...)
            bytes_extra = 1;
        } else if ((b & 0xF0) == 0xE0) {
            code_point = b & 0x0F; // Inicio de carácter de 3 bytes
            bytes_extra = 2;
        } else if ((b & 0xF8) == 0xF0) {
            code_point = b & 0x07; // Inicio de carácter de 4 bytes (Emojis...)
            bytes_extra = 3;
        } else {
            continue; // Byte de continuación suelto o inválido, lo ignoramos de forma segura
        }

        // Leer los bytes restantes que forman el carácter completo
        int lectura_valida = 1;
        for (int i = 0; i < bytes_extra; i++) {
            int siguiente = fgetc(file);
            if (siguiente == EOF) {
                lectura_valida = 0;
                break;
            }
            bytes_leidos++;
            unsigned char next_b = (unsigned char)siguiente;
            if ((next_b & 0xC0) != 0x80) { // Validación de estructura UTF-8
                lectura_valida = 0;
                break;
            }
            code_point = (code_point << 6) | (next_b & 0x3F);
        }

        // Si el carácter UTF-8 se reconstruyó con éxito, lo analizamos
        if (lectura_valida) {
            if (es_letra_unicode(code_point)) {
                if (total_elementos >= capacidad) {
                    capacidad *= 2;
                    resultado_final = realloc(resultado_final, capacidad * sizeof(char));
                }
                resultado_final[total_elementos++] = es_vocal_unicode(code_point) ? 1 : 0;
            }
        }

        // Barra de progreso
        if (mostrar_barra) {
            int porcentaje = (int)((bytes_leidos * 100) / total_bytes);
            if (porcentaje != ultimo_porcentaje) {
                ultimo_porcentaje = porcentaje;
                printf("\rProcesando texto: [%3d%%] [", porcentaje);
                int barras = porcentaje / 4; 
                for (int i = 0; i < 25; i++) {
                    if (i < barras) printf("=");
                    else printf(" ");
                }
                printf("]");
                fflush(stdout); 
            }
        }
    }

    fclose(file);
    if (mostrar_barra) printf("\n"); 
    
    printf("¡Proceso completado con éxito!\n");
    printf("Total de letras procesadas en el array: %zu\n", total_elementos);

    free(resultado_final);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Faltan argumentos.\n");
        mostrar_ayuda(argv[0]); 
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        mostrar_ayuda(argv[0]); 
        return 0;
    }

    if (strcmp(argv[1], "--barra_progreso") == 0) {
        if (argc < 3) {
            printf("Error: Debes especificar la ruta del archivo.\n");
            return 1;
        }
        return procesar_archivo(argv[2], 1); 
    }

    return procesar_archivo(argv[1], 0); 
}


