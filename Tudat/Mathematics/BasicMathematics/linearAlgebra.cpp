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

#include <cmath>

#include <Eigen/LU>

#include "Tudat/Mathematics/BasicMathematics/linearAlgebra.h"
#include "Tudat/Basics/utilities.h"
namespace tudat
{

namespace linear_algebra
{

//! Function that returns that 'cross-product matrix'
Eigen::Matrix3d getCrossProductMatrix( const Eigen::Vector3d& vector )
{
    Eigen::Matrix3d crossProductMatrix = Eigen::Matrix3d::Zero( );
    crossProductMatrix( 1, 0 ) = vector.z( );
    crossProductMatrix( 0, 1 ) = -vector.z( );
    crossProductMatrix( 2, 0 ) = -vector.y( );
    crossProductMatrix( 0, 2 ) = vector.y( );
    crossProductMatrix( 2, 1 ) = vector.x( );
    crossProductMatrix( 1, 2 ) = -vector.x( );
    return crossProductMatrix;
}

//! Compute cosine of the angle between two vectors.
double computeCosineOfAngleBetweenVectors( const Eigen::VectorXd& vector0,
                                           const Eigen::VectorXd& vector1 )
{
    if( !( vector0.size( ) == vector1.size( ) ) )
    {
        throw std::runtime_error( "Error when computing angle between vectors; size is incompatible" );
    }

    // Get the cosine of the angle by dotting the normalized vectors.
    double dotProductOfNormalizedVectors = vector0.normalized( ).dot( vector1.normalized( ) );

    // Explicitly define the extreme cases, which can give problems with the acos function.
    if ( dotProductOfNormalizedVectors >= 1.0 )
    {
        return 1.0;
    }

    else if ( dotProductOfNormalizedVectors <= -1.0 )
    {
        return -1.0;
    }
    // Determine the actual angle.
    else
    {
        return dotProductOfNormalizedVectors;
    }
}

//! Compute angle between two vectors.
double computeAngleBetweenVectors( const Eigen::VectorXd& vector0, const Eigen::VectorXd& vector1 )
{
    // Determine the cosine of the angle by using another routine.
    double dotProductOfNormalizedVectors = computeCosineOfAngleBetweenVectors( vector0, vector1 );

    // Return arccosine of the above, which is effectively the angle.
    return std::acos( dotProductOfNormalizedVectors );
}

//! Computes the difference between two 3d vectors.
Eigen::Vector3d computeVectorDifference( const Eigen::Vector3d& vector0,
                                         const Eigen::Vector3d& vector1 )
{
    return ( vector0 - vector1 );
}

//! Computes norm of the the difference between two 3d vectors.
double computeNormOfVectorDifference( const Eigen::Vector3d& vector0,
                                      const Eigen::Vector3d& vector1 )
{
    return ( vector0 - vector1 ).norm( );
}

//! Computes the norm of a 3d vector
double getVectorNorm( const Eigen::Vector3d& vector )
{
    return vector.norm( );
}

Eigen::Vector3d evaluateSecondBlockInStateVector(
        const boost::function< Eigen::Vector6d( const double ) > stateFunction,
        const double time )
{
    return stateFunction( time ).segment( 3, 3 );
}

//! Computes the norm of a 3d vector from a vector-returning function.
double getVectorNormFromFunction( const boost::function< Eigen::Vector3d( ) > vectorFunction )
{
    return getVectorNorm( vectorFunction( ) );
}


double getConditionNumberOfInformationMatrix( const Eigen::MatrixXd informationMatrix )
{
    return getConditionNumberOfDecomposedMatrix( getSVDDecompositionOfInformationMatrix( informationMatrix ) );
}

double getConditionNumberOfDecomposedMatrix( const Eigen::JacobiSVD< Eigen::MatrixXd >& singularValueDecomposition )
{
    Eigen::VectorXd singularValues = singularValueDecomposition.singularValues( );
    return singularValues( 0 ) / singularValues( singularValues.rows( ) - 1 );
}

Eigen::JacobiSVD< Eigen::MatrixXd > getSVDDecompositionOfInformationMatrix( const Eigen::MatrixXd& informationMatrix )
{
    return informationMatrix.jacobiSvd( Eigen::ComputeThinU | Eigen::ComputeFullV );
}

Eigen::VectorXd solveSystemOfEquationsWithSvd( const Eigen::MatrixXd matrixToInvert,
                                               const Eigen::VectorXd rightHandSideVector,
                                               const bool checkConditionNumber,
                                               const double maximumAllowedConditionNumber )
{
    Eigen::JacobiSVD< Eigen::MatrixXd > svdDecomposition = matrixToInvert.jacobiSvd( Eigen::ComputeThinU | Eigen::ComputeThinV );
    if( checkConditionNumber )
    {
        double conditionNumber = getConditionNumberOfDecomposedMatrix( svdDecomposition );

        if( conditionNumber > maximumAllowedConditionNumber )
        {
            std::cerr<<"Warning when performing least squares, condition number is "<<conditionNumber<<std::endl;
        }
    }
    return svdDecomposition.solve( rightHandSideVector );
}

Eigen::MatrixXd multiplyInformationMatrixByDiagonalWeightMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix )
{
    Eigen::MatrixXd weightedInformationMatrix = Eigen::MatrixXd::Zero( informationMatrix.rows( ), informationMatrix.cols( ) );

    for( unsigned int i = 0; i < informationMatrix.cols( ); i++ )
    {
        weightedInformationMatrix.block( 0, i, informationMatrix.rows( ), 1 ) =
                informationMatrix.block( 0, i, informationMatrix.rows( ), 1 ).cwiseProduct( diagonalOfWeightMatrix );
    }

    return weightedInformationMatrix;
}

Eigen::MatrixXd calculateCovarianceMatrixWithConsiderParameters(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const Eigen::MatrixXd& inverseOfAPrioriCovarianceMatrix,
        const Eigen::MatrixXd& considerInformationMatrix,
        const Eigen::MatrixXd& considerCovarianceMatrix )
{
    Eigen::MatrixXd noiseOnlyCovariance = calculateInverseOfUpdatedCovarianceMatrix(
                informationMatrix, diagonalOfWeightMatrix, inverseOfAPrioriCovarianceMatrix ).inverse( );
    Eigen::MatrixXd auxiliaryMatrix = noiseOnlyCovariance * multiplyInformationMatrixByDiagonalWeightMatrix(
                informationMatrix, diagonalOfWeightMatrix ).transpose( );

    return noiseOnlyCovariance + ( auxiliaryMatrix * considerInformationMatrix ) * considerCovarianceMatrix *
            ( considerInformationMatrix.transpose( ) * auxiliaryMatrix.transpose( ) );
}

Eigen::MatrixXd calculateInverseOfUpdatedCovarianceMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const Eigen::MatrixXd& inverseOfAPrioriCovarianceMatrix )
{
    //output::writeMatrixToFile( inverseOfAPrioriCovarianceMatrix, "currentInverseAprioriCovarianceMatrix.dat" );
    //output::writeMatrixToFile( informationMatrix.transpose( ) * multiplyInformationMatrixByDiagonalWeightMatrix(
    //                               informationMatrix, diagonalOfWeightMatrix ), "currentInverseNominalCovarianceMatrix.dat" );

    return inverseOfAPrioriCovarianceMatrix + informationMatrix.transpose( ) * multiplyInformationMatrixByDiagonalWeightMatrix(
                informationMatrix, diagonalOfWeightMatrix );
}

