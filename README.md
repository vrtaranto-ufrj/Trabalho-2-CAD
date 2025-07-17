# Trabalho-2-CAD
Odd-Even Transposition Sort Paralelo

Para compilar o código, utilize o comando:
```bash
make
```

Para executar o código serial:
```bash
./odd_even_serial <num-elementos>
```

Para executar o código paralelo OpenMP:
```bash
./odd_even_openmp <num-elementos> <num-threads>
```

Para executar o código paralelo MPI:
```bash
mpirun -np <num-processos> ./odd_even_mpi <num-elementos>
```

Ao rodar os códigos, eles irão executar o algoritmo 3 vezes, pois fui utilizado para obter os tempos para o relatório. Para o OpenMP, não usei o schadule para conseguir alterar ele em tempo de execução usando variáveis de ambiente, precisa alterar no código se quiser mudar o schedule.

Há trechos de código comentados que eram para printar o vetor inicialmente e no final já ordenado, juntamente com o print dizendo se o array realmente está alocado.