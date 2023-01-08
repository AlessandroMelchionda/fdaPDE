#include "../../FdaPDE.h"
#include "../../Skeletons/Include/Regression_Skeleton.h"
#include "../../Skeletons/Include/Regression_Skeleton_Time.h"
#include "../../Skeletons/Include/GAM_Skeleton.h"
#include "../../Skeletons/Include/GAM_Skeleton_time.h"
#include "../../Skeletons/Include/MixedEffects_Skeleton.h"
#include "../Include/Regression_Data.h"
#include "../../FE_Assemblers_Solvers/Include/Integration.h"
#include "../../Lambda_Optimization/Include/Optimization_Data.h"

// GAM + Mixed Effects
#include "../Include/FPIRLS.h"
#include "../Include/FPIRLS_Factory.h"

extern "C"
{
	//! This function manages the various options for Spatial Regression
	/*!
		This function is then called from R code.
		\param Rlocations an R-matrix containing the spatial locations of the observations
		\param RbaryLocations A list with three vectors:
				location points which are same as the given locations options (to checks whether both locations are the same),
		 		a vector of element id of the points from the mesh where they are located,
				a vector of barycenter of points from the located element.
		\param Robservations an R-vector containing the values of the observations.
		\param Rmesh an R-object containg the output mesh from Trilibrary
		\param Rorder an R-integer containing the order of the approximating basis.
		\param Rmydim an R-integer specifying if the mesh nodes lie in R^2 or R^3
		\param Rndim  an R-integer specifying if the "local dimension" is 2 or 3
		\param Rcovariates an R-matrix of covariates for the regression model
		\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
				the other are automatically considered in Neumann Condition.
		\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in RBCIndices
		\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions for the smooth regression with areal data
		\param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		\param Rsearch an R-integer to decide the search algorithm type (tree or naive search algorithm).
		\param Roptim optimzation type, DOF evaluation and loss function used coded as integer vector
		\param Rlambda a vector containing the penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods
		\param Rnrealizations integer, the number of random points used in the stochastic computation of the dofs
		\param Rseed integer, user defined seed for stochastic DOF computation methods
		\param RDOF_matrix user provided DOF matrix for GCV computation
		\param Rtune a R-double, Tuning parameter used for the estimation of GCV. called 'GCV.inflation.factor' in R code.
		\param Rsct user defined stopping criterion tolerance for optimized methods (newton or newton with finite differences)
		\param Rweights an R-vector containing the weights to use for weighted smoothing.
		\return R-vectors containg the coefficients of the solution, prediction of the values, optimization data and much more
	*/
	SEXP regression_Laplace(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rmesh, SEXP Rorder,SEXP Rmydim, SEXP Rndim,
		SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch,
		SEXP Roptim, SEXP Rlambda, SEXP Rnrealizations, SEXP Rseed, SEXP RDOF_matrix, SEXP Rtune, SEXP Rsct, SEXP Rweights)
	{
		//Set input data
		RegressionData regressionData(Rlocations, RbaryLocations, Robservations, Rorder, Rcovariates, RBCIndices, RBCValues, RincidenceMatrix, RarealDataAvg, Rsearch, Rweights);
		OptimizationData optimizationData(Roptim, Rlambda, Rnrealizations, Rseed, RDOF_matrix, Rtune, Rsct);

		UInt mydim = INTEGER(Rmydim)[0];
		UInt ndim = INTEGER(Rndim)[0];

		if(regressionData.getOrder()==1 && mydim==2 && ndim==2)
			return(regression_skeleton<RegressionData, 1, 2, 2>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==2)
			return(regression_skeleton<RegressionData, 2, 2, 2>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==2 && ndim==3)
			return(regression_skeleton<RegressionData, 1, 2, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==3)
			return(regression_skeleton<RegressionData, 2, 2, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==3 && ndim==3)
			return(regression_skeleton<RegressionData, 1, 3, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==3 && ndim==3)
			return(regression_skeleton<RegressionData, 2, 3, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==1 && ndim==2)
            		return(regression_skeleton<RegressionData, 1, 1, 2>(regressionData, optimizationData, Rmesh));
        	else if(regressionData.getOrder()==2 && mydim==1 && ndim==2)
            		return(regression_skeleton<RegressionData, 2, 1, 2>(regressionData, optimizationData, Rmesh));	

		return(NILSXP);
	}

	//! This function manages the various options for Spatio-Temporal Regression
	/*!
		This function is then called from R code.
		\param Rlocations an R-matrix containing the spatial locations of the observations
		\param RbaryLocations A list with three vectors:
				location points which are same as the given locations options (to checks whether both locations are the same),
		 		a vector of element id of the points from the mesh where they are located,
				a vector of barycenter of points from the located element.
		\param Rtime_locations an R-vector containing the temporal locations of the observations
		\param Robservations an R-vector containing the values of the observations.
		\param Rmesh an R-object containing the spatial mesh
		\param Rmesh_time an R-vector containing the temporal mesh
		\param Rorder an R-integer containing the order of the approximating basis in space.
		\param Rmydim an R-integer specifying if the mesh nodes lie in R^2 or R^3
		\param Rndim  an R-integer specifying if the "local dimension" is 2 or 3
		\param Rcovariates an R-matrix of covariates for the regression model
		\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
				the other are automatically considered in Neumann Condition.
		\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in RBCIndices
		\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions for the smooth regression with areal data
		\param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		\param Rflag_mass an R-integer that in case of separable problem specifies whether to use mass discretization or identity discretization
		\param Rflag_parabolic an R-integer specifying if the problem is parabolic or separable
		\param Rflag_iterative an R-integer specifying if the method is monolithic or iterative
	    \param Rmax_num_iteration Maximum number of steps run in the iterative algorithm, set to 50 by default.
		\param Rtreshold an R-double used for arresting the iterative algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		\param Ric an R-vector containing the initial condition needed in case of parabolic problem
		\param Rsearch an R-integer to decide the search algorithm type (tree or naive search algorithm).
		\param Roptim optimzation type, DOF evaluation and loss function used coded as integer vector
		\param Rlambda_S a vector containing the penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods
		\param Rlambda_T a vector containing the temporal penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods in eparable context
		\param Rnrealizations integer, the number of random points used in the stochastic computation of the dofs
		\param Rseed integer, user defined seed for stochastic DOF computation methods
		\param RDOF_matrix user provided DOF matrix for GCV computation
		\param Rtune a R-double, Tuning parameter used for the estimation of GCV. called 'GCV.inflation.factor' in R code.
		\param Rsct user defined stopping criterion tolerance for optimized methods (newton or newton with finite differences)
		\param Rweights an R-vector containing the weights to use for weighted smoothing.
		\return R-vectors containg the coefficients of the solution, prediction of the values, optimization data and much more
	*/
	SEXP regression_Laplace_time(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rmesh, SEXP Rmesh_time, SEXP Rorder, SEXP Rmydim, SEXP Rndim,
		SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,  SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic,SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rtreshold, SEXP Ric, SEXP Rsearch,
		SEXP Roptim, SEXP Rlambda_S, SEXP Rlambda_T, SEXP Rnrealizations, SEXP Rseed, SEXP RDOF_matrix, SEXP Rtune, SEXP Rsct, SEXP Rweights)
	{
	    	//Set input data
		RegressionData regressionData(Rlocations, RbaryLocations, Rtime_locations, Robservations, Rorder, Rcovariates, RBCIndices, RBCValues,
			RincidenceMatrix, RarealDataAvg, Rflag_mass, Rflag_parabolic,Rflag_iterative, Rmax_num_iteration, Rtreshold, Ric, Rsearch, Rweights);
		OptimizationData optimizationData(Roptim, Rlambda_S, Rlambda_T, Rflag_parabolic, Rnrealizations, Rseed, RDOF_matrix, Rtune, Rsct);

		UInt mydim = INTEGER(Rmydim)[0];
		UInt ndim = INTEGER(Rndim)[0];

		if(regressionData.getOrder()==1 && mydim==2 && ndim==2)
			return(regression_skeleton_time<RegressionData, 1, 2, 2>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==2)
			return(regression_skeleton_time<RegressionData, 2, 2, 2>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==1 && mydim==2 && ndim==3)
			return(regression_skeleton_time<RegressionData, 1, 2, 3>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==3)
			return(regression_skeleton_time<RegressionData, 2, 2, 3>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==1 && mydim==3 && ndim==3)
			return(regression_skeleton_time<RegressionData, 1, 3, 3>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==2 && mydim==3 && ndim==3)
			return(regression_skeleton_time<RegressionData, 2, 3, 3>(regressionData, optimizationData, Rmesh, Rmesh_time));
		else if(regressionData.getOrder()==1 && mydim==1 && ndim==2)
            		return(regression_skeleton_time<RegressionData, 1, 1, 2>(regressionData, optimizationData, Rmesh, Rmesh_time));
        	else if(regressionData.getOrder()==2 && mydim==1 && ndim==2)
            		return(regression_skeleton_time<RegressionData, 2, 1, 2>(regressionData, optimizationData, Rmesh, Rmesh_time));
            	
            	return(NILSXP);
	}

	//! This function manages the various options for GAM Spatial Regression
	/*!
		This function is then called from R code.
		\param Rlocations an R-matrix containing the spatial locations of the observations
		\param RbaryLocations A list with three vectors:
				location points which are same as the given locations options (to checks whether both locations are the same),
				a vector of element id of the points from the mesh where they are located,
				a vector of barycenter of points from the located element.
		\param Robservations an R-vector containing the values of the observations.
		\param Rmesh an R-object containg the output mesh from Trilibrary
		\param Rorder an R-integer containing the order of the approximating basis.
		\param Rmydim an R-integer specifying if the mesh nodes lie in R^2 or R^3
		\param Rndim  an R-integer specifying if the "local dimension" is 2 or 3
		\param Rcovariates an R-matrix of covariates for the regression model
		\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
				the other are automatically considered in Neumann Condition.
		\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in RBCIndices
		\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions for the smooth regression with areal data
		\param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		\param Rfamily Denotes the distribution of the data, within the exponential family.
		\param Rmax_num_iteration Maximum number of steps run in the PIRLS algorithm, set to 15 by default.
		\param Rtreshold an R-double used for arresting FPIRLS algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		\param Rtune It is usually set to 1, but may be higher. It gives more weight to the equivalent degrees of freedom in the computation of the value of the GCV.
		\param Rmu0 Initial value of the mean (natural parameter). There exists a default value for each familiy
		\param RscaleParam If necessary and not supplied, the scale parameter \phi is estimated. See method.phi for details.
		\param Rsearch an R-integer to decide the search algorithm type (tree or naive search algorithm).
		\param Roptim optimzation type, DOF evaluation and loss function used coded as integer vector
		\param Rlambda a vector containing the penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods
		\param Rnrealizations integer, the number of random points used in the stochastic computation of the dofs
		\param Rseed integer, user defined seed for stochastic DOF computation methods
		\param RDOF_matrix user provided DOF matrix for GCV computation
		\param Rtune a R-double, Tuning parameter used for the estimation of GCV. called 'GCV.inflation.factor' in R code.
		\param Rsct user defined stopping criterion tolerance for optimized methods (newton or newton with finite differences)
		\return R-vectors containg the coefficients of the solution, prediction of the values, optimization data and much more
	*/
	 SEXP gam_Laplace(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rmesh, SEXP Rorder,SEXP Rmydim, SEXP Rndim,
		SEXP Rcovariates,  SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
		SEXP Rfamily, SEXP Rmax_num_iteration, SEXP Rtreshold, SEXP Rmu0, SEXP RscaleParam, SEXP Rsearch,
		SEXP Roptim, SEXP Rlambda, SEXP Rnrealizations, SEXP Rseed, SEXP RDOF_matrix, SEXP Rtune, SEXP Rsct)
	{
	    	// Set up the GAMdata structure for the laplacian case
		GAMDataLaplace regressionData(Rlocations, RbaryLocations, Robservations, Rorder, Rcovariates, RBCIndices, RBCValues, RincidenceMatrix, RarealDataAvg, Rsearch, Rmax_num_iteration, Rtreshold);
		OptimizationData optimizationData(Roptim, Rlambda, Rnrealizations, Rseed, RDOF_matrix, Rtune, Rsct);

	 	UInt mydim = INTEGER(Rmydim)[0]; // Set the mesh dimension form R to C++
		UInt ndim = INTEGER(Rndim)[0]; // Set the mesh space dimension form R to C++

	  	std::string family = CHAR(STRING_ELT(Rfamily,0));

		if(regressionData.getOrder()==1 && mydim==2 && ndim==2)
			return(GAM_skeleton<GAMDataLaplace, 1, 2, 2>(regressionData, optimizationData, Rmesh, Rmu0 , family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==2)
			return(GAM_skeleton<GAMDataLaplace, 2, 2, 2>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==2 && ndim==3)
			return(GAM_skeleton<GAMDataLaplace, 1, 2, 3>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==3)
			return(GAM_skeleton<GAMDataLaplace, 2, 2, 3>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==3 && ndim==3)
			return(GAM_skeleton<GAMDataLaplace, 1, 3, 3>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==3 && ndim==3)
			return(GAM_skeleton<GAMDataLaplace, 2, 3, 3>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==1 && ndim==2)
            		return(GAM_skeleton<GAMDataLaplace, 1, 1, 2>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));
        	else if(regressionData.getOrder()==2 && mydim==1 && ndim==2)
            		return(GAM_skeleton<GAMDataLaplace, 2, 1, 2>(regressionData, optimizationData, Rmesh, Rmu0, family, RscaleParam));

	return(R_NilValue);
	}
	
	//! This function manages the various options for GAM Spatio-Temporal Regression
	/*!
		This function is then called from R code.
		\param Rlocations an R-matrix containing the spatial locations of the observations
		\param Rtime_locations an R-vector containing the temporal locations of the observations
		\param RbaryLocations A list with three vectors:
				location points which are same as the given locations options (to checks whether both locations are the same),
				a vector of element id of the points from the mesh where they are located,
				a vector of barycenter of points from the located element.
		\param Robservations an R-vector containing the values of the observations.
		\param Rmesh an R-object containg the output mesh from Trilibrary
		\param Rmesh_time an R-vector containing the temporal mesh
		\param Rorder an R-integer containing the order of the approximating basis.
		\param Rmydim an R-integer specifying if the mesh nodes lie in R^2 or R^3
		\param Rndim  an R-integer specifying if the "local dimension" is 2 or 3
		\param Rcovariates an R-matrix of covariates for the regression model
		\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
				the other are automatically considered in Neumann Condition.
		\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in RBCIndices
		\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions for the smooth regression with areal data
		\param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		\param Rflag_mass an R-integer that in case of separable problem specifies whether to use mass discretization or identity discretization
		\param Rflag_parabolic an R-integer specifying if the problem is parabolic or separable
		\param Rflag_iterative an R-integer specifying if the method is monolithic or iterative
		\param Rmax_num_iteration Maximum number of steps run in the PIRLS algorithm, set to 15 by default.
		\param Rtreshold an R-double used for arresting FPIRLS algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		\param Rfamily Denotes the distribution of the data, within the exponential family.
		\param Rmax_num_iteration_pirls Maximum number of steps run in the PIRLS algorithm, set to 15 by default.
		\param Rtreshold_pirls an R-double used for arresting the iterative algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		\param Rmu0 Initial value of the mean (natural parameter). There exists a default value for each familiy
		\param RscaleParam If necessary and not supplied, the scale parameter \phi is estimated. See method.phi for details.
		\param Rsearch an R-integer to decide the search algorithm type (tree or naive search algorithm).
		\param Roptim optimzation type, DOF evaluation and loss function used coded as integer vector
		\param Rlambda_S a vector containing the penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods
		\param Rlambda_T a vector containing the temporal penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods in separable context
		\param Rnrealizations integer, the number of random points used in the stochastic computation of the dofs
		\param Rseed integer, user defined seed for stochastic DOF computation methods
		\param RDOF_matrix user provided DOF matrix for GCV computation
		\param Rtune a R-double, Tuning parameter used for the estimation of GCV. called 'GCV.inflation.factor' in R code.
		\param Rsct user defined stopping criterion tolerance for optimized methods (newton or newton with finite differences)
		\return R-vectors containg the coefficients of the solution, prediction of the values, optimization data and much more
	*/
	SEXP gam_Laplace_time(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rmesh, SEXP Rmesh_time, 
		SEXP Rorder, SEXP Rmydim, SEXP Rndim, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,  SEXP RincidenceMatrix, 
		SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic,SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, 
		SEXP Ric, SEXP Rfamily, SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls, SEXP Rmu0, SEXP RscaleParam, SEXP Rsearch,
		SEXP Roptim, SEXP Rlambda_S, SEXP Rlambda_T, SEXP Rnrealizations, SEXP Rseed, SEXP RDOF_matrix, SEXP Rtune, SEXP Rsct)
	{
	    	//Set input data
		GAMDataLaplace regressionData(Rlocations, RbaryLocations, Rtime_locations, Robservations, Rorder, Rcovariates, RBCIndices, RBCValues,
            RincidenceMatrix, RarealDataAvg, Rflag_mass, Rflag_parabolic,
            Rflag_iterative, Rmax_num_iteration, Rthreshold, Ric, Rsearch,
            Rmax_num_iteration_pirls, Rthreshold_pirls);
		OptimizationData optimizationData(Roptim, Rlambda_S, Rlambda_T, Rflag_parabolic, Rnrealizations, Rseed, RDOF_matrix, Rtune, Rsct);
	  	std::string family = CHAR(STRING_ELT(Rfamily,0));

		UInt mydim = INTEGER(Rmydim)[0];
		UInt ndim = INTEGER(Rndim)[0];

		if(regressionData.getOrder()==1 && mydim==2 && ndim==2)
			return(GAM_skeleton_time<GAMDataLaplace, 1, 2, 2>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==2)
			return(GAM_skeleton_time<GAMDataLaplace, 2, 2, 2>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==2 && ndim==3)
			return(GAM_skeleton_time<GAMDataLaplace, 1, 2, 3>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==3)
			return(GAM_skeleton_time<GAMDataLaplace, 2, 2, 3>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==3 && ndim==3)
			return(GAM_skeleton_time<GAMDataLaplace, 1, 3, 3>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==3 && ndim==3)
			return(GAM_skeleton_time<GAMDataLaplace, 2, 3, 3>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==1 && mydim==1 && ndim==2)
			return(GAM_skeleton_time<GAMDataLaplace, 1, 1, 2>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));
		else if(regressionData.getOrder()==2 && mydim==1 && ndim==2)
			return(GAM_skeleton_time<GAMDataLaplace, 2, 1, 2>(regressionData, optimizationData, Rmesh, Rmesh_time, Rmu0, family, RscaleParam));	

	    	return(NILSXP);
	}

	//! This function manages the various options for Mixed Effects Spatial Regression
	/*!
		This function is then called from R code.
		\param Rlocations an R-matrix containing the spatial locations of the observations
		\param RbaryLocations A list with three vectors:
				location points which are same as the given locations options (to checks whether both locations are the same),
				a vector of element id of the points from the mesh where they are located,
				a vector of barycenter of points from the located element.
		\param Robservations an R-vector containing the values of the observations.
		\param Rmesh an R-object containg the output mesh from Trilibrary
		\param Rorder an R-integer containing the order of the approximating basis.
		\param Rmydim an R-integer specifying if the mesh nodes lie in R^2 or R^3
		\param Rndim  an R-integer specifying if the "local dimension" is 2 or 3
		\param Rcovariates an R-matrix of covariates for the regression model
		\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
				the other are automatically considered in Neumann Condition.
		\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in RBCIndices
		\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions for the smooth regression with areal data
		\param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		\param Rmax_num_iteration Maximum number of steps run in the PIRLS algorithm, set to 15 by default.
		\param Rtreshold an R-double used for arresting FPIRLS algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		\param Rtune It is usually set to 1, but may be higher. It gives more weight to the equivalent degrees of freedom in the computation of the value of the GCV.
		\param Rsearch an R-integer to decide the search algorithm type (tree or naive search algorithm).
		\param Roptim optimzation type, DOF evaluation and loss function used coded as integer vector
		\param Rlambda a vector containing the penalization term of the empirical evidence respect to the prior one. or initial codition for optimized methods
		\param Rnrealizations integer, the number of random points used in the stochastic computation of the dofs
		\param Rseed integer, user defined seed for stochastic DOF computation methods
		\param RDOF_matrix user provided DOF matrix for GCV computation
		\param Rtune a R-double, Tuning parameter used for the estimation of GCV. called 'GCV.inflation.factor' in R code.
		\param Rsct user defined stopping criterion tolerance for optimized methods (newton or newton with finite differences)
		\param Rrandom_effects_covariates an R-matrix of Random Effects covariates for the mixed Effects Regression Model
		\param Rgroup_sizes an R-vector containing the size of each group
		\param Rn_groups an R-integer storing the number of groups
		\return R-vectors containg the coefficients of the solution, prediction of the values, optimization data and much more
	*/
	 SEXP MixedEffects_Laplace(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rmesh, SEXP Rorder,SEXP Rmydim, SEXP Rndim,
		SEXP Rcovariates,  SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
		SEXP Rmax_num_iteration, SEXP Rtreshold, SEXP Rsearch,
		SEXP Roptim, SEXP Rlambda, SEXP Rnrealizations, SEXP Rseed, SEXP RDOF_matrix, SEXP Rtune, SEXP Rsct,
		SEXP Rrandom_effects_covariates, SEXP Rgroup_sizes, SEXP Rn_groups)
	{
	    	// Set up the MixedEffectsata structure for the laplacian case
		MixedEffectsDataLaplace regressionData(Rlocations, RbaryLocations, Robservations, Rorder, 
							 Rcovariates, RBCIndices, RBCValues, RincidenceMatrix, RarealDataAvg, Rsearch, 
							 Rmax_num_iteration, Rtreshold,
							 Rrandom_effects_covariates, Rgroup_sizes, Rn_groups);
		OptimizationData optimizationData(Roptim, Rlambda, Rnrealizations, Rseed, RDOF_matrix, Rtune, Rsct);

	 	UInt mydim = INTEGER(Rmydim)[0]; // Set the mesh dimension form R to C++
		UInt ndim = INTEGER(Rndim)[0]; // Set the mesh space dimension form R to C++

		std::cout << "memory allocated" << std::endl;

		if(regressionData.getOrder()==1 && mydim==2 && ndim==2)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 1, 2, 2>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==2)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 2, 2, 2>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==2 && ndim==3)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 1, 2, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==2 && ndim==3)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 2, 2, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==3 && ndim==3)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 1, 3, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==2 && mydim==3 && ndim==3)
			return(MixedEffects_skeleton<MixedEffectsDataLaplace, 2, 3, 3>(regressionData, optimizationData, Rmesh));
		else if(regressionData.getOrder()==1 && mydim==1 && ndim==2)
    		return(MixedEffects_skeleton<MixedEffectsDataLaplace, 1, 1, 2>(regressionData, optimizationData, Rmesh));
    	else if(regressionData.getOrder()==2 && mydim==1 && ndim==2)
    		return(MixedEffects_skeleton<MixedEffectsDataLaplace, 2, 1, 2>(regressionData, optimizationData, Rmesh));

	return(R_NilValue);
	}
}
