#ifndef MULTI_COMPONENT_WAVE_FUNCTION
#define MULTI_COMPONENT_WAVE_FUNCTION


#include "WaveFunction.hpp"


#include <vector>
#include <stdexcept>



class MultiComponentWaveFunction: public WaveFunction{
private:
    std::vector<WaveFunction *> _wfs;

public:
    MultiComponentWaveFunction(const int &nspacedim, const int &npart, bool flag_vd1=true, bool flag_d1vd1=true, bool flag_d2vd1=true):
    WaveFunction(nspacedim, npart, 0, 0, flag_vd1, flag_d1vd1, flag_d2vd1){}

    void addWaveFunction(WaveFunction * wf){
        if (wf->getNSpaceDim() != getNSpaceDim()){
            throw std::invalid_argument( "Provided wf's getNSpaceDim() is not valid" );
        }
        if (wf->getNPart() != getNPart()){
            throw std::invalid_argument( "Provided wf's getNPart() is not valid" );
        }
        if (wf->hasVD1() != hasVD1()){
            throw std::invalid_argument( "Provided wf's hasVD1() is not valid" );
        }
        if (wf->hasD1VD1() != hasD1VD1()){
            throw std::invalid_argument( "Provided wf's hasD1VD1() is not valid" );
        }
        if (wf->hasD2VD1() != hasD2VD1()){
            throw std::invalid_argument( "Provided wf's hasD2VD1() is not valid" );
        }

        _wfs.push_back(wf);
        setNProto( getNProto() + wf->getNProto() );
        setNVP( getNVP() + wf->getNVP() );
    }

    void setVP(const double *vp){
        int contvp = 0;
        for (WaveFunction * wf : _wfs){
            wf->setVP(vp+contvp);
            contvp += wf->getNVP();
        }
    }

    void getVP(double *vp){
        int contvp = 0;
        for (WaveFunction * wf : _wfs){
            wf->getVP(vp+contvp);
            contvp += wf->getNVP();
        }
    }

    void samplingFunction(const double * in, double * out){

    }

    double getAcceptance(){
        return 1.;
    }

    void computeAllDerivatives(const double *x){
        for (WaveFunction * wf : _wfs){
            wf->computeAllDerivatives();
        }

        // first derivative
        for (int i=0; i<getTotalNDim(); ++i){
            _setD1DivByWF(i, 0.);
            for (WaveFunction * wf : _wfs){
                _setD1DivByWF(i, getD1DivByWF(i) + wf->getD1DivByWF(i));
            }
        }
        // second derivative
        for (int i=0; i<getTotalNDim(); ++i){
            _setD2DivByWF(i, 0.);
            for (WaveFunction * wf : _wfs){
                _setD2DivByWF(i, getD2DivByWF(i) + wf->getD2DivByWF(i));
            }
            for (int iwf=0; iwf<_wfs.size()-1; ++iwf){
                for (int jwf=iwf+1; jwf<_wfs.size(); ++jwf){
                    _setD2DivByWF(i, getD2DivByWF(i) + 2. * _wfs[iwf]->getD1DivByWF(i) * _wfs[jwf]->getD1DivByWF(i));
                }
            }
        }
        // first variational
        if (hasVD1()){
            int contvp = 0;
            for (WaveFunction * wf : _wfs){
                for (int ivp=0; ivp<wf->getNVP(); ++ivp){
                    _setVD1DivByWF(ivp+contvp, wf->getVD1DivByWF(ivp));
                }
                contvp += wf->getNVP();
            }
        }
        // first cross derivative
        if (hasD1VD1()){
            int contvp;
            for (int i=0; i<getTotalNDim(); ++i){
                contvp = 0;
                for (WaveFunction * wf : _wfs){
                    for (int ivp=0; ivp<wf->getNVP(); ++ivp){
                        _setD1VD1DivByWF(i, ivp+contvp, wf->getD1VD1DivByWF(i, ivp));
                    }
                    contvp += wf->getNVP();
                }
                for (int iwf=0; iwf<_wfs.size(); ++iwf){
                    contvp = 0;
                    for (int jwf=0; jwf<_wfs.size(); ++jwf){
                        if (iwf != jwf){
                            for (int ivp=0; ivp<_wfs[jwf]; ++ivp){
                                _setD1VD1DivByWF(i, ivp+contvp, getD1VD1DivByWF() + _wfs[iwf]->getD1DivByWF(i) * _wfs[jwf]->getVD1DivByWF(ivp);
                            }
                        }
                        contvp += _wfs[jwf]->getNVP();
                    }
                }
            }
        }
        // second cross derivative
        if (hasD2VD1()){
            int contvp;
            for (int i=0; i<getTotalNDim(); ++i){
                 contvp = 0;
                 for ( WaveFunction * wf : _wfs){
                     for (int ivp=0; ivp<wf->getNVP(); ++ivp){
                        _setD2VD1DivByWF(i, ivp+contvp, wf->getD2VD1DivByWF(i, ivp));
                     }
                     contvp += wf->getNVP();
                 }

                 for (int iwf=0; iwf<_wfs.size(); ++iwf){
                     contvp = 0;
                     for (int jwf=0; jwf<_wfs.size(); ++jwf){
                         if (iwf != jwf){
                             for (int ivp=0; ivp<_wfs[jwf]->getNVP(); ++ivp){
                                 _setD2VD1DivByWF(i, ivp+contvp, getD2VD1DivByWF(i, ivp+contvp) + _wfs[iwf]->getD2DivByWF(i) * _wfs[jwf]->getVD1DivByWF(ivp));
                             }
                         }
                         contvp += _wfs[jwf]->getNVP();
                     }
                 }

                 for (int iwf=0; iwf<_wfs.size(); ++iwf){
                     contvp = 0;
                     for (int jwf=0; jwf<_wfs.size(); ++jwf){
                         if (iwf != jwf){
                             for (int ivp=0; ivp<_wfs[jwf]->getNVP(); ++ivp){
                                 _setD2VD1DivByWF(i, ivp+contvp, getD2VD1DivByWF(i, ivp+contvp) + 2. * _wfs[iwf]->getD1DivByWF(i) * _wfs[jwf]->getD1VD1DivByWF(i, ivp));
                             }
                         }
                         contvp += _wfs[jwf]->getNVP();
                     }
                 }

                 for (int iwf=0; iwf<_wfs.size()-1; ++iwf){
                     for (int jwf=iwf+1; jwf<_wfs.size(); ++jwf){
                         contvp = 0;
                         for (int kwf; kwf<_wfs.size(); ++kwf){
                            if ((kwf != iwf) && (kwf != jwf)){
                                for (int ivp=0; ivp<_wfs[kwf]->getNVP(); ++ivp){
                                    _setD2VD1DivByWF(i, ivp+contvp, getD2VD1DivByWF(i, ivp+contvp) + 2. * _wfs[iwf]->getD1DivByWF(i) * _wfs[jwf]->getD1DivByWF(i) * _wfs[kwf]->getVD1DivByWF(ivp));
                                }
                            }
                            contvp += _wfs[kwf]->getNVP();
                         }
                     }
                 }
            }
        }
    }

};


#endif
