#include <stdio.h>
#include <stdlib.h>
#include <time.h>

size_t numBytes;
size_t numInts;

size_t cacheLineSizeL1 = 64;
size_t cacheLineSizeL1Ints;
size_t cacheSizeL2 = 256 * ( 1 << 10);

int *intArrayA;
int *intArrayB;
int *intArraySum;

void setup() {
    numBytes = cacheSizeL2 / 4;
    numInts = numBytes / sizeof( int);

    cacheLineSizeL1Ints = cacheLineSizeL1 / sizeof( int);
    intArrayA = calloc( numInts + cacheLineSizeL1Ints, sizeof( int));
    intArrayB = calloc( numInts + cacheLineSizeL1Ints, sizeof( int));
    intArraySum = calloc( numInts + cacheLineSizeL1Ints, sizeof( int));

    for ( int i = 0;  i < numInts + cacheLineSizeL1Ints;  i++) {
        intArrayA[ i] = i;
        intArrayB[ i] = i + 1;
    }
    printf( "numBytes=%ld, numInts=%ld, allocated=%ld\n", numBytes, numInts, numInts + cacheLineSizeL1Ints);
}

void runLoop( int offset) {
    int *summandA = intArrayA + offset;
    int *summandB = intArrayB + offset;
    int *sum = intArraySum + offset;

    for ( int i = 0;  i < numInts;  i++) {
        sum[ i] = summandA[ i] + summandB[ i];
    }

}

int calibrate( int targetSecs) {
    int runs = 0;
    clock_t start = clock();
    runLoop( 0);
    runs++;
    clock_t end = clock();
    if ( 1.0 * ( end - start) / CLOCKS_PER_SEC > targetSecs) {
        return runs;
    }
    int loops = 1;
    double durS;
    do {
        for ( int i = 0;  i < loops;  i++) {
            runLoop( 0);
            runs++;
        }
        end = clock();
        loops *= 2;
        durS = 1.0 * ( end - start) / CLOCKS_PER_SEC;
        // printf( "runs=%d loops=%d durS=%f\n", runs, loops, durS);
    } while ( durS < 1);

    start = clock();
    for ( int i = 0;  i < runs;  i++) {
            runLoop( 0);
    }
    end = clock();

    durS = 1.0 * ( end - start) / CLOCKS_PER_SEC;
    int result = runs / durS * targetSecs;
    return result;
}

void run() {
    int targetSeconds = 10;
    printf( "calibrated ");
    fflush( stdout);
    int loopCount = calibrate( targetSeconds);
    long additions = numInts * loopCount;
    printf( "to %d runs in %d s\n", loopCount, targetSeconds);

    for ( int offset = 0;  offset < cacheLineSizeL1Ints;  offset++) {
        clock_t start = clock();
        for ( int i = 0;  i < loopCount;  i++) {
            runLoop( offset);
        }
        clock_t end = clock();
        double durS = 1.0 * ( end - start) / CLOCKS_PER_SEC;
        printf( "performed %ld additions in %f s (%f ns/op) with offset %d\n",
			additions, durS, (1e9 * durS / additions), offset);
    }
}

int main()
{
	setup();
	run();
    return 0;
}
