#include "vmc/ParticleArrayHelper.hpp"

#include <cassert>


int main()
{
    using namespace vmc;

    const int NPART = 3;
    const int NSPACEDIM = 3;
    const int RANDOMINT = 37;

    ParticleArrayHelper pah(NSPACEDIM);


    // create the initial multi-particle array
    double x[NPART*NSPACEDIM];
    for (int i = 0; i < NPART; ++i) {
        for (int j = 0; j < NSPACEDIM; ++j) {
            x[i*NPART + j] = RANDOMINT*i + j;
        }
    }



    // check getParticleArray
    double * vec;
    for (int i = 0; i < NPART; ++i) {
        vec = pah.getParticleArray(x, i);
        for (int j = 0; j < NSPACEDIM; ++j) {
            assert(vec[j] == x[i*NPART + j]);
            assert(x[i*NPART + j] == RANDOMINT*i + j);    // array x was not modified
        }
    }



    // check setParticleArray
    double newx[NSPACEDIM];
    for (int i = 0; i < NSPACEDIM; ++i) { newx[i] = 2*RANDOMINT*i; }

    double oldx[NSPACEDIM];
    for (int i = 0; i < NPART; ++i) {
        // store the original array values
        vec = pah.getParticleArray(x, i);
        for (int j = 0; j < NSPACEDIM; ++j) { oldx[j] = vec[j]; }

        // change the values for the particle i
        pah.setParticleArray(x, i, newx);
        // check that the values in the array x are as expected
        for (int i2 = 0; i2 < NPART; ++i2) {
            if (i2 == i) {
                // new values
                for (int j = 0; j < NSPACEDIM; ++j) {
                    assert(vec[j] == newx[j]);
                }
            }
            else {
                // old values
                for (int j = 0; j < NSPACEDIM; ++j) {
                    assert(x[i2*NPART + j] == RANDOMINT*i2 + j);
                }
            }
        }
        // set x to its original values
        pah.setParticleArray(x, i, oldx);
        // check that x is set back to the original values
        for (int i2 = 0; i2 < NPART; ++i2) {
            for (int j = 0; j < NSPACEDIM; ++j) {
                assert(x[i2*NPART + j] == RANDOMINT*i2 + j);
            }
        }
    }



    // check addArrayToParticleArray
    for (int i = 0; i < NPART; ++i) {
        // store the original array values
        vec = pah.getParticleArray(x, i);
        for (int j = 0; j < NSPACEDIM; ++j) { oldx[j] = vec[j]; }

        // change the values for the particle i
        pah.addArrayToParticleArray(x, i, newx);
        // check that the values in the array x are as expected
        for (int i2 = 0; i2 < NPART; ++i2) {
            if (i2 == i) {
                // new values
                for (int j = 0; j < NSPACEDIM; ++j) {
                    assert(vec[j] == oldx[j] + newx[j]);
                }
            }
            else {
                // old values
                for (int j = 0; j < NSPACEDIM; ++j) {
                    assert(x[i2*NPART + j] == RANDOMINT*i2 + j);
                }
            }
        }
        // set x to its original values
        for (double &j : newx) { j = -j; }

        pah.addArrayToParticleArray(x, i, newx);
        for (double &j : newx) { j = -j; }

        // check that x is set back to the original values
        for (int i2 = 0; i2 < NPART; ++i2) {
            for (int j = 0; j < NSPACEDIM; ++j) {
                assert(x[i2*NPART + j] == RANDOMINT*i2 + j);
            }
        }
    }

    return 0;
}
