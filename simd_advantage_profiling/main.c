#include"stdio.h"
#include"stdlib.h"
#include"stdint.h"
#include"x86intrin.h"

typedef float float32;
typedef double float64;

typedef float64 number_t;

const unsigned STRIDE = 2;

// Wrapper in case I want to change this later
uint64_t get_tick() {
    return __rdtsc();
}

// SAXPY / AXPY: y <- ax + y
number_t axpy(const number_t* x, number_t a, unsigned length) {
    number_t y = 0;
    for(unsigned s = 0; s < STRIDE; s++) {
        for(unsigned i = s; i < length; i += STRIDE) {
            y += a*x[i];
        }
    }
    return y;
}

// Dot product / reduction: s <- \sum x_i y_i
number_t dot_product(const number_t* x, const number_t* y, unsigned length) {
    number_t product = 0;
    for(unsigned s = 0; s < STRIDE; s++) {
        for(unsigned i = s; i < length; i += STRIDE) {
            product += x[i] * y[i];
        }
    }
    return product;
}

// Elementwise multiply: z_i <- x_i * y_i
void elementwise_multiply(const number_t* x, const number_t* y, number_t* z, unsigned length) {
    for(unsigned s = 0; s < STRIDE; s++) {
           for(unsigned i = s; i < length; i += STRIDE) {
            z[i] += x[i] * y[i];
        }
    }
}

// Fill array in with random numbers using the given seed
void prep_array(number_t* a, unsigned length, unsigned seed, number_t scale_factor) {
    srand(seed);
    for(unsigned i = 0; i < length; i++) {
        // from Stackoverflow, to generate random float
        a[i] = scale_factor*((number_t)rand() / (number_t)(RAND_MAX));
    }
}

// Get a bunch of random seeds using the main random seed
void get_seeds(unsigned main_seed, unsigned* seeds, unsigned how_many) {
    srand(main_seed);
    for(unsigned i = 0; i < how_many; i++) {
        seeds[i] = rand();
    }
}

int main(int argc, const char** argv) {
    const unsigned RNG_SEED = 1234567;
    const unsigned ARR_LENGTH = 16;
    const unsigned RUNS = 12;
    unsigned seeds[RUNS];

    get_seeds(RNG_SEED, seeds, RUNS);

    for(unsigned i = 0; i < RUNS; i++) {
        number_t arr[ARR_LENGTH];
        prep_array(arr, ARR_LENGTH, seeds[i], 200);
        
        uint64_t tick_before = get_tick();
        number_t y = axpy(arr, 12.3, ARR_LENGTH);
        uint64_t tick_after = get_tick();

        printf("Start tick: %lu\n", tick_before);
        printf("End tick: %lu\n", tick_after);
        printf("Elapsed time in ticks: %lu\n", tick_after-tick_before);

        printf("array: [ ");
        for(unsigned i = 0; i < ARR_LENGTH; i++) {
            printf("%.2f ", arr[i]);
        }
        printf("]\ny = %f\n", y);
    }
}
