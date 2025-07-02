//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxValueEntropySearchMOBO.h"
#include "Normal.h"
#include <cmath>
#include <random>

registerMooseObject("StochasticToolsApp", MaxValueEntropySearchMOBO);

InputParameters
MaxValueEntropySearchMOBO::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();

  params.addClassDescription("Max-value entropy search for multi-objective bayesian optimization");
  return params;
}

MaxValueEntropySearchMOBO::MaxValueEntropySearchMOBO(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
MaxValueEntropySearchMOBO::computeAcquisition(
    std::vector<Real> & /*acq*/,
    const std::vector<Real> & /*gp_mean*/,
    const std::vector<Real> & /*gp_std*/,
    const std::vector<std::vector<Real>> & /*test_inputs*/,
    const std::vector<std::vector<Real>> & /*train_inputs*/,
    const std::vector<Real> & /*generic*/) const
{
}

void
MaxValueEntropySearchMOBO::Acquisition(std::vector<Real> & acq,
                                       const std::vector<Real> & gp_mean,
                                       const RealEigenMatrix & test_uncertainty,
                                       const std::vector<std::vector<Real>> & test_inputs,
                                       const std::vector<std::vector<Real>> & /*train_inputs*/,
                                       const std::vector<Real> & /*generic*/,
                                       const Real & num_props,
                                       const std::vector<GaussianProcessSurrogate> & gps) const
{
  // compute s samples of all the objs
  // currently setting s to 5. change to a variable later.
  const int s = 5;
  Eigen::Tensor<Real, 3> samples_of_objs(gp_mean.size(), gp_mean[0].size(), s);
  for (size_t i = 0; i < gp_mean[0].size(); i++)
  {
    RealEigenMatrix gp_mean_mat(gp_mean.size(), 1);

    for (size_t i = 0; i < gp_mean.size(); i++)
    {
      gp_mean_mat(i) = gp_mean[i];
    }

    RealEigenMatrix ith_gp_sample = this.ComputeSample(gp_mean_mat, test_uncertainty[i], s);

    for (size_t j = 0; j < gp_mean.size(); j++)
    {
      for (size_t k = 0; k < s; k++)
      {
        samples_of_objs(j, i, k) = ith_gp_sample(j, k);
      }
    }
  }
}

RealEigenMatrix
MaxValueEntropySearchMOBO::ComputeSample(const RealEigenMatrix & gp_mean,
                                         const RealEigenMatrix & test_uncertainty,
                                         const int num_samples) const
{
  std::vector<std::vector<Real>> normal_sample_mat;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<> distrib(0.0, 1.0);

  for (size_t j = 0; j < num_samples; j++)
  {
    std::vector<Real> normal_sample_vec;
    for (size_t i = 0; i < gp_mean.size(); i++)
    {
      auto sample = Normal::quantile(distrib(generator), 0.0, 1.0);
      normal_sample_vec.push_back(sample); // distrib(generator)
    }
    normal_sample_mat.push_back(normal_sample_vec);
  }

  RealEigenMatrix normal_sample(gp_mean.size(), int(num_samples));

  for (size_t j = 0; j < num_props; j++)
  {
    for (size_t i = 0; i < gp_mean.size(); i++)
    {
      normal_sample(i, j) = normal_sample_mat[j][i];
    }
  }

  RealEigenMatrix account_for_cov = test_uncertainty * normal_sample;

  RealEigenMatrix samples = gp_mean_mat + account_for_cov;

  return samples
}

RealEigenMatrix
MaxValueEntropySearchMOBO::NSGAII(const RealEigenMatrix & gp_samples) const
{
  auto num_rows = gp_samples.rows();
  auto num_objs = gp_samples.cols();
  // check for whether a function evaluation dominates the other
  bool dominates(const RealEigenMatrix & gp_sample_1, const RealEigenMatrix & gp_sample_2)
  {
    std::vector<bool> all_array, any_array;

    for (size_t i = 0; i < num_objs; i++)
    {
      all_array.push_back(gp_sample_1(i) <= gp_sample_2(i));
      any_array.push_back(gp_sample_1(i) < gp_sample_2(i));
    }
    return std::all_of(all_array.begin(), all_array.end(), [](bool i) { return i; }) &&
           std::any_of(any_array.begin(), any_array.end(), [](bool i) { return i; });
  }

  // finding the index of an element in array
  int index_of(int a, const std::vector<int> & list)
  {
    auto it = std::find(list.begin(), list.end(), a);
    if (it != list.end())
    {
      return std::distance(list.begin(), it);
    }
    else
    {
      return -1;
    }
  }

  // Function to sort list1 based on values
  std::vector<int> sort_by_values(const std::vector<int> & list1, const std::vector<int> & values)
  {
    std::vector<int> sorted_list = list1;

    // Custom comparator to sort indices based on values
    std::sort(sorted_list.begin(),
              sorted_list.end(),
              [&values](int a, int b) { return values[a] < values[b]; });

    return sorted_list;
  }

  // crowding distance
  std::vector<double> crowding_distance(const std::vector<std::vector<Real>> & values,
                                        const std::vector<int> & front)
  {
    std::vector<double> distance(front.size(), 0.0);
    std::vector<int> sorted1 = sort_by_values(front, values1);
    std::vector<int> sorted2 = sort_by_values(front, values2);

    distance[0] = std::numeric_limits<double>::infinity();
    distance[distance.size() - 1] = std::numeric_limits<double>::infinity();

    double max1 = *std::max_element(values1.begin(), values1.end());
    double min1 = *std::min_element(values1.begin(), values1.end());
    double max2 = *std::max_element(values2.begin(), values2.end());
    double min2 = *std::min_element(values2.begin(), values2.end());

    for (size_t k = 1; k < front.size() - 1; ++k)
    {
      distance[k] += (values1[sorted1[k + 1]] - values1[sorted1[k - 1]]) / (max1 - min1);
      distance[k] += (values2[sorted2[k + 1]] - values2[sorted2[k - 1]]) / (max2 - min2);
    }

    return distance;
  }

  // fast-non-dominated-sort(P)
  std::vector<std::vector<int>> S, front;
  std::vector<int> n(num_rows), rank(num_rows);

  for (size_t i = 0; i < num_rows; i++)
  {
    std::vector<Real> S_i;
    n[i] = 0;
    for (size_t j = 0; j < num_rows; j++)
    {
      if (dominates(gp_samples[i], gp_samples[j]))
      {
        S_i.push_back(gp_samples[j]);
      }
      else if (dominates(gp_samples[j], gp_samples[i]))
      {
        n[i] += 1
      }
    }
    if (n[i] == 0)
    {
      rank[i] = 0;
      if (std::find(front[0].begin(), front[0].end(), i) == front[0].end())
        front[0].push_back(i);
    }
  }

  int i = 0;
  while (!front[i].empty())
  {
    std::vector<int> Q;
    for (size_t j = 0; j < front[i].size(); ++j)
    {
      int p = front[i][j];
      for (size_t k = 0; k < S[p].size(); ++k)
      {
        int q = S[p][k];
        n[q] -= 1;
        if (n[q] == 0)
        {
          rank[q] = i + 1;
          if (std::find(Q.begin(), Q.end(), q) == Q.end())
          {
            Q.push_back(q);
          }
        }
      }
    }
    i += 1;
    front.push_back(Q);
  }

  return std::vector<std::vector<int>>(front.begin(), front.end() - 1);
}