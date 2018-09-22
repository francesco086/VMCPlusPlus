#include "VMC.hpp"
#include "MPIVMC.hpp"

// --- compute quantities

void VMC::computeVariationalEnergy(const long & Nmc, double * E, double * dE){
    getMCI()->clearSamplingFunctions(); getMCI()->addSamplingFunction(_wf);
    getMCI()->clearObservables(); getMCI()->addObservable(_H);
    MPIVMC::Integrate(getMCI(), Nmc, E, dE, true, true);
}


// --- Optimization methods

void VMC::conjugateGradientOptimization(const long &E_Nmc, const long &grad_E_Nmc){
    ConjugateGradientOptimization * opt = new ConjugateGradientOptimization(_wf, _H, E_Nmc, grad_E_Nmc, getMCI());
    opt->optimizeWF();
    delete opt;
};

void VMC::stochasticReconfigurationOptimization(const long &Nmc, const bool flag_noisy){
    WFOptimization * opt;
    if (flag_noisy) opt = new NoisyStochasticReconfigurationOptimization(_wf, _H, Nmc, getMCI());
    else opt = new StochasticReconfigurationOptimization(_wf, _H, Nmc, getMCI());
    opt->optimizeWF();
    delete opt;
};

void VMC::simulatedAnnealingOptimization(const long &Nmc, const double &iota, const double &kappa, const double &lambda, gsl_siman_params_t &params){
    SimulatedAnnealingOptimization * opt = new SimulatedAnnealingOptimization(_wf, _H, Nmc, getMCI(), iota, kappa, lambda, params);
    opt->optimizeWF();
    delete opt;
};

void VMC::nmsimplexOptimization(const long &Nmc, const double &iota, const double &kappa, const double &lambda){
    NMSimplexOptimization * opt = new NMSimplexOptimization(_wf, _H, _mci, Nmc, iota, kappa, lambda);
    opt->optimizeWF();
    delete opt;
};
