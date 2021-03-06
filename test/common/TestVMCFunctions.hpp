#ifndef VMC_TESTVMCFUNCTIONS_HPP
#define VMC_TESTVMCFUNCTIONS_HPP

#include <cmath>

#include "vmc/EuclideanMetric.hpp"
#include "vmc/Hamiltonian.hpp"
#include "vmc/TwoBodyPseudoPotential.hpp"
#include "vmc/WaveFunction.hpp"


class QuadrExponential1D1POrbital: public vmc::WaveFunction
{
protected:
    double _a, _b;

    mci::SamplingFunctionInterface * _clone() const final
    {
        return new QuadrExponential1D1POrbital(_a, _b);
    }

public:
    QuadrExponential1D1POrbital(const double a, const double b):
            vmc::WaveFunction(1, 1, 1, 2, true, false, false)
    {
        _a = a;
        _b = b;
    }

    void setVP(const double * in) final
    {
        _a = in[0];
        //if (_a<0.01) _a=0.01;
        _b = in[1];
        //if (_b<0.01) _b=0.01;
        using namespace std;
        //cout << "change a and b! " << _a << "   " << _b << endl;
    }
    void getVP(double * out) const final
    {
        out[0] = _a;
        out[1] = _b;
    }

    void protoFunction(const double * in, double * out) final
    {
        *out = -2.*(_b*(in[0] - _a)*(in[0] - _a));
    }

    double acceptanceFunction(const double * protoold, const double * protonew) const final
    {
        return exp(protonew[0] - protoold[0]);
    }

    void computeAllDerivatives(const double * in) final
    {
        _setD1DivByWF(0, -2.*_b*(in[0] - _a));
        _setD2DivByWF(0, -2.*_b + (-2.*_b*(in[0] - _a))*(-2.*_b*(in[0] - _a)));
        if (hasVD1()) {
            _setVD1DivByWF(0, 2.*_b*(in[0] - _a));
            _setVD1DivByWF(1, -(in[0] - _a)*(in[0] - _a));
        }
    }

    double computeWFValue(const double * protovalues) const final
    {
        return exp(0.5*protovalues[0]);
    }
};

/*
  Trial Wave Function for N particles in 1 dimension, that uses parameters ai and b, with fixed ai and variational b.
  Psi  =  exp( -b * sum((xi-ai)^2) )
*/
class QuadrExponential1DNPOrbital: public vmc::WaveFunction
{
protected:
    double * _a;
    double _b;
    double _bi; // inverse b

    mci::SamplingFunctionInterface * _clone() const final
    {
        return new QuadrExponential1DNPOrbital(_npart, _a, _b);
    }

public:
    QuadrExponential1DNPOrbital(const int &npart, const double * a, const double &b):
            vmc::WaveFunction(1, npart, 1, 1, true, true, true), _b(b), _bi(1./b)
    {
        _a = new double[npart];
        for (int i = 0; i < npart; ++i) { _a[i] = a[i]; }
    }

    ~QuadrExponential1DNPOrbital() final
    {
        delete[] _a;
    }

    void setVP(const double * in) final
    {
        _b = in[0];
    }

    void getVP(double * out) const final
    {
        out[0] = _b;
    }

    void protoFunction(const double * x, double * out) final
    {
        *out = 0.;
        for (int i = 0; i < _npart; ++i) {
            *out += (x[i] - _a[i])*(x[i] - _a[i]);
        }
        *out *= -2.*_b;
    }

    double acceptanceFunction(const double * protoold, const double * protonew) const final
    {
        return exp(protonew[0] - protoold[0]);
    }

