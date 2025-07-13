#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <stdint.h>


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

void odd_even_sort_openmp(int arr[], int buffer[], int n, int num_processors, int rank) {
    MPI_Barrier(MPI_COMM_WORLD);
    double comm_time = 0.0;
    int size = n / num_processors;
    size += size % 2 != 0;
    printf("size: %d\n", size);

    for (int phase = 0; phase < n; phase++) {
        double comm_inicio = MPI_Wtime();
        MPI_Scatter(&buffer[phase % 2 != 0], size, MPI_INT, arr, size, MPI_INT, 0, MPI_COMM_WORLD);
        // MPI_Barrier(MPI_COMM_WORLD);
        // printf("antes rank (%d): ", rank);
        // print_array(arr, size);
        // if (rank == 0) {
        //     print_array(buffer, n);
        // }
        double comm_fim = MPI_Wtime();

        comm_time += (comm_fim - comm_inicio);

        for (int i = 1; i < size; i += 2) {
            if (arr[i-1] > arr[i]) {
                swap(&arr[i-1], &arr[i]);
            }
        }

        // printf("depois rank (%d): ", rank);
        // print_array(arr, size);
        comm_inicio = MPI_Wtime();
        MPI_Gather(arr, size, MPI_INT, &buffer[phase % 2 != 0], size, MPI_INT, 0, MPI_COMM_WORLD);
        comm_fim = MPI_Wtime();
        // MPI_Barrier(MPI_COMM_WORLD);
        // if (rank == 0) {
        //     print_array(buffer, n);
        // }

        comm_time += (comm_fim - comm_inicio);
    }
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
    int *arr = malloc(n * sizeof(int) + sizeof(int));
    int *buffer = NULL;
    double tempo_total, inicio, fim;
    MPI_Init(&argc, &argv);
    
    int rank, num_processors;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
    
    if (rank == 0) {
        int size = n / num_processors;
        size += size % 2 != 0;

        int padded_n = size * num_processors + 1;
        buffer = malloc(padded_n * sizeof(int));
        generate_random_array(buffer, n, 1000);
        for (int i = 0; i < padded_n - n; i++) {
            buffer[n + i] = INT32_MAX;
        }

        printf("Array original: ");
        if (n <= 20) {
            print_array(buffer, n);
        }
        else {
            print_array(buffer, 20);
            printf("(exibindo apenas os 20 primeiros elementos)\n");
        }
    }
    
    inicio = MPI_Wtime();
    odd_even_sort_openmp(arr, buffer, n, num_processors, rank);
    fim = MPI_Wtime();

    tempo_total = fim - inicio;

    if (rank == 0) {
        // printf("Tempo Total (max): %.6f s\n", total_time_global);
        // printf("Tempo Comunicação (soma): %.6f s\n", comm_time_global);
        // printf("Overhead (aprox): %.2f%%\n", (comm_time_global/total_time_global)*100);
        printf("Tempo de execução serial: %.6f segundos\n", tempo_total);
        printf("Array está ordenado: %s\n", is_sorted(buffer, n) ? "Sim" : "Não");

        printf("Array Final: ");
        if (n <= 20) {
            print_array(buffer, n);
        }
        else {
            print_array(buffer, 20);
            printf("(exibindo apenas os 20 primeiros elementos)\n");
        }
    }

    free(arr);
    MPI_Finalize();
    return 0;
}
