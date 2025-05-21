//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessSurrogate.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", GaussianProcessSurrogate);

InputParameters
GaussianProcessSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcessSurrogate::GaussianProcessSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _gp(declareModelData<StochasticTools::GaussianProcess>("_gp")),
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
{
}

void
GaussianProcessSurrogate::setupCovariance(UserObjectName covar_name)
{
  if (_gp.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _gp.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
}

Real
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  std::vector<Real> y;
  std::vector<Real> std;
  this->evaluate(x, y, std);
  std_dev = std[0];
  return y[0];
}

void
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  std::vector<Real> std_dummy;
  this->evaluate(x, y, std_dummy);
}

void
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x,
                                   std::vector<Real> & y,
                                   std::vector<Real> & std) const
{
  const unsigned int n_dims = _training_params.cols();

  mooseAssert(x.size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _gp.getCovarFunction().numOutputs();

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  RealEigenMatrix test_points(1, n_dims);
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    test_points(0, ii) = x[ii];

  _gp.getParamStandardizer().getStandardized(test_points);

  RealEigenMatrix K_train_test(_training_params.rows() * n_outputs, n_outputs);

  _gp.getCovarFunction().computeCovarianceMatrix(
      K_train_test, _training_params, test_points, false);
  RealEigenMatrix K_test(n_outputs, n_outputs);
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Compute the predicted mean value (centered)
  RealEigenMatrix pred_value = (K_train_test.transpose() * _gp.getKResultsSolve()).transpose();
  // De-center/scale the value and store for return
  _gp.getDataStandardizer().getDestandardized(pred_value);

  RealEigenMatrix pred_var =
      K_test - (K_train_test.transpose() * _gp.getKCholeskyDecomp().solve(K_train_test));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _gp.getDataStandardizer().getDescaled(std_dev_mat);

  for (const auto output_i : make_range(n_outputs))
  {
    y[output_i] = pred_value(0, output_i);
    std[output_i] = std_dev_mat(output_i, output_i);
  }
}

  const Eigen::LLT<RealEigenMatrix> & GaussianProcessSurrogate::getPredVarCholesky(const std::vector<std::vector<Real>> & x)const{
  const unsigned int n_dims = _training_params.cols();
  const unsigned int num_test_points = x.size();

  mooseAssert(x[0].size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _gp.getCovarFunction().numOutputs();

  std::vector<std::vector<Real>> test_points_std_array;
  // RealEigenMatrix test_points(x.size(), n_dims);
  for (unsigned int jj =0; jj<num_test_points; jj++){
    std::vector<Real> test_row;
    for (unsigned int ii = 0; ii < n_dims; ++ii)
      test_row.push_back(x[jj][ii]);
    test_points_std_array.push_back(test_row);
  }

  RealEigenMatrix test_points = Eigen::Map<Eigen::Matrix<Real, num_test_points, n_dims> >(test_points_std_array.data());

  // does this broadcast? if so we can skip for loop?
  for (unsigned int jj = 0; jj < num_test_points; ++jj){
    _gp.getParamStandardizer().getStandardized(test_points(jj,Eigen::all));
  }
  


  RealEigenMatrix K_train_test_major(_training_params.rows() * n_outputs, num_test_points*n_outputs);

  for (unsigned int jj = 0; jj < num_test_points; ++jj){
    RealEigenMatrix K_train_test(_training_params.rows() * n_outputs, n_outputs);

    _gp.getCovarFunction().computeCovarianceMatrix(
        K_train_test, _training_params, test_points(jj,Eigen::all), false);
    
    for (unsigned int ii = 0; ii < n_dims; ++ii)
      K_train_test_major(ii,jj) = K_train_test(ii);
  }


  RealEigenMatrix K_test_major(n_outputs*num_test_points, n_outputs*num_test_points);

  for (unsigned int jj = 0; jj < num_test_points; ++jj){
    for (unsigned int ii = 0; ii < num_test_points; ++ii){
      RealEigenMatrix K_test(n_outputs, n_outputs);
      _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points(jj,Eigen::all), test_points(ii,Eigen::all), false);
      K_test_major(jj, ii) = K_test(0,0);
    }
  }

  RealEigenMatrix pred_var =
      K_test_major - (K_train_test_major.transpose() * _gp.getKCholeskyDecomp().solve(K_train_test_major));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _gp.getDataStandardizer().getDescaled(std_dev_mat);

  RealEigenMatrix cov_mat(num_test_points*n_outputs, num_test_points*n_outputs);
  
  for(unsigned int i=0; i< n_outputs; i++){
    for(unsigned int j=0; j< n_outputs; j++){
      cov_mat(i,j)= Utility::pow<2>(std_dev_mat(i,j));
    }
  }
  
  return cov_mat.llt();
  }
