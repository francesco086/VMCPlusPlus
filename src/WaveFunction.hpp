#ifndef WAVE_FUNCTION
#define WAVE_FUNCTION

#include "MCISamplingFunctionInterface.hpp"
#include "MCICallBackOnAcceptanceInterface.hpp"

#include <iostream>

/*
IMPLEMENTATIONS OF THIS INTERFACE MUST INCLUDE:

    - void setVP(const double *vp)
            set the variational parameters

    - void getVP(double *vp)
            get the variational parameters

    - void samplingFunction(const double * in, double * out)
            heritage from MCISamplingFunctionInterface, uses Psi^2

    - double getAcceptance()
            heritage from MCISamplingFunctionInterface

    - void computeAllDerivatives(const double *x)
            use the setters for derivatives values (setD1DivByWF, setD2DivByWF, etc.)

*/


class WaveFunction: public MCISamplingFunctionInterface, public MCICallBackOnAcceptanceInterface{
protected:
    int _nvp;  //number of variational parameters involved
    int _npart;
    int _nspacedim;

    double * _d1_divbywf;
    double * _d2_divbywf;
    double * _vd1_divbywf;
    double ** _d1vd1_divbywf;
    double ** _d2vd1_divbywf;

public:
    WaveFunction(const int &nspacedim, const int &npart, const int &ncomp, const int &nvp, bool flag_vd1=true, bool flag_d1vd1=true, bool flag_d2vd1=true):
    MCISamplingFunctionInterface(nspacedim*npart, ncomp),
    MCICallBackOnAcceptanceInterface(nspacedim*npart){
        _nspacedim=nspacedim;
        _npart=npart;
        _nvp=nvp;

        _d1_divbywf = new double[nspacedim*npart];
        _d2_divbywf = new double[nspacedim*npart];

        _vd1_divbywf = 0;
        if (flag_vd1){
            _vd1_divbywf = new double[nvp];
        }

        _d1vd1_divbywf = 0;
        if (flag_d1vd1){
            _d1vd1_divbywf = new double*[nspacedim*npart];
            for (int id1=0; id1<nspacedim*npart; ++id1) _d1vd1_divbywf[id1] = new double[nvp];
        }

        _d2vd1_divbywf = 0;
        if (flag_d2vd1){
            _d2vd1_divbywf = new double*[nspacedim*npart];
            for (int id2=0; id2<nspacedim*npart; ++id2) _d2vd1_divbywf[id2] = new double[nvp];
        }
    }
    virtual ~WaveFunction(){
        delete[] _d1_divbywf;
        delete[] _d2_divbywf;
        if (_vd1_divbywf != 0){
            delete[] _vd1_divbywf;
        }
        if (_d1vd1_divbywf != 0){
            for (int id1=0; id1<getTotalNDim(); ++id1){
                delete[] _d1vd1_divbywf[id1];
            }
            delete[] _d1vd1_divbywf;
        }
        if (_d2vd1_divbywf != 0){
            for (int id2=0; id2<getTotalNDim(); ++id2){
                delete[] _d2vd1_divbywf[id2];
            }
            delete[] _d2vd1_divbywf;
        }
    }

    int getNSpaceDim(){return _nspacedim;}
    int getTotalNDim(){return MCISamplingFunctionInterface::getNDim();}
    int getNPart(){return _npart;}
    int getNVP(){return _nvp;}


    // --- interface for manipulating the variational parameters
    virtual void setVP(const double *vp) = 0;    // --- MUST BE IMPLEMENTED
    virtual void getVP(double *vp) = 0;    // --- MUST BE IMPLEMENTED


    // --- computation of the derivatives
    // When called, this method computes all the internal values, such as the derivatives,
    // and stored them internally, ready to be accessed with the getters methods.
    // It requires the positions as input
    virtual void computeAllDerivatives(const double *x) = 0;    // --- MUST BE IMPLEMENTED


    // --- method herited from MCICallBackOnAcceptanceInterface, that will simply call computeAllDerivatives
    void callBackFunction(const double *x, const bool flag_observation){
        if (flag_observation){
            computeAllDerivatives(x);
        }
    }


    // --- getters and setters for the derivatives
    // first derivative divided by the wf
<<<<<<< HEAD
    void setD1DivByWF(const int &id1, const double &d1_divbywf){_d1_divbywf[id1] = d1_divbywf;}
    double getD1DivByWF(const int &id1){return _d1_divbywf[id1];}
=======
    void setD1DivByWF(const int &id1, const double &d1_logwf){_d1_logwf[id1] = d1_logwf;}
    double getD1DivByWF(const int &id1){return _d1_logwf[id1];}
    double * getD1DivByWF(){return _d1_logwf;}
>>>>>>> started implementing - work in progress..
    // second derivative divided by the wf
    void setD2DivByWF(const int &id2, const double &d2_divbywf){_d2_divbywf[id2] = d2_divbywf;}
    double getD2DivByWF(const int &id2){return _d2_divbywf[id2];}
    // variational derivative divided by the wf
    bool hasVD1(){return (_vd1_divbywf == 0 ? false : true);}
    void setVD1DivByWF(const int &ivd1, const double &vd1_divbywf){_vd1_divbywf[ivd1] = vd1_divbywf;}
    double getVD1DivByWF(const int &ivd1){return _vd1_divbywf[ivd1];}
    // cross derivative: first derivative and first variational derivative divided by the wf
    bool hasD1VD1(){return (_d1vd1_divbywf == 0 ? false : true);}
    void setD1VD1DivByWF(const int &id1, const int &ivd1, const double &d1vd1_divbywf){_d1vd1_divbywf[id1][ivd1] = d1vd1_divbywf;}
    double getD1VD1DivByWF(const int &id1, const int &ivd1){return _d1vd1_divbywf[id1][ivd1];}
    // cross derivative: second derivative and first variational derivative divided by the wf
    bool hasD2VD1(){return (_d2vd1_divbywf == 0 ? false : true);}
    void setD2VD1DivByWF(const int &id2, const int &ivd1, const double &d2vd1_divbywf){_d2vd1_divbywf[id2][ivd1] = d2vd1_divbywf;}
    double getD2VD1DivByWF(const int &id2, const int &ivd1){return _d2vd1_divbywf[id2][ivd1];}
};


#endif
