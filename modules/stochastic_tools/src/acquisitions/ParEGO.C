//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParEGO.h"
#include "Normal.h"
#include "Gamma.h"
#include <cmath>
#include <random>

registerMooseObject("StochasticToolsApp", ParEGO);

InputParameters
ParEGO::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();

  params.addClassDescription("based on ParEGO: A Hybrid Algorithm With On-Line Landscape "
                             "Approximation for Expensive Multiobjective Optimization Problems");
  return params;
}

ParEGO::ParEGO(const InputParameters & parameters) : ParallelAcquisitionFunctionBase(parameters) {}

void
ParEGO::computeAcquisition(std::vector<Real> & /*acq*/,
                           const std::vector<Real> & /*gp_mean*/,
                           const std::vector<Real> & /*gp_std*/,
                           const std::vector<std::vector<Real>> & /*test_inputs*/,
                           const std::vector<std::vector<Real>> & /*train_inputs*/,
                           const std::vector<Real> & /*generic*/) const
{
}

void
ParEGO::Acquisition(std::vector<Real> & acq,
                    const std::vector<Real> & gp_mean,
                    const RealEigenMatrix & test_uncertainty,
                    const std::vector<std::vector<Real>> & test_inputs,
                    const std::vector<std::vector<Real>> & /*train_inputs*/,
                    const std::vector<Real> & /*generic*/,
                    const Real & num_props,
                    const std::vector<GaussianProcessSurrogate> & gps) const
{
  std::vector<std::vector<Real>> lambda_mat;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<> distrib(0.0, 1.0);

  // since you want a batch, obtain num_props lambda vectors. lambda vectors are dirichlet
  // distributed
  for (unsigned int j = 0; j < num_props; j++)
  {
    std::vector<Real> lambda_vec;
    Real sum = 0;
    for (unsigned int i = 0; i < gp_mean[0].size(); i++)
    {
      auto sample = Gamma::quantile(distrib(generator1), 1.5, 1.0);
      lambda_vec.push_back(sample); // distrib(generator)
      sum += sample;
    }

    for (unsigned int i = 0; i < lambda_vec.size(); i++)
    {
      lambda_vec[i] /= sum;
    }

    lambda_mat.push_back(lambda_vec);
  }

  RealEigenMatrix augmented_tchebycheff(gp_mean.size(), int(num_props));
  for (unsigned int j = 0; j < num_props; j++)
  {
    for (unsigned int i = 0; i < gp_mean.size(); i++)
    {
      RealEigenMatrix ljfj(1, gp_mean[0].size());
      for (unsigned int k = 0; k < gp_mean[0].size(); k++)
      {
        ljfj(0, k) = lambda_mat[j][k] * gp_mean[i][k]
      }
      augmented_tchebycheff(i, j) = ljfj.maxCoeff() + 0.05 * ljfj.sum();
    }
  }
  RealEigenMatrix maxrowwise = augmented_tchebycheff.rowwise().maxCoeff();
  for (unsigned int i = 0; i < gp_mean.size(); i++)
  {
    acq[i] = maxrowwise(i);
  }
}

void
ParEGO::generate_sample_from_gp_posterior(const std::vector<std::vector<Real>> & test_inputs,
                                          const std::vector<GaussianProcessSurrogate> & gps)
{
  std::vector<Real> tmp;
  tmp.resize(_n_dim);
  for (unsigned int i = 0; i < _gp_outputs_test.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = _inputs_test[i][j];

    for (unsigned int j = 0; j < _num_objs; ++j)
      _gp_outputs_test[i][j] = _gp_eval[j].evaluate(tmp, _gp_std_test[i]);
  }
}
