#include"time.h"
#include"stdio.h"
#include"stdlib.h"
#include"stdint.h"
#include"x86intrin.h"

#ifndef RNGOFFSET
#define RNGOFFSET 0
#endif

#if NOVECTORIZE
#define OPT_DECORATION __attribute__((optnone))
#else
#define OPT_DECORATION
#endif

#ifndef NUMBER_T
#define NUMBER_T float64
#endif

typedef float float32;
typedef double float64;
typedef NUMBER_T number_t;

// Helpers for running the tests
uint64_t get_tick();
void prep_array(number_t* a, unsigned length, unsigned seed, number_t scale_factor);
void get_seeds(unsigned main_seed, unsigned* seeds, unsigned how_many);

// All optimizations within these functions are disabled with NOVECTORIZE set
OPT_DECORATION number_t axpy(const number_t* x, number_t a, unsigned length);
OPT_DECORATION number_t dot_product(const number_t* x, const number_t* y, unsigned length);
OPT_DECORATION void elementwise_multiply(const number_t* x, const number_t* y, number_t* z, unsigned length);

// Enum so that it's a compile-time constant
const unsigned STRIDE = 2;
enum { ARR_LENGTH = 256 };
const unsigned RUNS = 2000;
const number_t RAND_SCALE_FACTOR = 200;

// These are global to prevent clang from optimizing them out
// It's ugly but so is the rest of the code
__attribute__((used)) number_t y = 0.0;
__attribute__((used)) number_t p = 0.0;
__attribute__((used)) number_t c[ARR_LENGTH];

int main(/*int argc, const char** argv*/) {
    unsigned seeds[RUNS*2];
    const unsigned RNG_SEED = (unsigned)time(NULL) + RNGOFFSET;

    get_seeds(RNG_SEED, seeds, RUNS*2);

    printf("parameters: runs = %u, arr_length = %u, rng_seed = %u\n", RUNS, ARR_LENGTH, RNG_SEED);

    printf("running axpy\n");
    uint64_t ticks_total = 0;
    for(unsigned i = 0; i < RUNS; i++) {
        number_t arr[ARR_LENGTH];
        number_t coeff = 0.0;
        prep_array(arr, ARR_LENGTH, seeds[i], RAND_SCALE_FACTOR);
        prep_array(&coeff, 1, seeds[RUNS], RAND_SCALE_FACTOR);
        
        uint64_t tick_before = get_tick();
        y = axpy(arr, coeff, ARR_LENGTH);
        uint64_t tick_after = get_tick();
        uint64_t tick_diff = tick_after-tick_before;

        ticks_total += tick_diff;
    }
    printf("axpy: avg tick runtime: %f\n\n", ((double)ticks_total)/RUNS);

    printf("running dot product\n");
    ticks_total = 0;
    for(unsigned i = 0; i < RUNS; i++) {
        number_t a[ARR_LENGTH];
        number_t b[ARR_LENGTH];
        prep_array(a, ARR_LENGTH, seeds[i], RAND_SCALE_FACTOR);
        prep_array(b, ARR_LENGTH, seeds[i+RUNS], RAND_SCALE_FACTOR);
        
        uint64_t tick_before = get_tick();
        p = dot_product(a, b, ARR_LENGTH);
        uint64_t tick_after = get_tick();
        uint64_t tick_diff = tick_after-tick_before;

        ticks_total += tick_diff;
    }
    printf("dot product: avg tick runtime: %f\n\n", ((double)ticks_total)/RUNS);

    printf("running elementwise multiply\n");
    ticks_total = 0;
    for(unsigned i = 0; i < RUNS; i++) {
        number_t a[ARR_LENGTH];
        number_t b[ARR_LENGTH];
        prep_array(a, ARR_LENGTH, seeds[i], RAND_SCALE_FACTOR);
        prep_array(b, ARR_LENGTH, seeds[i+RUNS], RAND_SCALE_FACTOR);
        
        uint64_t tick_before = get_tick();
        elementwise_multiply(a, b, c, ARR_LENGTH);
        uint64_t tick_after = get_tick();
        uint64_t tick_diff = tick_after-tick_before;

        ticks_total += tick_diff;
    }
    printf("elementwise multiply: avg tick runtime: %f\n\n", ((double)ticks_total)/RUNS);
}

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