    void computeAllDerivatives(const double * x) final
    {
        if (hasVD1()) {
            _setVD1DivByWF(0, 0);
        }
        for (int i = 0; i < _npart; ++i) {
            _setD1DivByWF(i, -2.*_b*(x[i] - _a[i]));
            _setD2DivByWF(i, -2.*_b*(1. + (x[i] - _a[i])*getD1DivByWF(i)));
            if (hasVD1()) {
                _setVD1DivByWF(0, getVD1DivByWF(0) - (x[i] - _a[i])*(x[i] - _a[i]));
            }
        }

        for (int i = 0; i < _npart; ++i) {
            if (hasD1VD1()) {
                _setD1VD1DivByWF(i, 0, getD1DivByWF(i)*(_bi + getVD1DivByWF(0)));
            }
            if (hasD2VD1()) {
                _setD2VD1DivByWF(i, 0, getD2DivByWF(i)*(_bi + getVD1DivByWF(0)) + _bi*getD1DivByWF(i)*getD1DivByWF(i));
            }
        }
    }

    double computeWFValue(const double * protovalues) const final
    {
        return exp(0.5*protovalues[0]);
    }
};


class Gaussian1D1POrbital: public vmc::WaveFunction
{
protected:
    double _b;

    mci::SamplingFunctionInterface * _clone() const final
    {
        return new Gaussian1D1POrbital(_b);
    }

public:
    explicit Gaussian1D1POrbital(const double b):
            vmc::WaveFunction(1, 1, 1, 1, true, false, false)
    {
        _b = b;
    }

    void setVP(const double * in) final
    {
        _b = *in;
        //if (_b<0.01) _b=0.01;
        using namespace std;
        //cout << "change b! " << _b << endl;
    }
    void getVP(double * out) const final
    {
        *out = _b;
    }

    void protoFunction(const double * in, double * out) final
    {
        *out = -2.*_b*(*in)*(*in);
    }

    double acceptanceFunction(const double * protoold, const double * protonew) const final
    {
        return exp(protonew[0] - protoold[0]);
    }

    void computeAllDerivatives(const double * in) final
    {
        _setD1DivByWF(0, -2.*_b*(*in));
        _setD2DivByWF(0, -2.*_b + 4.*_b*_b*(*in)*(*in));
        if (hasVD1()) {
            _setVD1DivByWF(0, (-(*in)*(*in)));
        }
    }

    double computeWFValue(const double * protovalues) const final
    {
        return exp(0.5*protovalues[0]);
    }
};

/*
  Gaussian around origin with constant norm <Psi(p)|Psi(p)>
  f(r) = sqrt(a)*exp(-0.5 * a^2 * x^2)
*/
class ConstNormGaussian1D1POrbital: public vmc::WaveFunction
{
protected:
    double _a; // the vp
    double _sqa; // sqrt(a)
    double _asq; // a^2
    double _asqsq; // a^2*a^2

    mci::SamplingFunctionInterface * _clone() const final
    {
        return new ConstNormGaussian1D1POrbital(_a, this->hasVD1());
    }

public:
    explicit ConstNormGaussian1D1POrbital(double a, bool flag_vd1 = true):
            WaveFunction(1, 1, 1, 1, flag_vd1, false, false),
            _a(a), _sqa(sqrt(a)), _asq(a*a), _asqsq((a*a)*(a*a)) {}

    void setVP(const double in[]) final
    {
        _a = in[0];
        _sqa = sqrt(_a);
        _asq = _a*_a;
        _asqsq = _asq*_asq;
    }
    void getVP(double out[]) const final { out[0] = _a; }

    void protoFunction(const double in[], double out[]) final
    {
        out[0] = _asq*in[0]*in[0];
    }

    double acceptanceFunction(const double protoold[], const double protonew[]) const final
    {
        const double diff = protoold[0] - protonew[0];
        return exp(diff);
    }

    void computeAllDerivatives(const double in[]) final
    {
        const double xsq = in[0]*in[0];
        _setD1DivByWF(0, -_asq*in[0]);
        _setD2DivByWF(0, _asqsq*xsq - _asq);
        if (this->hasVD1()) { _setVD1DivByWF(0, (0.5/_a - _a*xsq)); }
    }

    double computeWFValue(const double protovalues[]) const final
    {
        return _sqa*exp(-0.5*protovalues[0]);
    }
};


class HarmonicOscillator1D1P: public vmc::Hamiltonian
{
protected:
    double _w;

