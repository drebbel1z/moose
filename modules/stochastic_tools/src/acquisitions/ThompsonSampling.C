//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThompsonSampling.h"
#include "Normal.h"
#include <cmath>
#include <random>

registerMooseObject("StochasticToolsApp", ThompsonSampling);

InputParameters
ThompsonSampling::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  
  params.addClassDescription("Thompson Sampling acquisition function.");
  return params;
}

ThompsonSampling::ThompsonSampling(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
ThompsonSampling::computeAcquisition(std::vector<Real> & /*acq*/,
                                        const std::vector<Real> & /*gp_mean*/,
                                        const std::vector<Real> & /*gp_std*/,
                                        const std::vector<std::vector<Real>> & /*test_inputs*/,
                                        const std::vector<std::vector<Real>> & /*train_inputs*/,
                                        const std::vector<Real> & /*generic*/) const
{
}


void
ThompsonSampling::computeAcquisition(std::vector<Real> & acq,
                                        const std::vector<Real> & gp_mean,
                                        const RealEigenMatrix & test_uncertainty,
                                        const std::vector<std::vector<Real>> & /*test_inputs*/,
                                        const std::vector<std::vector<Real>> & /*train_inputs*/,
                                        const std::vector<Real> & /*generic*/) const
{
  std::vector<Real> normal_sample_vec;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<>  distrib(0.0, 1.0);

  for(unsigned int i=0; i< gp_mean.size();i++){
    auto sample= Normal::quantile(distrib(generator),0.0,1.0);
    normal_sample_vec.push_back(sample); // distrib(generator)
  }

  RealEigenMatrix normal_sample(gp_mean.size(),1);
  for(unsigned int i=0;i<gp_mean.size();i++){
      normal_sample(i,0)=normal_sample_vec[i];
  }

  RealEigenMatrix account_for_cov = test_uncertainty * normal_sample;

  for(unsigned int i=0; i< gp_mean.size();i++){
    acq[i] = gp_mean[i] + account_for_cov(i,0);
  }
}

