import os
import itertools
import subprocess

# Fast math required for vectorization involving floats
# Rpass used to show that the vectorization did indeed occur
CFLAGS = "-O2 -g3 -Wall -Wextra -ffast-math -Rpass=loop-vectorize".split()

MACRO_FLAGS = {
    # With and without vectorization disabled
    "NOVECTORIZE": ["0", "1"],
    # Which number type to use
    "NUMBER_T": ["float32", "float64"],
    # How many iterations to run
    "_RUNS": ["2000"],
    # Length of the arrays. 512, 2048 are clean,
    #   521 and 1033 are deliberately not
    "_ARR_LENGTH": ["512", "2048", "521", "1033"],
    # Strided vs contiguous (1)
    "_STRIDE": ["1", "2", "4", "8"],
}
MACRO_FLAGS_LIST = [[f"{flag}={v}" for v in vals] for flag, vals in MACRO_FLAGS.items()]
MACRO_FLAGS_PROD = list(itertools.product(*MACRO_FLAGS_LIST))


# Compile the code with the flags
def compile(flags, outfilename):
    print(f"Compiling with flags {flags}")
    subprocess.run(
        ["clang"]
        + CFLAGS
        + [f"-D{flag}" for flag in flags]
        + ["main.c", "-o", f"out/{outfilename}"]
    )


# Compile and run
def main():
    for flagset in MACRO_FLAGS_PROD:
        s = "_".join(flagset)
        compile(list(flagset), f"main-{s}.out")
    for file in os.listdir("out"):
        print(file)
        subprocess.call(f"out/{file}")


if __name__ == "__main__":
    main()
