/*    Copyright (c) 2010-2017, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 */

#ifndef TUDAT_LEASTSQUARESESTIMATION_H
#define TUDAT_LEASTSQUARESESTIMATION_H

#include <map>

#include <boost/function.hpp>

#include <Eigen/Core>
#include <Eigen/SVD>

namespace tudat
{

namespace linear_algebra
{

//! Function to get condition number of matrix (using SVD decomposition)
/*!
 *  Function to get condition number of matrix (using SVD decomposition)
 * \param informationMatrix Matrix for which condition number is to be computed
 * \return Condition number of matrix
 */
double getConditionNumberOfInformationMatrix( const Eigen::MatrixXd informationMatrix );

//! Function to get condition number of matrix from SVD decomposition
/*!
 *  Function to get condition number of matrix from SVD decomposition
 * \param singularValueDecomposition SVD decomposition of matrix
 * \return Condition number of matrix
 */
double getConditionNumberOfDecomposedMatrix( const Eigen::JacobiSVD< Eigen::MatrixXd >& singularValueDecomposition );

//! Solve system of equations with SVD decomposition, checking condition number in the process
/*!
 * Solve system of equations with SVD decomposition, checking condition number in the process. This function solves
 * A*x = b for the vector x.
 * \param matrixToInvert Matrix A that is to be inverted to solve the equation
 * \param rightHandSideVector Vector on the righthandside of the matrix equation that is to be solved
 * \param checkConditionNumber Boolean to denote whether the condition number is checked when estimating (warning is printed
 * when value exceeds maximumAllowedConditionNumber)
 * \param maximumAllowedConditionNumber Maximum value of the condition number of the covariance matrix that is allowed
 * (warning printed when exceeded)
 * \return Solution x of matrix equation A*x=b
 */
Eigen::VectorXd solveSystemOfEquationsWithSvd( const Eigen::MatrixXd matrixToInvert,
                                               const Eigen::VectorXd rightHandSideVector,
                                               const bool checkConditionNumber = 1,
                                               const double maximumAllowedConditionNumber = 1.0E-8 );

//! Function to multiply information matrix by diagonal weights matrix
/*!
 * Function to multiply information matrix by diagonal weights matrix
 * \param informationMatrix Matrix containing partial derivatives of observations (rows) w.r.t. estimated parameters
 * (columns)
 * \param diagonalOfWeightMatrix Diagonal of observation weights matrix (assumes all weights to be uncorrelated)
 * \return informationMatrix, premultiplied by square matrix with diagonalOfWeightMatrix as diagonal elements
 */
Eigen::MatrixXd multiplyInformationMatrixByDiagonalWeightMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix );

//! Function to compute inverse of covariance matrix at current iteration, including influence of a priori information
/*!
 * Function to compute inverse of covariance matrix at current iteration, including influence of a priori information
 * \param informationMatrix Matrix containing partial derivatives of observations (rows) w.r.t. estimated parameters
 * (columns)
 * \param diagonalOfWeightMatrix Diagonal of observation weights matrix (assumes all weights to be uncorrelated)
 * \param inverseOfAPrioriCovarianceMatrix Inverse of a priori covariance matrix
 * (warning printed when exceeded)
 * \return Inverse of covariance matrix at current iteration
 */
Eigen::MatrixXd calculateInverseOfUpdatedCovarianceMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const Eigen::MatrixXd& inverseOfAPrioriCovarianceMatrix );

//! Function to compute inverse of covariance matrix at current iteration
/*!
 * Function to compute inverse of covariance matrix at current iteration
 * \param informationMatrix Matrix containing partial derivatives of observations (rows) w.r.t. estimated parameters
 * (columns)
 * \param diagonalOfWeightMatrix Diagonal of observation weights matrix (assumes all weights to be uncorrelated)
 * \return Inverse of covariance matrix at current iteration
 */
Eigen::MatrixXd calculateInverseOfUpdatedCovarianceMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix );

//! Function to perform an iteration least squares estimation from information matrix, weights and residuals and a priori
//! information
/*!
 * Function to perform an iteration least squares estimation from information matrix, weights and residuals and a priori
 * information, as is typically done in orbit determination. This function also takes an inverse if the a priori covariance
 * matrix to constrain/stabilize the inversion.
 * \param informationMatrix Matrix containing partial derivatives of observations (rows) w.r.t. estimated parameters
 * (columns)
 * \param observationResiduals Difference between measured and simulated observations
 * \param diagonalOfWeightMatrix Diagonal of observation weights matrix (assumes all weights to be uncorrelated)
 * \param inverseOfAPrioriCovarianceMatrix Inverse of a priori covariance matrix
 * (warning printed when exceeded)
 * \param checkConditionNumber Boolean to denote whether the condition number is checked when estimating (warning is printed
 * when value exceeds maximumAllowedConditionNumber)
 * \param maximumAllowedConditionNumber Maximum value of the condition number of the covariance matrix that is allowed
 * \return Pair containing: (first: parameter adjustment, second: inverse covariance)
 */
std::pair< Eigen::VectorXd, Eigen::MatrixXd > performLeastSquaresAdjustmentFromInformationMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& observationResiduals,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const Eigen::MatrixXd& inverseOfAPrioriCovarianceMatrix,
        const bool checkConditionNumber = 1,
        const double maximumAllowedConditionNumber = 1.0E8  );

//! Function to perform an iteration least squares estimation from information matrix, weights and residuals
/*!
 * Function to perform an iteration least squares estimation from information matrix, weights and residuals, as is
 * typically done in orbit determination
 * \param informationMatrix Matrix containing partial derivatives of observations (rows) w.r.t. estimated parameters
 * (columns)
 * \param observationResiduals Difference between measured and simulated observations
 * \param diagonalOfWeightMatrix Diagonal of observation weights matrix (assumes all weights to be uncorrelated)
 * \param checkConditionNumber Boolean to denote whether the condition number is checked when estimating (warning is printed
 * when value exceeds maximumAllowedConditionNumber)
 * \param maximumAllowedConditionNumber Maximum value of the condition number of the covariance matrix that is allowed
 * (warning printed when exceeded)
 * \return Pair containing: (first: parameter adjustment, second: inverse covariance)
 */
std::pair< Eigen::VectorXd, Eigen::MatrixXd > performLeastSquaresAdjustmentFromInformationMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& observationResiduals,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const bool checkConditionNumber = 1,
        const double maximumAllowedConditionNumber = 1.0E8  );



} // namespace linear_algebra

} // namespace tudat

#endif // TUDAT_LEASTSQUARESESTIMATION_H
