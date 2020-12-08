//Comments and explanations are in the main and above the function names
//The examples were executed in the terminal with "g++ -fopenmp quicksort.cpp -o quicksort && ./quicksort 100"
//With 100 iterations, and 17 threads (seemed to be the sweet spot) all parallel examples were faster than the sequential variation.
//When we executed this it was by a factor of 1.134, 1.192, and 1.050 respectively

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUM 32767                                             // Elementanzahl

// ---------------------------------------------------------------------------
// Vertausche zwei Zahlen im Feld v an der Position i und j

void swap(float *v, int i, int j)
{
    float t = v[i]; 
    v[i] = v[j];
    v[j] = t;
}

// ---------------------------------------------------------------------------
// Serielle Version von Quicksort (Wirth) 

//This function was used to test if the quicksorts were successful, it is inexpensive and has little effect on the time comparisons
bool isSorted(float* v, int start, int end)
{
    float prev = 0.f;

    for (int i = start; i <= end; ++i)
    {
        if (v[i] < prev) return false;
        prev = v[i];
    }

    return true;
}

//This is the default version
void quicksort(float *v, int start, int end) 
{
    int i = start, j = end;
    float pivot;

    pivot = v[(start + end) / 2];                         // mittleres Element
    do {
        while (v[i] < pivot)
            i++;
        while (pivot < v[j])
            j--;
        if (i <= j) {               // wenn sich beide Indizes nicht beruehren
            swap(v, i, j);
            i++;
            j--;
        }
   } while (i <= j);
   if (start < j)                                        // Teile und herrsche
       quicksort(v, start, j);                      // Linkes Segment zerlegen
   if (i < end)
       quicksort(v, i, end);                       // Rechtes Segment zerlegen
}

//This version splits the recursive code into parallel sections, splitting the workload of every recursive call onto two threads, allowing every recursive call to be passed onto another thread, rather than being handled sequentially
void quicksortParallelSections(float* v, int start, int end)
{
    int i = start, j = end;
    float pivot;

    pivot = v[(start + end) / 2];                         // mittleres Element
    do {
        while (v[i] < pivot)
            i++;
        while (pivot < v[j])
            j--;
        if (i <= j) {               // wenn sich beide Indizes nicht beruehren
            swap(v, i, j);
            i++;
            j--;
        }
    } while (i <= j);

    #pragma omp parallel num_threads(17)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            if (start < j)                                        // Teile und herrsche
                quicksort(v, start, j);                      // Linkes Segment zerlegen
            #pragma omp section
            if (i < end)
                quicksort(v, i, end);                       // Rechtes Segment zerlegen
        }
    }
}

//This version splits the recursive code into tasks, producing almost the same effect as above, however the workload on tasks can be split across multiple threads and times, therefore with the right number of threads it is most efficient
void quicksortParallelTask(float* v, int start, int end)
{
    int i = start, j = end;
    float pivot;

    pivot = v[(start + end) / 2];                         // mittleres Element
    do {
        while (v[i] < pivot)
            i++;
        while (pivot < v[j])
            j--;
        if (i <= j) {               // wenn sich beide Indizes nicht beruehren
            swap(v, i, j);
            i++;
            j--;
        }
    } while (i <= j);

    #pragma omp parallel num_threads(17)
    {
        #pragma omp single
        {
            #pragma omp task
            if (start < j)                                        // Teile und herrsche
                quicksort(v, start, j);                      // Linkes Segment zerlegen
            #pragma omp task
            if (i < end)
                quicksort(v, i, end);
        }
    }
}

//This version uses a for loop combined with if statements to produce the same effect as the sections example
void quicksortParallelCursed(float* v, int start, int end)
{
    int i = start, j = end;
    float pivot;

    pivot = v[(start + end) / 2];                         // mittleres Element
    do {
        while (v[i] < pivot)
            i++;
        while (pivot < v[j])
            j--;
        if (i <= j) {               // wenn sich beide Indizes nicht beruehren
            swap(v, i, j);
            i++;
            j--;
        }
    } while (i <= j);

    #pragma omp parallel num_threads(17)
    {
        #pragma omp for
        for (int x = 0; x <= 1; ++x)
        {
            if (x == 0)
                if (start < j)                                        // Teile und herrsche
                    quicksort(v, start, j);                      // Linkes Segment zerlegen

            if (x == 1)
                if (i < end)
                    quicksort(v, i, end);
        }
    }
    }

