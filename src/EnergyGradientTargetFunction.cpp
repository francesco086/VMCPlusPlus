#include "vmc/EnergyGradientTargetFunction.hpp"

#include "vmc/EnergyGradientMCObservable.hpp"
#include "vmc/MPIVMC.hpp"

namespace vmc
{

nfm::NoisyValue EnergyGradientTargetFunction::f(const std::vector<double> &vp)
{
    // set the variational parameters given as input
    _wf->setVP(vp.data());
    // perform the integral and store the values
    double obs[4];
    double dobs[4];
    MPIVMC::Integrate(_mci, _E_Nmc, obs, dobs, true, true);
    nfm::NoisyValue f{obs[0], dobs[0]};

    if (_lambda_reg > 0.) { // compute the regularization term
        const double norm = std::inner_product(vp.begin(), vp.end(), vp.begin(), 0.);
        f.val += _lambda_reg*norm/_wf->getNVP();
    }
    
    return f;
}


void EnergyGradientTargetFunction::grad(const std::vector<double> &vp, nfm::NoisyGradient &grad)
{
    fgrad(vp, grad);
}


nfm::NoisyValue EnergyGradientTargetFunction::fgrad(const std::vector<double> &vp, nfm::NoisyGradient &grad)
{
    // set the variational parameters given as input
    _wf->setVP(vp.data());
    // add gradient obs to MCI
    const int blocksize = this->hasGradErr() ? 1 : 0;
    _mci->addObservable(EnergyGradientMCObservable(_wf, _H), blocksize, 1, false, blocksize>0); // skipping equlibiration for gradients
    // perform the integral and store the values
    double obs[4 + 2*_wf->getNVP()];
    double dobs[4 + 2*_wf->getNVP()];
    MPIVMC::Integrate(_mci, _grad_E_Nmc, obs, dobs, true, true);
    _mci->popObservable(); // remove the gradient obs (it will be deleted)
    // create pointers for ease of use and readability
    const double * const H = obs;
    const double * const dH = dobs;
    const double * const Oi = obs + 4;
    const double * const dOi = dobs + 4;
    const double * const HOi = obs + 4 + _wf->getNVP();
    const double * const dHOi = dobs + 4 + _wf->getNVP();
    nfm::NoisyValue f{H[0], dH[0]};
    // compute direction (or gradient) to follow
    for (int i = 0; i < _wf->getNVP(); ++i) {
        grad.val[i] = -2.*(HOi[i] - H[0]*Oi[i]);
    }
    if (this->hasGradErr()) {
        for (int i = 0; i < _wf->getNVP(); ++i) {
            grad.err[i] = 2.*(dHOi[i] + fabs(H[0]*Oi[i])*(dH[0]/H[0] + dOi[i]/Oi[i]));
        }
    }

    if (_lambda_reg > 0.) { // compute the regularization terms
        const double norm = std::inner_product(vp.begin(), vp.end(), vp.begin(), 0.);
        const double fac = _lambda_reg/_wf->getNVP();
        f.val += fac*norm;
        for (int i = 0; i < _wf->getNVP(); ++i) { grad.val[i] += 2.*fac*vp[i]; }
    }

    return f;
}
} // namespace vmc