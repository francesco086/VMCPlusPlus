#ifndef NOISY_STOCHASTIC_RECONFIGURATION_OPTIMIZATION
#define NOISY_STOCHASTIC_RECONFIGURATION_OPTIMIZATION

#include "vmc/WFOptimization.hpp"
#include "vmc/NoisyStochasticReconfigurationTargetFunction.hpp"
#include "nfm/DynamicDescent.hpp"

#include "mci/MCIntegrator.hpp"



class NoisyStochasticReconfigurationOptimization: public WFOptimization{

private:
    long _Nmc;
    double _stepSize;

public:
    NoisyStochasticReconfigurationOptimization(WaveFunction * wf, Hamiltonian * H, const long &Nmc, MCI * mci, const double stepSize = 1.): WFOptimization(wf, H, mci){
        _Nmc = Nmc;
        _stepSize = stepSize;
    }
    virtual ~NoisyStochasticReconfigurationOptimization(){}

    // optimization
    void optimizeWF(){
        // create targetfunction
        NoisyStochasticReconfigurationTargetFunction * targetf = new NoisyStochasticReconfigurationTargetFunction(_wf, _H, getMCI(), _Nmc, 0., true);
        // declare the Dynamic Descent object
        DynamicDescent ddesc(targetf, _stepSize);
        // allocate an array that will contain the wave function variational parameters
        double wfpar[_wf->getNVP()];
        // get the variational parameters
        _wf->getVP(wfpar);
        // set the actual variational parameters as starting point for the Conjugate Gradient algorithm
        ddesc.setX(wfpar);
        // find the optimal parameters by minimizing the energy with the Conjugate Gradient algorithm
        ddesc.findMin();
        // set the found parameters in the wave function
        ddesc.getX(wfpar);
        _wf->setVP(wfpar);

        delete targetf;
    }
};



#endif
