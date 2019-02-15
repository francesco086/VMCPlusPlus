#include "vmc/SymmetrizerWaveFunction.hpp"
#include "vmc/PairSymmetrizerWaveFunction.hpp"

#include <assert.h>
#include <math.h>
#include <iostream>
#include <random>


/*
  Trial Wave Function for N particles in 1 dimension, that uses parameters ai and b, with fixed ai and variational b.
  Psi  =  exp( -b * sum((xi-ai)^2) )
*/
class QuadrExponential1DNPOrbital: public WaveFunction{
protected:
    double * _a;
    double _b;
    double _bi; // inverse b

public:
    QuadrExponential1DNPOrbital(const int &npart, const double * a, const double &b):
    WaveFunction(1, npart, 1, 1, true, true, true), _b(b), _bi(1./b)
    {
        _a=new double[npart];
        for (int i=0; i<npart; ++i) _a[i] = a[i];
    }

    ~QuadrExponential1DNPOrbital()
    {
        delete [] _a;
    }

    void setVP(const double *in){
        _b=in[0];
    }

    void getVP(double *out){
        out[0]=_b;
    }

    void samplingFunction(const double *x, double *out)
    {
        *out = 0.;
        for (int i=0; i<_npart; ++i) {
            *out += (x[i]-_a[i])*(x[i]-_a[i]);
        }
        *out *= -2.*_b;
    }

    double getAcceptance(const double * protoold, const double * protonew)
    {
        return exp(protonew[0]-protoold[0]);
    }

    void computeAllDerivatives(const double *x)
    {
        if (hasVD1()){
            _setVD1DivByWF(0, 0);
        }
        for (int i=0; i<_npart; ++i) {
            _setD1DivByWF(i, -2.*_b*(x[i]-_a[i]));
            _setD2DivByWF(i, -2.*_b*(1. + (x[i]-_a[i]) * getD1DivByWF(i)));
            if (hasVD1()){
                _setVD1DivByWF(0, getVD1DivByWF(0) - (x[i]-_a[i])*(x[i]-_a[i]));
            }
        }

        for (int i=0; i<_npart; ++i) {
            if (hasD1VD1()){
                _setD1VD1DivByWF(i, 0, getD1DivByWF(i) * (_bi + getVD1DivByWF(0)));
            }
            if (hasD2VD1()){
                _setD2VD1DivByWF(i, 0, getD2DivByWF(i) * (_bi + getVD1DivByWF(0)) + _bi * getD1DivByWF(i)*getD1DivByWF(i));
            }
        }
    }

    double computeWFValue(const double * protovalues)
    {
        return exp(0.5*protovalues[0]);
    }
};


