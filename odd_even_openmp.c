#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void odd_even_sort_openmp(int arr[], int n, int num_threads) {
    int phase, i;

    #pragma omp parallel num_threads(num_threads) shared(phase, n, arr)
    {
        for (phase = 0; phase < n;) {
            if (phase % 2 == 0) {
                #pragma omp for schedule(dynamic) private(i)
                for (i = 1; i < n; i += 2) {
                    if (arr[i-1] > arr[i]) {
                        swap(&arr[i-1], &arr[i]);
                    }
                }
            } else {
                #pragma omp for schedule(dynamic) private(i)
                for (i = 1; i < n-1; i += 2) {
                    if (arr[i] > arr[i+1]) {
                        swap(&arr[i], &arr[i+1]);
                    }
                }
            }
            #pragma omp single
            {
                phase++;
            }
        }
    }
}

void print_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

int is_sorted(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        if (arr[i] > arr[i+1]) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <tamanho_array> <num_threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int *arr = malloc(n * sizeof(int));
    double media_tempos, inicio, fim;
    
    // Gerar array aleatório
    // generate_random_array(arr, n, 1000);
    // printf("Array original: ");
    // if (n <= 20) {
    //     print_array(arr, n);
    // }
    // else {
    //     print_array(arr, 20);
    //     printf("(exibindo apenas os 20 primeiros elementos)\n");
    // }
    double soma_tempos = 0.0;

    int rodadas = 3;
    for (int i = 0; i < rodadas; i++) {
        generate_random_array(arr, n, 1000);
        inicio = omp_get_wtime();
        odd_even_sort_openmp(arr, n, num_threads);
        fim = omp_get_wtime();
        soma_tempos += (fim - inicio);
    }

    media_tempos = soma_tempos / rodadas;

    char tempo_str[64];
    snprintf(tempo_str, sizeof(tempo_str), "%'.6f", media_tempos);
    // Substitui o ponto por vírgula
    for (char *p = tempo_str; *p; ++p) {
        if (*p == '.') {
            *p = ',';
            break;
        }
    }
    printf("Tempo médio de execução com %d threads: %s segundos\n", num_threads, tempo_str);

    // printf("Tempo de execução serial: %.6f segundos\n", tempo_total);

    // printf("Array ordenado: ");
    // if (n <= 20) {
    //     print_array(arr, n);
    // }
    // else {
    //     print_array(arr, 20);
    //     printf("(exibindo apenas os 20 primeiros elementos)\n");
    // }

    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");
    free(arr);
    return 0;
}