    mci::ObservableFunctionInterface * _clone() const final
    {
        return new HarmonicOscillator1D1P(_w, _flag_PBKE);
    }

public:
    explicit HarmonicOscillator1D1P(double w, bool flag_PBKE = true): vmc::Hamiltonian(1, 1, flag_PBKE) { _w = w; }
    double localPotentialEnergy(const double * r) final
    {
        return (0.5*_w*_w*(*r)*(*r));
    }
};


class He3u2: public vmc::TwoBodyPseudoPotential
{
/*
    u(r) = b/r^5
*/
private:
    double _b;

public:
    explicit He3u2(vmc::EuclideanMetric * em):
            vmc::TwoBodyPseudoPotential(em, 1, true, true, true)
    {
        _b = -1.;
    }

    void setVP(const double * vp) final { _b = vp[0]; }
    void getVP(double * vp) const final { vp[0] = _b; }

    double ur(const double dist) final
    {
        return _b/pow(dist, 5);
    }

    double urD1(const double dist) final
    {
        return -5.*_b/pow(dist, 6);
    }

    double urD2(const double dist) final
    {
        return 30.*_b/pow(dist, 7);
    }

    void urVD1(const double dist, double * vd1) final
    {
        vd1[0] = 1./pow(dist, 5);
    }

    void urD1VD1(const double dist, double * d1vd1) final
    {
        d1vd1[0] = -5./pow(dist, 6);
    }

    void urD2VD1(const double dist, double * d1vd1) final
    {
        d1vd1[0] = 30./pow(dist, 7);
    }
};


class PolynomialU2: public vmc::TwoBodyPseudoPotential
{
/*
    u(r) = a * r^2 + b * r^3
*/

private:
    double _a, _b;

public:
    PolynomialU2(vmc::EuclideanMetric * em, double a, double b):
            vmc::TwoBodyPseudoPotential(em, 2, true, true, true)
    {
        _a = a;
        _b = b;
    }

    void setVP(const double * vp) final
    {
        _a = vp[0];
        _b = vp[1];
    }
    void getVP(double * vp) const final
    {
        vp[0] = _a;
        vp[1] = _b;
    }

    double ur(const double r) final
    {
        return _a*pow(r, 2) + _b*pow(r, 3);
    }

    double urD1(const double r) final
    {
        return 2.*_a*r + 3.*_b*pow(r, 2);
    }

    double urD2(const double r) final
    {
        return 2.*_a + 6.*_b*r;
    }

    void urVD1(const double r, double * vd1) final
    {
        vd1[0] = r*r;
        vd1[1] = r*r*r;
    }

    void urD1VD1(const double r, double * d1vd1) final
    {
        d1vd1[0] = 2.*r;
        d1vd1[1] = 3.*r*r;
    }

    void urD2VD1(const double r, double * d2vd1) final
    {
        d2vd1[0] = 2.;
        d2vd1[1] = 6.*r;
    }
};


class FlatU2: public vmc::TwoBodyPseudoPotential
{
/*
    u(r) = K
*/
private:
    double _K;

public:
    FlatU2(vmc::EuclideanMetric * em, const double &K):
            vmc::TwoBodyPseudoPotential(em, 1, true, true, true)
    {
        _K = K;
    }

    void setVP(const double * vp) final
    {
        _K = vp[0];
    }
    void getVP(double * vp) const final
    {
        vp[0] = _K;
    }

    double ur(const double /*r*/) final
    {
        return _K;
    }

    double urD1(const double /*r*/) final
    {
        return 0.;
    }

    double urD2(const double /*r*/) final
    {
        return 0.;
    }

    void urVD1(const double /*r*/, double * vd1) final
    {
        vd1[0] = 1.;
    }

    void urD1VD1(const double /*r*/, double * d1vd1) final
    {
        d1vd1[0] = 0.;
    }

    void urD2VD1(const double /*r*/, double * d2vd1) final
    {
        d2vd1[0] = 0.;
    }
};

#endif
