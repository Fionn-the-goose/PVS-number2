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

// ---------------------------------------------------------------------------
// Hauptprogramm

int main(int argc, char *argv[])
{
double start, end;
start = omp_get_wtime();

    float *v;                                                         // Feld                               
    int iter;                                                // Wiederholungen             

    if (argc != 2) {                                      // Benutzungshinweis
        printf ("Vector sorting\nUsage: %s <NumIter>\n", argv[0]); 
        return 0;
    }
    iter = atoi(argv[1]);                               
    v = (float *) calloc(NUM, sizeof(float));        // Speicher reservieren

    printf("Perform vector sorting %d times...\n", iter);
    for (int i = 0; i < iter; i++) {               // Wiederhole das Sortieren
        for (int j = 0; j < NUM; j++)      // Mit Zufallszahlen initialisieren
            v[j] = (float)rand();

        #pragma omp parallel num_threads(4)
        {
            quicksort(v, 0, NUM-1);                                  // Sortierung
        }
    }

end = omp_get_wtime();

    printf ("\nDone: %f. seconds taken.\n", end - start);
    return 0;
}