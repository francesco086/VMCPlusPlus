#include "ShadowWaveFunction.hpp"

#include <cmath>


void ShadowWaveFunction::computeAllDerivatives(const double *x){
    const double div2NumSWFSampling = 1./(2.*_num_swf_sampling);
    const double two_div_tau = 2./_tau;
    const double invtau = 1./_tau;

    double * d1 = _getD1DivByWF();
    for (int i=0; i<getTotalNDim(); ++i){
        d1[i] = 0.;
        for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
            d1[i] -= two_div_tau * (x[i] - _s1[isampling][i]);
            d1[i] -= two_div_tau * (x[i] - _s2[isampling][i]);
        }
        d1[i] *= div2NumSWFSampling;
    }

    double * d2 = _getD2DivByWF();
    for (int i=0; i<getTotalNDim(); ++i){
        d2[i] = 0.;
        for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
            d2[i] += pow(two_div_tau * (x[i] - _s1[isampling][i]), 2);
            d2[i] += pow(two_div_tau * (x[i] - _s2[isampling][i]), 2);
        }
        d2[i] *= div2NumSWFSampling;
        d2[i] -= two_div_tau;
    }

    if (hasVD1()){
        double * vd1 = _getVD1DivByWF();
        // kernel component
        vd1[0] = 0.;
        for (int i=0; i<getTotalNDim(); ++i){
            for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
                vd1[0] += pow(x[i] - _s1[isampling][i], 2);
                vd1[0] += pow(x[i] - _s2[isampling][i], 2);
            }
        }
        vd1[0] = vd1[0]*div2NumSWFSampling * pow(invtau, 2);
        // pure shadow components
        for (int ivp=1; ivp<getNVP(); ++ivp){
            vd1[ivp] = 0.;   // initialize to zero
        }
        int contvp = 1;
        for (PureShadowWaveFunction * pswf : _pswfs){
            for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
                pswf->computeAllDerivatives(_s1[isampling]);
                for (int ivp=0; ivp<pswf->getNVP(); ++ivp){
                    vd1[ivp+contvp] += pswf->getVD1DivByWF(ivp);
                }
                pswf->computeAllDerivatives(_s2[isampling]);
                for (int ivp=0; ivp<pswf->getNVP(); ++ivp){
                    vd1[ivp+contvp] += pswf->getVD1DivByWF(ivp);
                }
            }
            contvp += pswf->getNVP();
        }
        for (int ivp=1; ivp<getNVP(); ++ivp){
            vd1[ivp] *= div2NumSWFSampling;   // divide by the number of samplings
        }
    }

    if (hasD1VD1()){
        double ** d1vd1 = _getD1VD1DivByWF();
        for (int i=0; i<getTotalNDim(); ++i){
            for (int ivp=0; ivp<getNVP(); ++ivp){
                d1vd1[i][ivp] = getD1DivByWF(i) * getVD1DivByWF(ivp);
            }
            d1vd1[i][0] -= getD1DivByWF(0)*invtau;
        }
    }

    if (hasD2VD1()){
        double ** d2vd1 = _getD2VD1DivByWF();
        for (int i=0; i<getTotalNDim(); ++i){
            for (int ivp=0; ivp<getNVP(); ++ivp){
                d2vd1[i][ivp] = getD2DivByWF(i) * getVD1DivByWF(ivp);
            }
            d2vd1[i][0] += invtau * (two_div_tau - 8.*getVD1DivByWF(0));
        }
    }
}



void ShadowWaveFunction::samplingFunction(const double * x, double * proto){
    using namespace std;
    // normal distribution
    normal_distribution<double> norm;
    const double sigma = sqrt(0.5*_tau);
    // sum of the pure shadow wave functions
    double sum_wf_s1 = 0.;
    double sum_wf_s2 = 0.;

    // sample and store the shadows (will be used for computing the derivatives)
    for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
        for (int i=0; i<getTotalNDim(); ++i){
            norm = normal_distribution<double>(x[i], sigma);
            _s1[isampling][i] = norm(_rgen);
            _s2[isampling][i] = norm(_rgen);
        }
    }

    // compute the pure shadow wf values and sum them up
    for (int isampling=0; isampling<_num_swf_sampling; ++isampling){
        for (PureShadowWaveFunction * pswf : _pswfs){
            sum_wf_s1 += pswf->value(_s1[isampling]);
            sum_wf_s2 += pswf->value(_s2[isampling]);
        }
    }

    proto[0] = sum_wf_s1 * sum_wf_s2;
}


double ShadowWaveFunction::getAcceptance(const double * protoold, const double * protonew){
    if (_pswfs.size() < 1){
        // if there are no pure shadow components, always accept the move
        return 1.;
    }

    return protonew[0]/protoold[0];
}



void ShadowWaveFunction::getVP(double *vp){
    vp[0] = _tau;
    int contvp = 1;
    for (PureShadowWaveFunction * pswf : _pswfs){
        pswf->getVP(vp+contvp);
        contvp += pswf->getNVP();
    }
}


void ShadowWaveFunction::setVP(const double *vp){
    _tau = vp[0];
    int contvp = 1;
    for (PureShadowWaveFunction * pswf : _pswfs){
        pswf->setVP(vp+contvp);
        contvp += pswf->getNVP();
    }
}



void ShadowWaveFunction::addPureShadowWaveFunction(PureShadowWaveFunction * pswf){
    if (pswf->getNSpaceDim() != getNSpaceDim()){
        throw std::invalid_argument( "Provided pure shadow wf's getNSpaceDim() is not valid" );
    }
    if (pswf->getNPart() != getNPart()){
        throw std::invalid_argument( "Provided pure shadow wf's getNPart() is not valid" );
    }
    if (pswf->hasVD1() != hasVD1()){
        throw std::invalid_argument( "Provided pure shadow wf's hasVD1() is not valid" );
    }
    _pswfs.push_back(pswf);
    setNVP( getNVP() + pswf->getNVP() );
}