int main(){
    using namespace std;

    // constants
    const int NSPACEDIM = 1;
    const int NPART = 3;
    const int NTOTALDIM = NPART*NSPACEDIM; 
    const double GAUSS_EXPF = 0.9;
    const double DX = 0.0001;
    const double TINY = 0.02;
    const double SUPERTINY = 0.0000001;

    // random generator
    random_device rdev;
    mt19937_64 rgen;
    uniform_real_distribution<double> rd;
    rgen = mt19937_64(rdev());
    rgen.seed(18984687);
    rd = uniform_real_distribution<double>(-0.05, 0.05);

    // define a non-symmetric wavefunction
    const double ai_nosym[NPART] = {0.5, -0.25, 0.0};
    QuadrExponential1DNPOrbital * phi_nosym = new QuadrExponential1DNPOrbital(NPART, ai_nosym, GAUSS_EXPF);

    // fully (anti-)symmetrized wfs
    SymmetrizerWaveFunction * phi_sym = new SymmetrizerWaveFunction(phi_nosym, false);
    SymmetrizerWaveFunction * phi_asym = new SymmetrizerWaveFunction(phi_nosym, true); // antisymmetric version

    // pair-wise (anti-)symmetrized wfs
    PairSymmetrizerWaveFunction * phi_psym = new PairSymmetrizerWaveFunction(phi_nosym, false);
    PairSymmetrizerWaveFunction * phi_pasym = new PairSymmetrizerWaveFunction(phi_nosym, true);

    // particles position and all permutations
    double ** xp = new double * [6];
    for (int i=0; i<6; ++i) xp[i] = new double[NTOTALDIM];
    xp[0][0] = 0.2; xp[0][1] = -0.5; xp[0][2] = 0.7;
    xp[1][0] = xp[0][0]; xp[1][1] = xp[0][2]; xp[1][2] = xp[0][1];
    xp[2][0] = xp[0][1]; xp[2][1] = xp[0][0]; xp[2][2] = xp[0][2];
    xp[3][0] = xp[0][1]; xp[3][1] = xp[0][2]; xp[3][2] = xp[0][0];
    xp[4][0] = xp[0][2]; xp[4][1] = xp[0][0]; xp[4][2] = xp[0][1];
    xp[5][0] = xp[0][2]; xp[5][1] = xp[0][1]; xp[5][2] = xp[0][0];

    bool isOdd[6] = {false, true, true, false, false, true};

    // compute the new protovalues
    double protov_nosym[2], protov_sym[2], protov_asym[2], protov_psym[2], protov_pasym[2];
    
    for (int i=0; i<6; ++i) {
        size_t offset = (i==0 ? 0 : 1);

        phi_nosym->samplingFunction(xp[i], protov_nosym + offset);
        phi_sym->samplingFunction(xp[i], protov_sym + offset);
        phi_asym->samplingFunction(xp[i], protov_asym + offset);
        phi_psym->samplingFunction(xp[i], protov_psym + offset);
        phi_pasym->samplingFunction(xp[i], protov_pasym + offset);

        // cout << "Perm " << i << ": ";
        // cout << " phi_nosym " << phi_nosym->computeWFValue(protov_nosym + offset);
        // cout << " phi_sym " << phi_sym->computeWFValue(protov_sym + offset);
        // cout << " phi_asym " << phi_asym->computeWFValue(protov_asym + offset) << endl;
        // cout << " phi_psym " << phi_psym->computeWFValue(protov_psym + offset);
        // cout << " phi_pasym " << phi_pasym->computeWFValue(protov_pasym + offset) << endl;

        if (i>0) {
            assert(protov_nosym[0] != protov_nosym[1]);
            if (isOdd[i]) assert(abs(protov_asym[0] + protov_asym[1]) < SUPERTINY);
            else assert(abs(protov_asym[0] - protov_asym[1]) < SUPERTINY);
            assert(abs(protov_sym[0] - protov_sym[1]) < SUPERTINY);
        } // we can't do asserts like this for the approximately symmetrized wfs
    }

    std::vector<WaveFunction *> wfs;
    wfs.push_back(phi_nosym);
    wfs.push_back(phi_sym);
    wfs.push_back(phi_asym);
    wfs.push_back(phi_psym);
    wfs.push_back(phi_pasym);

    vector<string> names {"phi_nosym", "phi_sym", "phi_asym", "phi_psym", "phi_pasym"};

    double x[NTOTALDIM];
    for (int i=0; i<NTOTALDIM; ++i) x[i] = xp[0][i];

    double vp[1];
    vp[0] = GAUSS_EXPF;
    int cont = 0;
    for (WaveFunction * wf : wfs) {
        // cout << "Checking " << names[cont] << " ..." << endl;
        
        // verify variational parameter
        double wfvp;
        wf->getVP(&wfvp);
        assert(vp[0] == wfvp);

        // pre-compute all the derivatives analytically
        wf->computeAllDerivatives(xp[0]);

        // initial wave function
        double f, fdx, fmdx, fdvp, fdxdvp, fmdxdvp;
        double samp;
        wf->samplingFunction(x, &samp); f = wf->computeWFValue(&samp);


        // --- check the first derivatives
        for (int i=0; i<NTOTALDIM; ++i){
            const double origx = x[i];
            x[i] += DX;
            wf->samplingFunction(x, &samp); fdx = wf->computeWFValue(&samp);
            const double numderiv = (fdx-f)/(DX*f);

            // cout << "getD1DivByWF(" << i <<") = " << wf->getD1DivByWF(i) << endl;
            // cout << "numderiv = " << numderiv << endl << endl;
            assert( abs( (wf->getD1DivByWF(i) - numderiv)/numderiv) < TINY );

            x[i] = origx;
        }


        // --- check the second derivatives
        for (int i=0; i<NTOTALDIM; ++i){
            const double origx = x[i];
            x[i] = origx + DX;
            wf->samplingFunction(x, &samp); fdx = wf->computeWFValue(&samp);
            x[i] = origx - DX;
            wf->samplingFunction(x, &samp); fmdx = wf->computeWFValue(&samp);
            const double numderiv = (fdx - 2.*f + fmdx) / (DX*DX*f);

            // cout << "getD2DivByWF(" << i << ") = " << wf->getD2DivByWF(i) << endl;
            // cout << "numderiv = " << numderiv << endl << endl;
            assert( abs( (wf->getD2DivByWF(i) - numderiv)/numderiv) < TINY );

            x[i] = origx;
        }


        // -- check the first variational derivative
        for (int i=0; i<wf->getNVP(); ++i){
            const double origvp = vp[i];
            vp[i] += DX;
            wf->setVP(vp);
            wf->samplingFunction(x, &samp); fdvp = wf->computeWFValue(&samp);
            const double numderiv = (fdvp - f)/(DX*f);

            // cout << "getVD1DivByWF(" << i << ") = " << wf->getVD1DivByWF(i) << endl;
            // cout << "numderiv = " << numderiv << endl << endl;
            assert( abs( (wf->getVD1DivByWF(i) - numderiv)/numderiv ) < TINY );

            vp[i] = origvp;
            wf->setVP(vp);
        }


        // --- check the first cross derivative
        for (int i=0; i<NTOTALDIM; ++i){
            for (int j=0; j<wf->getNVP(); ++j){
                const double origx = x[i];
                const double origvp = vp[j];

                x[i] += DX;
                wf->samplingFunction(x, &samp); fdx = wf->computeWFValue(&samp);

                x[i] = origx;
                vp[j] += DX;
                wf->setVP(vp);
                wf->samplingFunction(x, &samp); fdvp = wf->computeWFValue(&samp);

                x[i] += DX;
                wf->samplingFunction(x, &samp); fdxdvp = wf->computeWFValue(&samp);

                const double numderiv = (fdxdvp - fdx - fdvp + f)/(DX*DX*f);

                // cout << "getD1VD1DivByWF(" << i << ", " << j << ") = " << wf->getD1VD1DivByWF(i, j) << endl;
                // cout << "numderiv = " << numderiv << endl << endl;
                assert( abs( (wf->getD1VD1DivByWF(i, j)-numderiv)/numderiv ) < TINY );

                x[i] = origx;
                vp[j] = origvp;
                wf->setVP(vp);
            }
        }


        // --- check the second cross derivative
        for (int i=0; i<NTOTALDIM; ++i){
            for (int j=0; j<wf->getNVP(); ++j){
                const double origx = x[i];
                const double origvp = vp[j];

                x[i] = origx + DX;
                wf->samplingFunction(x, &samp); fdx = wf->computeWFValue(&samp);

                vp[j] = origvp + DX;
                wf->setVP(vp);
                wf->samplingFunction(x, &samp); fdxdvp = wf->computeWFValue(&samp);

                x[i] = origx;
                wf->samplingFunction(x, &samp); fdvp = wf->computeWFValue(&samp);

                x[i] = origx - DX;
                wf->samplingFunction(x, &samp); fmdxdvp = wf->computeWFValue(&samp);

                vp[j] = origvp;
                wf->setVP(vp);
                wf->samplingFunction(x, &samp); fmdx = wf->computeWFValue(&samp);

                const double numderiv = (fdxdvp - 2.*fdvp + fmdxdvp - fdx + 2.*f - fmdx)/(DX*DX*DX*f);

                // cout << "getD2VD1DivByWF(" << i << ", " << j << ") = " << wf->getD2VD1DivByWF(i, j) << endl;
                // cout << "numderiv = " << numderiv << endl << endl;
                assert( abs( (wf->getD2VD1DivByWF(i, j)-numderiv)/numderiv ) < TINY );

                x[i] = origx;
                vp[j] = origvp;
                wf->setVP(vp);
            }
        }
        ++cont;
    }

    for (int i=0; i<6; ++i) delete [] xp[i];
    delete [] xp;
    
    delete phi_pasym;
    delete phi_psym;

    delete phi_asym;
    delete phi_sym;

    delete phi_nosym;

    return 0;
}