Eigen::MatrixXd calculateInverseOfUpdatedCovarianceMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& diagonalOfWeightMatrix )
{
    return calculateInverseOfUpdatedCovarianceMatrix( informationMatrix, diagonalOfWeightMatrix,
                                                      Eigen::MatrixXd::Zero( informationMatrix.cols( ), informationMatrix.cols( ) ) );
}

std::pair< Eigen::VectorXd, Eigen::MatrixXd > performLeastSquaresAdjustmentFromInformationMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& observationResiduals,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const Eigen::MatrixXd& inverseOfAPrioriCovarianceMatrix,
        const Eigen::VectorXd& aPrioriAdjustmentEstimate,
        const bool checkConditionNumber,
        const double maximumAllowedConditionNumber )
{
    Eigen::VectorXd rightHandSide = informationMatrix.transpose( ) *
            ( diagonalOfWeightMatrix.cwiseProduct( observationResiduals ) );
    Eigen::MatrixXd inverseOfCovarianceMatrix = calculateInverseOfUpdatedCovarianceMatrix(
                informationMatrix, diagonalOfWeightMatrix, inverseOfAPrioriCovarianceMatrix );
    return std::make_pair( solveSystemOfEquationsWithSvd( inverseOfCovarianceMatrix, rightHandSide,
                                                          checkConditionNumber, maximumAllowedConditionNumber ), inverseOfCovarianceMatrix );
}

