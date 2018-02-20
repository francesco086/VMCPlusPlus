#include <iostream>
#include <cmath>
#include <stdexcept>
#include <gsl/gsl_siman.h>

#include "WaveFunction.hpp"
#include "Hamiltonian.hpp"
#include "VMC.hpp"
#include "ConjGrad.hpp"
#include "FFNNWaveFunction.hpp"

#include "FeedForwardNeuralNetwork.hpp"
#include "PrintUtilities.hpp"



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

    // potential energy
    double localPotentialEnergy(const double *r)
    {
        return (0.5*_w*_w*(*r)*(*r));
    }
};





int main(){
    using namespace std;

    const int HIDDENLAYERSIZE = 5;
    const int NHIDDENLAYERS = 1;
    FeedForwardNeuralNetwork * ffnn = new FeedForwardNeuralNetwork(2, HIDDENLAYERSIZE, 2);
    for (int i=0; i<NHIDDENLAYERS-1; ++i){
        ffnn->pushHiddenLayer(HIDDENLAYERSIZE);
    }
    ffnn->connectFFNN();
    // FeedForwardNeuralNetwork * ffnn = new FeedForwardNeuralNetwork("start_wf.txt");
    ffnn->addFirstDerivativeSubstrate();
    ffnn->addSecondDerivativeSubstrate();
    cout << "Created FFNN with " << NHIDDENLAYERS << " hidden layer(s) of " << HIDDENLAYERSIZE << " units each." << endl << endl;

    // Declare the trial wave functions
    FFNNWaveFunction * psi = new FFNNWaveFunction(1, 1, ffnn);

    // Store in two files the the initial wf, one for plotting, and for recovering it as nn
    cout << "Writing the plot file of the initial wave function in (plot_)init_wf.txt" << endl << endl;
    double * base_input = new double[psi->getFFNN()->getNInput()]; // no need to set it, since it is 1-dim
    const int input_i = 0;
    const int output_i = 0;
    const double min = -5.;
    const double max = 5.;
    const int npoints = 500;
    writePlotFile(psi->getFFNN(), base_input, input_i, output_i, min, max, npoints, "getOutput", "plot_init_wf.txt");
    psi->getFFNN()->storeOnFile("wf_init.txt");




    // Declare an Hamiltonian
    // We use the harmonic oscillator with w=1 and w=2
    const double w1 = 1.;
    HarmonicOscillator1D1P * ham = new HarmonicOscillator1D1P(w1, psi);




    cout << endl << " - - - FFNN-WF FUNCTION OPTIMIZATION - - - " << endl << endl;

    VMC * vmc; // VMC object we will resuse
    const long E_NMC = 4000l; // MC samplings to use for computing the energy
    cout << "E_NMC = " << E_NMC << endl << endl;
    double * energy = new double[4]; // energy
    double * d_energy = new double[4]; // energy error bar
    double * vp = new double[psi->getNVP()];


    cout << "-> ham1:    w = " << w1 << endl << endl;
    vmc = new VMC(psi, ham);

    // set an integration range, because the NN might be completely delocalized
    double ** irange = new double*[1];
    irange[0] = new double[2];
    irange[0][0] = -5.;
    irange[0][1] = 5.;
    cout << "Integration range: " << irange[0][0] << "   <->   " << irange[0][1] << endl << endl;
    vmc->getMCI()->setIRange(irange);


    cout << "   Starting energy:" << endl;
    vmc->computeVariationalEnergy(E_NMC, energy, d_energy);
    cout << "       Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
    cout << "       Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
    cout << "       Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
    cout << "       Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl;




    const int N_TRIES = 0;
    const int ITERS_FIXED_T = 10;
    const double STEP_SIZE = 0.5;
    const double K = 1.;
    const double T_INITIAL = 0.5;
    const double MU_T = 1.01;
    const double T_MIN = 0.001;
    cout << "Simulated Annealing parameters:    N_TRIES=" << N_TRIES << "    ITERS_FIXED_T=" << ITERS_FIXED_T << "    STEP_SIZE=" << STEP_SIZE << "    K=" << K << "    T_INITIAL=" << T_INITIAL << "    MU_T=" << MU_T << "    T_MIN=" << T_MIN << endl << endl;
    cout << "   Optimization . . ." << endl;
    gsl_siman_params_t params = {N_TRIES, ITERS_FIXED_T, STEP_SIZE, K, T_INITIAL, MU_T, T_MIN};
    vmc->simulatedAnnealingOptimization(E_NMC, 1.0, 10.0, 0.01, params);
    cout << "   . . . Done!" << endl << endl;

    cout << "   Optimized energy:" << endl;
    vmc->computeVariationalEnergy(E_NMC, energy, d_energy);
    cout << "       Total Energy        = " << energy[0] << " +- " << d_energy[0] << endl;
    cout << "       Potential Energy    = " << energy[1] << " +- " << d_energy[1] << endl;
    cout << "       Kinetic (PB) Energy = " << energy[2] << " +- " << d_energy[2] << endl;
    cout << "       Kinetic (JF) Energy = " << energy[3] << " +- " << d_energy[3] << endl << endl << endl;


    // Store in two files the the initial wf, one for plotting, and for recovering it as nn
    cout << "Writing the plot file of the optimised wave function in (plot_)opt_wf.txt" << endl << endl;
    writePlotFile(psi->getFFNN(), base_input, input_i, output_i, min, max, npoints, "getOutput", "plot_opt_wf.txt");
    psi->getFFNN()->storeOnFile("opt_wf.txt");



    delete[] irange[0];
    delete[] irange;
    delete vmc;
    delete[] vp;
    delete[] d_energy;
    delete[] energy;
    delete ham;
    delete base_input;
    delete psi;
    delete ffnn;



    return 0;
}