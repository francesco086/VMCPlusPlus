#ifndef VMC_SYMMETRIZERWAVEFUNCTION_HPP
#define VMC_SYMMETRIZERWAVEFUNCTION_HPP

#include "vmc/WaveFunction.hpp"

namespace vmc
{

class SymmetrizerWaveFunction: public WaveFunction
{
    /*
      This class wraps around an arbitrary WaveFunction and applies by default the
      general Symmetrizer operator or by optional second constructor argument instead
      the general AntiSymmetrizer operator to the given wavefunction. The result is
      a wavefunction that is either symmetric or antisymmetric with respect to arbitrary
      particle exchange permutations.

      NOTE 1: These general Symmetrizer operators are not Slater determinants, so they don't
      rely on N single particle orbitals, but instead a single N-particle wavefunction.

      NOTE 2: Be aware that the time to evaluate the samplingFunction or derivatives of the
      SymmetrizerWaveFunction requires at least N! as much time as the corresponding evaluations
      of the original WaveFunction, where n is the number of particles. Therefore it cannot be used
      in practice for more than a handful of particles.
    */
protected:
    WaveFunction * const _wf; // we wrap around an existing wavefunction
    const bool _flag_antisymmetric; // should we use the antisymmetrizer instead of symmetrizer?

    // internal helpers
    unsigned long _npart_factorial() const;
    void _swapPositions(double * x, int i, int j);
    void _swapIndices(int * ids, int i, int j);
    void _computeStandardDerivatives(const double * x, double normf);
    void _addSwapDerivatives(const double * x, double normf, const int * ids);

    mci::SamplingFunctionInterface * _clone() const final
    {
        return new SymmetrizerWaveFunction(_wf, _flag_antisymmetric);
    }

    // we have a ProtoFunctionInterface as member (_wf), so we need to implement these:
    void _newToOld() override;
    void _oldToNew() override { _wf->oldToNew(); }

public:
    explicit SymmetrizerWaveFunction(WaveFunction * wf, bool flag_antisymmetric = false):
            WaveFunction(wf->getNSpaceDim(), wf->getNPart(), 1, wf->getNVP(), wf->hasVD1(), wf->hasD1VD1(), wf->hasD2VD1()),
            _wf(wf), _flag_antisymmetric(flag_antisymmetric) {}

    ~SymmetrizerWaveFunction() override = default;

    void setVP(const double * vp) override;

    void getVP(double * vp) const override;

    void protoFunction(const double * in, double * out) override;

    double acceptanceFunction(const double * protoold, const double * protonew) const override;

    void computeAllDerivatives(const double * x) override;

    double computeWFValue(const double * protovalues) const override;

    bool isAntiSymmetric() const { return _flag_antisymmetric; }
};
} // namespace vmc

#endif
