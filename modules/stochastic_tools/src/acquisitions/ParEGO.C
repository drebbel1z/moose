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
ParEGO::computeAcquisition(std::vector<Real> & acq,
                           const std::vector<std::vector<Real>> & gp_mean,
                           const Eigen::Tensor<Real, 3> & test_uncertainty,
                           const std::vector<std::vector<Real>> & test_inputs,
                           const std::vector<std::vector<Real>> & /*train_inputs*/,
                           const std::vector<Real> & /*generic*/,
                           const Real & num_props) const
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

      Real gamma_sample = Gamma::quantile(distrib(generator), 1.5, 1.0);

      lambda_vec.push_back(gamma_sample); // distrib(generator)

      sum += gamma_sample;
    }

    for (unsigned int i = 0; i < lambda_vec.size(); i++)
    {
      lambda_vec[i] /= sum;
    }

    lambda_mat.push_back(lambda_vec);
  }

  // create a matrix like gp_mean
  std::vector<std::vector<Real>> samples_from_gps(gp_mean.size(),
                                                  std::vector<Real>(gp_mean[0].size()));

  // sample from the gps and put it in samples from gps

  for (int i = 0; i < gp_mean[0].size(); i++)
  {
    // copy first column into a vector as well as first view of the uncertainty tensor
    std::vector<Real> mean_column(gp_mean.size());
    RealEigenMatrix test_uncertainty_view(gp_mean.size(), gp_mean.size());
    for (size_t j = 0; j < gp_mean.size(); j++)
    {
      mean_column[j] = gp_mean[j][i];
    }

    for (int j = 0; j < gp_mean.size(); j++)
    {
      for (int k = 0; k < gp_mean.size(); k++)
      {
        test_uncertainty_view(j, k) = test_uncertainty(j, k, i);
      }
    }

    // sample from gp
    std::vector<Real> sample_from_gp =
        this->generate_sample_from_gp_posterior(mean_column, test_uncertainty_view, num_props);
    for (size_t j = 0; j < gp_mean.size(); j++)
    {
      samples_from_gps[j][i] = sample_from_gp[j];
    }
  }

  RealEigenMatrix augmented_tchebycheff(gp_mean.size(), int(num_props));
  for (unsigned int j = 0; j < num_props; j++)
  {
    for (unsigned int i = 0; i < gp_mean.size(); i++)
    {
      RealEigenMatrix ljfj(1, gp_mean[0].size());
      for (unsigned int k = 0; k < gp_mean[0].size(); k++)
      {
        ljfj(0, k) = lambda_mat[j][k] * samples_from_gps[i][k];
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

std::vector<Real>
ParEGO::generate_sample_from_gp_posterior(const std::vector<Real> & gp_mean,
                                          const RealEigenMatrix & test_uncertainty,
                                          const int num_props) const
{
  std::vector<Real> sample_from_gp(gp_mean.size());
  std::vector<std::vector<Real>> normal_sample_mat;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<> distrib(0.0, 1.0);

  for (unsigned int j = 0; j < num_props; j++)
  {
    std::vector<Real> normal_sample_vec;
    for (unsigned int i = 0; i < gp_mean.size(); i++)
    {
      auto sample = Normal::quantile(distrib(generator), 0.0, 1.0);
      normal_sample_vec.push_back(sample); // distrib(generator)
    }
    normal_sample_mat.push_back(normal_sample_vec);
  }

  RealEigenMatrix normal_sample(gp_mean.size(), int(num_props));
  for (unsigned int j = 0; j < num_props; j++)
  {
    for (unsigned int i = 0; i < gp_mean.size(); i++)
    {
      normal_sample(i, j) = normal_sample_mat[j][i];
    }
  }

  RealEigenMatrix account_for_cov = test_uncertainty * normal_sample;

  RealEigenMatrix maxrowwise = account_for_cov.rowwise().maxCoeff();

  for (unsigned int i = 0; i < gp_mean.size(); i++)
  {
    sample_from_gp[i] = gp_mean[i] + maxrowwise(i);
  }

  return sample_from_gp;
}
