import itertools
import subprocess

CFLAGS = "-O2 -g3 -Wall -Wextra -ffast-math".split()

MACRO_FLAGS = {"NOVECTORIZE": ["0", "1"], "NUMBER_T": ["float32", "float64"]}
MACRO_FLAGS_LIST = [[f"{flag}={v}" for v in vals] for flag,vals in MACRO_FLAGS.items()]
MACRO_FLAGS_PROD = list(itertools.product(*MACRO_FLAGS_LIST))

def compile(flags, outfilename):
    print(f"Compiling {outfilename} with flags {flags}")
    subprocess.run(["clang"] + CFLAGS + [f"-D{flag}" for flag in flags] + ["main.c", "-o", outfilename])

def main():
    for flagset in MACRO_FLAGS_PROD:
        s = "_".join(flagset)
        compile(list(flagset), f"main-{s}.out")

main()

"""
echo "COMPILING NO OPTIMIZE"
clang $CFLAGS -DNOVECTORIZE -DRNGOFFSET=100 main.c -o main-no-optimize.out

echo "COMPILING DEFAULT"
clang $CFLAGS main.c -o main.out

echo "NO OPTIMIZE"
./main-no-optimize.out

echo "DEFAULT"
./main.out
"""