// ---------------------------------------------------------------------------
// Hauptprogramm

int main(int argc, char *argv[])
{
    //Sequential
    double baseSpeed;
    {
        double start, end;
        start = omp_get_wtime();

        float* v;                                                         // Feld                               
        int iter;                                                // Wiederholungen             

        if (argc != 2) {                                      // Benutzungshinweis
            printf("\n\nVector sorting\nUsage: %s <NumIter>\n", argv[0]);
            return 0;
        }
        iter = atoi(argv[1]);
        v = (float*)calloc(NUM, sizeof(float));        // Speicher reservieren

        printf("\n\nPerform vector sorting %d times...\n", iter);
        for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
            for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
                v[j] = (float)rand();

            quicksort(v, 0, NUM - 1);                                  // Sortierung
            if (!isSorted(v, 0, NUM - 1)) printf("\n Failed: Not Sorted. \n"); //hurts runtime, but tests validity
        }

        end = omp_get_wtime();

        baseSpeed = end - start;
        printf("\nSequential Done: %f. seconds taken.\n", end - start);
    }

    //Parallel 1 : Sections
    {
        double start, end;
        start = omp_get_wtime();

        float* v;                                                         // Feld                               
        int iter;                                                // Wiederholungen             

        if (argc != 2) {                                      // Benutzungshinweis
            printf("\n\nVector sorting\nUsage: %s <NumIter>\n", argv[0]);
            return 0;
        }
        iter = atoi(argv[1]);
        v = (float*)calloc(NUM, sizeof(float));        // Speicher reservieren

        printf("\n\nPerform vector sorting %d times...\n", iter);
        for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
            for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
                v[j] = (float)rand();

            quicksortParallelSections(v, 0, NUM - 1);                                  // Sortierung
            if (!isSorted(v, 0, NUM - 1)) printf("\n Failed: Not Sorted. \n"); //hurts runtime, but tests validity
        }

        end = omp_get_wtime();

        printf("\nSections Done: %f. seconds taken.\n", end - start);
        printf("\nSpeedup: %f\n", baseSpeed / (end - start));
    }

    //Parallel 2 : Task
    {
        double start, end;
        start = omp_get_wtime();

        float* v;                                                         // Feld                               
        int iter;                                                // Wiederholungen             

        if (argc != 2) {                                      // Benutzungshinweis
            printf("\n\nVector sorting\nUsage: %s <NumIter>\n", argv[0]);
            return 0;
        }
        iter = atoi(argv[1]);
        v = (float*)calloc(NUM, sizeof(float));        // Speicher reservieren

        printf("\n\nPerform vector sorting %d times...\n", iter);
        for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
            for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
                v[j] = (float)rand();

            quicksortParallelTask(v, 0, NUM - 1);                                  // Sortierung
            if (!isSorted(v, 0, NUM - 1)) printf("\n Failed: Not Sorted. \n"); //hurts runtime, but tests validity
        }

        end = omp_get_wtime();

        printf("\nTasks Done: %f. seconds taken.\n", end - start);
        printf("\nSpeedup: %f\n", baseSpeed / (end - start));
    }


    //Parallel 3 : Cursed
    {
        double start, end;
        start = omp_get_wtime();

        float* v;                                                         // Feld                               
        int iter;                                                // Wiederholungen             

        if (argc != 2) {                                      // Benutzungshinweis
            printf("\n\nVector sorting\nUsage: %s <NumIter>\n", argv[0]);
            return 0;
        }
        iter = atoi(argv[1]);
        v = (float*)calloc(NUM, sizeof(float));        // Speicher reservieren

        printf("\n\nPerform vector sorting %d times...\n", iter);
        for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
            for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
                v[j] = (float)rand();

            quicksortParallelCursed(v, 0, NUM - 1);                                  // Sortierung
            if (!isSorted(v, 0, NUM - 1)) printf("\n Failed: Not Sorted. \n"); //hurts runtime, but tests validity
        }

        end = omp_get_wtime();

        printf("\nOh god this parallel version should not exist Done: %f. seconds taken.\n", end - start);
        printf("\nSpeedup: %f\n", baseSpeed / (end - start));
    }
}