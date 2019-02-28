#include <cmath>
#include <gsl/gsl_siman.h>
#include <iostream>
#include <stdexcept>

#include "nfm/ConjGrad.hpp"
#include "nfm/LogNFM.hpp"
#include "vmc/Hamiltonian.hpp"
#include "vmc/MPIVMC.hpp"
#include "vmc/VMC.hpp"
#include "vmc/WaveFunction.hpp"


/*
  Hamiltonian describing a 1-particle harmonic oscillator:
  H  =  p^2 / 2m  +  1/2 * w^2 * x^2
*/
class HarmonicOscillator1D1P: public Hamiltonian
{
protected:
    double _w;

public:
    HarmonicOscillator1D1P(const double w, WaveFunction * wf):
        Hamiltonian(1 /*num space dimensions*/, 1 /*num particles*/, wf) {_w=w;}

    // potential energy
    double localPotentialEnergy(const double *r) override
    {
        return (0.5*_w*_w*(*r)*(*r));
    }
};



/*
  Trial Wave Function for 1 particle in 1 dimension, that uses two variational parameters: a and b.
  Psi  =  exp( -b * (x-a)^2 )
  Notice that the corresponding probability density (sampling function) is Psi^2.
*/
class QuadrExponential1D1POrbital: public WaveFunction{
protected:
    double _a, _b;

public:
    QuadrExponential1D1POrbital(const double a, const double b):
    WaveFunction(1 /*num space dimensions*/, 1 /*num particles*/, 1 /*num wf components*/, 2 /*num variational parameters*/, false /*VD1*/, false /*D1VD1*/, false /*D2VD1*/) {
            _a=a; _b=b;
        }

    void setVP(const double *in) override{
        _a=in[0];
        _b=in[1];
    }

    void getVP(double *out) override{
        out[0]=_a;
        out[1]=_b;
    }

    void samplingFunction(const double *x, double *out) override{
        /*
          Compute the sampling function proto value, used in getAcceptance()
        */
        *out = -2.*(_b*(x[0]-_a)*(x[0]-_a));
    }

    double getAcceptance(const double * protoold, const double * protonew) override{
        /*
          Compute the acceptance probability
        */
        return exp(protonew[0]-protoold[0]);
    }

    void computeAllDerivatives(const double *x) override{
        _setD1DivByWF(0, -2.*_b*(x[0]-_a));
        _setD2DivByWF(0, -2.*_b + (-2.*_b*(x[0]-_a))*(-2.*_b*(x[0]-_a)));
        if (hasVD1()){
            _setVD1DivByWF(0, 2.*_b*(x[0]-_a));
            _setVD1DivByWF(1, -(x[0]-_a)*(x[0]-_a));
        }
    }

    double computeWFValue(const double * protovalues) override{
        return exp(0.5*protovalues[0]);
    }
};




int main(){
    using namespace std;

    MPIVMC::Init(); // make this usable with a MPI-compiled library

    // Declare some trial wave functions
    auto * psi = new QuadrExponential1D1POrbital(-0.5, 1.0);

    // Declare an Hamiltonian
    // We use the harmonic oscillator with w=1 and w=2
    const double w = 1.;
    auto * ham = new HarmonicOscillator1D1P(w, psi);


    cout << endl << " - - - WAVE FUNCTION OPTIMIZATION - - - " << endl << endl;

    const int NMC = 10000l; // MC samplings to use for computing the energy
    double energy[4]; // energy
    double d_energy[4]; // energy error bar
    double vp[psi->getNVP()];



    VMC * vmc = new VMC(psi, ham);

    cout << "-> ham:    w = " << w << endl << endl;

    cout << "   Initial Wave Function parameters:" << endl;
    psi->getVP(vp);
    cout << "       a = " << vp[0] << endl;
    cout << "       b = " << vp[1] << endl;

    cout << "   Starting energy:" << endl;
    vmc->computeVariationalEnergy(NMC, energy, d_energy);
    cout << "       Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
    cout << "       Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
    cout << "       Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
    cout << "       Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl;

    cout << "   Optimization . . ." << endl;

    // settings for performance
    vmc->getMCI()->setNfindMRT2steps(10);
    vmc->getMCI()->setNdecorrelationSteps(1000);

    // simulated annealing parameters
    int N_TRIES = 20;
    int ITERS_FIXED_T = 20;
    double STEP_SIZE = 0.1;
    double K = 0.1;
    double T_INITIAL = 1.0;
    double MU_T = 1.3;
    double T_MIN = 0.01;
    gsl_siman_params_t params = {N_TRIES, ITERS_FIXED_T, STEP_SIZE, K, T_INITIAL, MU_T, T_MIN};

    vmc->simulatedAnnealingOptimization(NMC, 1., 0.1, 0., params);

    cout << "   . . . Done!" << endl << endl;

    cout << "   Optimized Wave Function parameters:" << endl;
    psi->getVP(vp);
    cout << "       a = " << vp[0] << endl;
    cout << "       b = " << vp[1] << endl;

    cout << "   Optimized energy:" << endl;
    vmc->computeVariationalEnergy(NMC, energy, d_energy);
    cout << "       Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
    cout << "       Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
    cout << "       Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
    cout << "       Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl << endl;


    delete vmc;
    delete ham;
    delete psi;


    MPIVMC::Finalize();

    return 0;
}
