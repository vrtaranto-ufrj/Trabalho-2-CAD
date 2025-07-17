#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void print_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int compare_ints(const void* a, const void* b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

double odd_even_sort_mpi(int arr[], int arr_global[], int n, int size, int num_processos, int rank) {
    double comm_time = 0.0;
    int fronteira, trocas_globais;
    bool swap_feito = false;

    // Primeiramente rodamos o quicksort localmente para garantir que o array esteja ordenado antes de iniciar o algoritmo
    qsort(arr, size, sizeof(int), compare_ints);

    for (int phase = 0; phase < n; phase++) {
        swap_feito = false;

        double comm_inicio = MPI_Wtime();
        if (phase % 2 == 0) {
            // Em rodadas pares, o processo de rank par troca seu ultimo elemento com primeiro elemento do processo seguinte
            if (rank % 2 == 0 && rank != num_processos - 1) {
                MPI_Sendrecv(&arr[size - 1], 1, MPI_INT, rank + 1, 0,
                             &fronteira, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (arr[size - 1] > fronteira) {
                    arr[size - 1] = fronteira;
                    swap_feito = true;
                }
            } else if (rank % 2 != 0) {
                MPI_Sendrecv(&arr[0], 1, MPI_INT, rank - 1, 0,
                             &fronteira, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (arr[0] < fronteira) {
                    arr[0] = fronteira;
                    swap_feito = true;
                }
            }
        // Já em rodadas ímpares, o processo de rank ímpar troca seu último elemento com o primeiro elemento do processo seguinte
        } else {
            if (rank % 2 != 0 && rank != num_processos - 1) {
                MPI_Sendrecv(&arr[size - 1], 1, MPI_INT, rank + 1, 0,
                             &fronteira, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (arr[size - 1] > fronteira) {
                    arr[size - 1] = fronteira;
                    swap_feito = true;
                }
            } else if (rank % 2 == 0 && rank != 0) {
                MPI_Sendrecv(&arr[0], 1, MPI_INT, rank - 1, 0,
                             &fronteira, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (arr[0] < fronteira) {
                    arr[0] = fronteira;
                    swap_feito = true;
                }
            }
        }
        double comm_fim = MPI_Wtime();
        comm_time += (comm_fim - comm_inicio);

        // Se houve troca, realizamos uma ordenação local para garantir que o array esteja ordenado
        if (swap_feito) {
            // Como o array estava ordenado e agora houve uma troca, o array ficou "99%" ordenado, então podemos usar o insertion sort para ordenar rapidamente,
            // pois o insertion sort é eficiente para arrays quase ordenados
            // A complexidade do insertion sort é O(n^2) no pior caso, mas O(n) no melhor caso, que é o caso aqui, pois o array está quase ordenado
            for (int i = 1; i < size; i++) {
                int key = arr[i];
                int j = i - 1;
                while (j >= 0 && arr[j] > key) {
                    arr[j + 1] = arr[j];
                    j--;
                }
                arr[j + 1] = key;
            }
        }

        // Aqui checamos se houve trocas em todos os processos, e se não houve, o array está ordenado e podemos sair do loop
        MPI_Allreduce(&swap_feito, &trocas_globais, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if (trocas_globais == 0) {
            break; // Se não houve trocas, o array está ordenado
        }

        
    }
    // Unificamos os arrays locais em um array global no processo 0
    MPI_Gather(arr, size, MPI_INT, arr_global, size, MPI_INT, 0, MPI_COMM_WORLD);

    return comm_time;
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
    if (argc != 2) {
        printf("Uso: %s <tamanho_array>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    double tempo_total, inicio, fim;

    MPI_Init(&argc, &argv);
    
    int rank, num_processos;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processos);
    int size = n / num_processos;
    double soma_comm_time = 0.0, soma_global_time = 0.0;

    int *arr = NULL;
    int *arr_local = malloc(size * sizeof(int));

    if (rank == 0) {
        arr = malloc(n * sizeof(int));
    }
    
    int rodadas = 3; // Número de rodadas para média
    for (int r = 0; r < rodadas; r++) {
        if (rank == 0) {
            generate_random_array(arr, n, 1000);
        }
        // Distribui o array para todos os processos
        MPI_Scatter(arr, size, MPI_INT, arr_local, size, MPI_INT, 0, MPI_COMM_WORLD);

        
        inicio = MPI_Wtime();
        double comm_time = odd_even_sort_mpi(arr_local, arr, n, size, num_processos, rank);
        MPI_Barrier(MPI_COMM_WORLD);
        fim = MPI_Wtime();

        tempo_total = fim - inicio;
        double global_comm_time, global_tempo_total;
        MPI_Reduce(&comm_time, &global_comm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&tempo_total, &global_tempo_total, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            soma_comm_time += global_comm_time;
            soma_global_time += global_tempo_total;
        }
    }


    if (rank == 0) {
        printf("Tempo Total (max): %.6f s\n", soma_global_time);
        printf("Tempo Comunicação (soma): %.6f s\n", soma_comm_time);
        printf("Overhead (aprox): %.2f%%\n", (soma_comm_time/soma_global_time)*100);
        printf("Eficiencia (aprox): %.2f%%\n", (soma_global_time - soma_comm_time)/soma_global_time * 100);
        printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");
    }

    if (rank == 0) {
        free(arr);
    }
    free(arr_local);
    MPI_Finalize();
    return 0;
}
