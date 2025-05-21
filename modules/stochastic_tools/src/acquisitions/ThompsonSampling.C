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
ThompsonSampling::computeAcquisition(std::vector<Real> & acq,
                                        const std::vector<Real> & gp_mean,
                                        const Eigen::LLT<RealEigenMatrix> & test_uncertainty,
                                        const std::vector<std::vector<Real>> & /*test_inputs*/,
                                        const std::vector<std::vector<Real>> & /*train_inputs*/,
                                        const std::vector<Real> & /*generic*/) const
{
  const std::vector<Real> normal_sample_vec;

  for(unsigned int i=0; i< gp_mean.size();i++){
    normal_sample_vec.push_back(Normal::quantile(Sampler::getRand()));
  }
  
  RealEigenMatrix normal_sample = Eigen::Map<Eigen::Matrix<Real, gp_mean.size(), 1> >(normal_sample_vec.data());

  RealEigenMatrix account_for_cov = test_uncertainty.matrixL() * normal_sample;

  for(unsigned int i=0; i< gp_mean.size();i++){
    acq[i] = gp_mean[i]+account_for_cov(i,0);
  }
}

void
ThompsonSampling::computeAcquisition(std::vector<Real> & acq,
                                        const std::vector<Real> & gp_mean,
                                        const std::vector<Real> & test_uncertainty,
                                        const std::vector<std::vector<Real>> & /*test_inputs*/,
                                        const std::vector<std::vector<Real>> & /*train_inputs*/,
                                        const std::vector<Real> & generic) const
{
}
