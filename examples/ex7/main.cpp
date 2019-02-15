#include <iostream>
#include <cmath>
#include <stdexcept>

#include "vmc/WaveFunction.hpp"
#include "vmc/Hamiltonian.hpp"
#include "vmc/VMC.hpp"
#include "nfm/ConjGrad.hpp"

#include "vmc/MPIVMC.hpp" // this example requires MPI!

/*
  Hamiltonian describing a 1-particle harmonic oscillator:
  H  =  p^2 / 2m  +  1/2 * w^2 * x^2
*/
class HarmonicOscillator1D1P: public Hamiltonian{

protected:
    double _w;

public:
    HarmonicOscillator1D1P(const double w, WaveFunction * wf):
        Hamiltonian(1 /*num space dimensions*/, 1 /*num particles*/, wf) {_w=w;}
    virtual ~HarmonicOscillator1D1P(){}

    // potential energy
    double localPotentialEnergy(const double *r)
    {
        return (0.5*_w*_w*(*r)*(*r));
    }
};



/*
  Trial Wave Function for 1 particle in 1 dimension, that uses one variational parameters: b.
  Psi  =  exp( -b * x^2 )
  Notice that the corresponding probability density (sampling function) is Psi^2.
*/
class Gaussian1D1POrbital: public WaveFunction
{
protected:
    double _b;

public:
    Gaussian1D1POrbital(const double b):
    WaveFunction(1, 1, 1, 1, false, false, false){
        _b=b;
    }

    void setVP(const double *in)
    {
        _b=*in;
        //if (_b<0.01) _b=0.01;
        using namespace std;
        //cout << "change b! " << _b << endl;
    }
    void getVP(double *out)
    {
        *out=_b;
    }

    void samplingFunction(const double *in, double *out)
    {
        *out=-2.*_b*(*in)*(*in);
    }

    double getAcceptance(const double * protoold, const double * protonew)
    {
        return exp(protonew[0]-protoold[0]);
    }

    void computeAllDerivatives(const double *in){
        _setD1DivByWF(0, -2.*_b*(*in));
        _setD2DivByWF(0, -2.*_b+4.*_b*_b*(*in)*(*in));
        if (hasVD1()){
            _setVD1DivByWF(0, (-(*in)*(*in)));
        }
    }

    double computeWFValue(const double * protovalues)
    {
        return exp(protovalues[0]);
    }
};



int main(){
    using namespace std;

    int myrank = MPIVMC::Init();
    cout << "Hello from rank " << myrank << endl;

    // Declare some trial wave functions
    Gaussian1D1POrbital psi(0.6);

    // Declare an Hamiltonian for each wave function
    // We use the harmonic oscillator with w=1
    HarmonicOscillator1D1P ham(1., &psi);


    const long E_NMC = 50000l; // MC samplings to use for computing the energy
    double energy[4]; // energy
    double d_energy[4]; // energy error bar

    VMC vmc(&psi, &ham);

    // example of file out with MPI
    auto obsfile = "obsfile" + std::to_string(myrank);
    auto wlkfile = "wlkfile" + std::to_string(myrank);;
    vmc.getMCI()->storeObservablesOnFile(obsfile.c_str(), 1);
    vmc.getMCI()->storeWalkerPositionsOnFile(wlkfile.c_str(), 1);

    if (myrank==0) cout << endl << " - - - EVALUATION OF ENERGY - - - " << endl << endl;

    // First compute the energy with auto-mode findMRT2step/initialDecorr/blocking
    if (myrank==0) cout << "Computing energy in parallel with auto-mode findMRT2step/initialDecorr (inconsistent time per CPU)." << endl;
    vmc.computeVariationalEnergy(E_NMC, energy, d_energy);
    if (myrank==0) {
        cout << "Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
        cout << "Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
        cout << "Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
        cout << "Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl;
    }

    // Now fix the number of steps for findMRT2step/initialDecorr
    vmc.getMCI()->setNfindMRT2steps(20);
    vmc.getMCI()->setNdecorrelationSteps(2000);
    if (myrank==0) cout << "Computing energy in parallel with fixed-mode findMRT2step/initialDecorr (consistent time per CPU)." << endl;
    vmc.computeVariationalEnergy(E_NMC, energy, d_energy);
    if (myrank==0) {
        cout << "Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
        cout << "Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
        cout << "Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
        cout << "Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl;
    }

    MPIVMC::Finalize();

    return 0;
}