std::pair< Eigen::VectorXd, Eigen::MatrixXd > performLeastSquaresAdjustmentFromInformationMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& observationResiduals,
        const Eigen::VectorXd& diagonalOfWeightMatrix,
        const bool checkConditionNumber,
        const double maximumAllowedConditionNumber )
{
    return performLeastSquaresAdjustmentFromInformationMatrix(
                informationMatrix, observationResiduals, diagonalOfWeightMatrix,
                Eigen::MatrixXd::Zero( informationMatrix.cols( ), informationMatrix.cols( ) ),
                Eigen::VectorXd::Zero( observationResiduals.size( ) ), checkConditionNumber, maximumAllowedConditionNumber );
}

std::pair< Eigen::VectorXd, Eigen::MatrixXd > performLeastSquaresAdjustmentFromInformationMatrix(
        const Eigen::MatrixXd& informationMatrix,
        const Eigen::VectorXd& observationResiduals,
        const bool checkConditionNumber,
        const double maximumAllowedConditionNumber )
{
    return performLeastSquaresAdjustmentFromInformationMatrix(
                informationMatrix, observationResiduals, Eigen::VectorXd::Constant( observationResiduals.size( ), 1, 1.0 ),
                checkConditionNumber, maximumAllowedConditionNumber );
}

Eigen::VectorXd getLeastSquaresPolynomialFit(
        const Eigen::VectorXd& independentValues,
        const Eigen::VectorXd& dependentValues,
        const std::vector< double >& polynomialPowers )
{
    if( independentValues.rows( ) != dependentValues.rows( ) )
    {
        std::cerr<<"Error when doing least squares polynomial fit, size of dependent and independent variable vectors is not equal"<<std::endl;
    }

    Eigen::MatrixXd partialMatrix = Eigen::MatrixXd::Zero( dependentValues.rows( ), polynomialPowers.size( ) );

    for( unsigned int i = 0; i < independentValues.rows( ); i++ )
    {
        for( unsigned int j = 0; j < polynomialPowers.size( ); j++ )
        {
            partialMatrix( i, j ) = std::pow( independentValues( i ), polynomialPowers.at( j ) );
        }
    }

    return performLeastSquaresAdjustmentFromInformationMatrix( partialMatrix, dependentValues ).first;
}

std::vector< double > getLeastSquaresPolynomialFit(
        const std::map< double, double >& independentDependentValueMap,
        const std::vector< double >& polynomialPowers )
{
    return utilities::convertEigenVectorToStlVector(
                getLeastSquaresPolynomialFit(
                    utilities::convertStlVectorToEigenVector(
                        utilities::createVectorFromMapKeys( independentDependentValueMap ) ),
                    utilities::convertStlVectorToEigenVector(
                        utilities::createVectorFromMapValues( independentDependentValueMap ) ), polynomialPowers ) );

}


} // namespace linear_algebra

} // namespace tudat
