#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

// Lista exhaustiva de vocales occidentales con acentos, diéresis, circunflejos, etc.
const wchar_t VOCALES_OCCIDENTALES[] = L"aeiouAEIOU"
                                       L"áéíóúÁÉÍÓÚüÜ" // Español, Catalán
                                       L"àèìòùÀÈÌÒÙ"   // Francés, Italiano, Portugués
                                       L"âêîôûÂÊÎÔÛ"   // Francés, Portugués
                                       L"äëöüÄËÖÜ"     // Alemán, Neerlandés
                                       L"ãõÃÕ"         // Portugués
                                       L"åÅæÆøØ";      // Escandinavo

void mostrar_ayuda(const char *nombre_programa) {
    printf("Uso: %s [OPCIONES] <ruta_del_archivo>\n\n", nombre_programa);
    printf("Opciones:\n");
    printf("  --help            Muestra este mensaje de ayuda y termina.\n");
    printf("  --barra_progreso  Muestra visualmente el avance en la terminal.\n\n");
}

int es_vocal_occidental(wchar_t c) {
    const wchar_t *p = VOCALES_OCCIDENTALES;
    while (*p) {
        if (*p == c) return 1;
        p++;
    }
    return 0;
}

int procesar_archivo(const char *ruta, int mostrar_barra) {
    // Abrir en modo lectura normal para que fgetwc procese UTF-8 correctamente
    FILE *file = fopen(ruta, "r"); 
    if (!file) {
        printf("Error: No se pudo abrir el archivo '%s'.\n", ruta);
        return 1;
    }

    // Calcular tamaño total aproximado en bytes para la barra de progreso
    fseek(file, 0, SEEK_END);
    long total_bytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (total_bytes == 0) {
        printf("El archivo '%s' está vacío.\n", ruta);
        fclose(file);
        return 0;
    }

    size_t capacidad = 10000;
    size_t total_elementos = 0;
    char *resultado_final = malloc(capacidad * sizeof(char));

    long bytes_leidos = 0;
    int ultimo_porcentaje = -1;
    wint_t c;

    // fgetwc lee caracteres UTF-8 completos (alfabetos occidentales) automáticamente
    while ((c = fgetwc(file)) != WEOF) {
        // En flujos de caracteres anchos, ftell nos da la posición en bytes del archivo
        bytes_leidos = ftell(file); 

        // iswalpha detecta de forma nativa si es una letra en cualquier idioma configurado
        if (iswalpha(c)) {
            if (total_elementos >= capacidad) {
                capacidad *= 2;
                resultado_final = realloc(resultado_final, capacidad * sizeof(char));
            }

            // Clasificar según nuestra lista extendida de vocales occidentales
            if (es_vocal_occidental(c)) {
                resultado_final[total_elementos++] = 1; // Vocal
            } else {
                resultado_final[total_elementos++] = 0; // Consonante (o caracteres como b, ç, ñ, ß)
            }
        }

        if (mostrar_barra) {
            int porcentaje = (int)((bytes_leidos * 100) / total_bytes);
            if (porcentaje > 100) porcentaje = 100; // Evitar desbordamientos por estimación de bytes
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
    printf("Total de letras occidentales procesadas: %zu\n", total_elementos);

    free(resultado_final);
    return 0;
}

int main(int argc, char *argv[]) {
    // CLAVE: Configurar el sistema para usar la codificación UTF-8 nativa de la máquina
    setlocale(LC_ALL, "");

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
