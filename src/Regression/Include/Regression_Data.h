#ifndef __REGRESSION_DATA_H__
#define __REGRESSION_DATA_H__

#include "../../FdaPDE.h"
#include "../../Mesh/Include/Mesh_Objects.h"
#include "../../FE_Assemblers_Solvers/Include/Param_Functors.h"

//!  An IO handler class for objects passed from R
/*!
 * This class, given the data from R, convert them in a C++ format, offering a
 * series of method for their access, so isolating the more the possible the specific
 * code for R/C++ data conversion.
*/
class  RegressionData
{
	protected:
		const RNumericMatrix locations_;		//!< Design matrix pointer and dimensions.
		VectorXr 	   observations_; 		//!< Observations data
		bool 		   locations_by_nodes_{}; 	//!< If location is on the mesh nodes or not.
		UInt 		   nRegions_ = 0; 		//!< For areal data.
		bool 		   arealDataAvg_{}; 		//!< Is areal data averaged ?
		SpMat	   	   WeightsMatrix_; 		//!< Weighted regression.
		bool           	   isFPIRLS = false;


		// Design matrix
		MatrixXr covariates_;
		UInt n_ = 0;
		UInt p_ = 0;


	private:
		std::vector<UInt> observations_indices_;
		std::vector<UInt> observations_na_;

		std::vector<Real> time_locations_;		//!< Vector of the time locations

		// Barycenter information
		VectorXi element_ids_; 				//!< Elements id information
		MatrixXr barycenters_; 				//!< Barycenter information
		bool locations_by_barycenter_{};

		// Other parameters
		UInt order_ = 0;

		// Boundary + Initial
		std::vector<Real> bc_values_;
		std::vector<UInt> bc_indices_;
		VectorXr ic_; 					//!< Initial conditions

		// Areal data
		MatrixXi incidenceMatrix_;

		bool flag_mass_{};				//!< Mass penalization, only for separable version (flag_parabolic_==FALSE)
		bool flag_parabolic_{};
		bool flag_iterative_{};     			//!<True if iterative-method for space time smoothing is selected
		bool flag_SpaceTime_{}; 			// TRUE if space time smoothing
		UInt search_ = 0; 				// search algorith type

        	// Iterative method
        	UInt max_num_iterations_ = 0; 			//!< Max number of iterations allowed
        	Real threshold_ = 0.; 				//!< Limit in difference among J_k and J_k+1 for which we stop iterative method.

		// -- SETTERS --
		void setObservations(SEXP Robservations);
		void setObservationsTime(SEXP Robservations);
		void setBaryLocations(SEXP RbaryLocations);
		void setTimeLocations(SEXP Rtime_locations);
		void setCovariates(SEXP Rcovariates);
		void setIncidenceMatrix(SEXP RincidenceMatrix);
		void setWeights(SEXP Rweights);
		void setWeightsTime(SEXP Rweights);

	public:
		// -- CONSTRUCTORS --

		//! A basic version of the constructor.
		/*!
			It initializes the object storing the R given objects. This is the simplest of the two possible interfaces with R
			\param Rlocations an R-matrix containing the location of the observations.
			\param Robservations an R-vector containing the values of the observations.
			\param Rorder an R-integer containing the order of the approximating basis.
			\param RlambdaS an R-double containing the penalization term of the empirical evidence respect to the prior one.
			\param Rcovariates an R-matrix storing the covariates of the regression
			\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions in the model with areal data
			\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
					the other are automatically considered in Neumann Condition.
			\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in Rbindex
			\param GCV an R boolean indicating whether GCV has to be computed or not
			\param DOF an R boolean indicating whether dofs of the model have to be computed or not
		        \param RGCVmethod an R-integer indicating the method to use to compute the dofs when DOF is TRUE, can be either 1 (exact) or 2 (stochastic)
		        \param Rnrealizations the number of random points used in the stochastic computation of the dofs
		        \param Rsearch an R-integer to decide the search algorithm type (tree or naive or walking search algorithm).
		        \param Rtune an R-double parameter used in the computation of the GCV. The default value is 1.
		        \param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
			\param Rweights an R-vector containing the weights to use for weighted smoothing.


		*/
		explicit RegressionData(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder, SEXP Rcovariates,
			SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch);

		explicit RegressionData(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder, SEXP Rcovariates,
			SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, SEXP Rweights);

		explicit RegressionData(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder, SEXP Rcovariates,
			SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative,SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch);
		
		explicit RegressionData(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder, SEXP Rcovariates,
					SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative,SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, SEXP Rweights);

		explicit RegressionData(Real* locations, UInt n_locations, UInt ndim, VectorXr & observations, UInt order, MatrixXr & covariates,
			 SpMat & WeightsMatrix, std::vector<UInt> & bc_indices, std::vector<Real> & bc_values,  MatrixXi & incidenceMatrix, bool arealDataAvg, UInt search);

		// -- PRINTERS --
		void printObservations(std::ostream & out) const;
		void printCovariates(std::ostream & out) const;
		void printLocations(std::ostream & out) const;
		void printIncidenceMatrix(std::ostream & out) const;

		// -- GETTERS --
		// Observations [[GM passng to const pointers??]]
		//! A method returning a const pointer to the observations vector
		const VectorXr * getObservations(void) const {return &observations_;}
		//! A method returning the number of observations
		UInt getNumberofObservations(void) const {return observations_.size();}
		//! A method returning the number of space observations
		UInt getNumberofSpaceObservations(void) const
			{return observations_.size()/(time_locations_.size()==0 ? 1:time_locations_.size() );}
		//! A method returning the number of time observations

		UInt getNumberofTimeObservations(void) const {return time_locations_.size();}
		const std::vector<UInt> * getObservationsIndices(void) const {return &observations_indices_;}
		const std::vector<UInt> * getObservationsNA(void) const {return &observations_na_;}
        //! A method returning the maximum iteration for the iterative method
        const UInt get_maxiter() const {return max_num_iterations_;}
        //! A method returning the treshold (iterative methos)
        const Real get_treshold() const {return threshold_;}

		// Locations [[GM passng to const pointers??]]
		//! A method returning the locations of the observations
		template<UInt ndim>
		Point<ndim> getLocations(UInt i) const {return Point<ndim>(i, locations_);}
		//! A method returning the locations of the time observations
		std::vector<Real> const & getTimeLocations(void) const {return time_locations_;}
		bool isLocationsByNodes(void) const {return locations_by_nodes_;}
		bool isLocationsByBarycenter(void) const {return locations_by_barycenter_;}
		MatrixXr const & getBarycenters(void) const {return barycenters_;} 	//not pointer to avoid compilation error in templates, not used in mixedFERegression
		VectorXi const & getElementIds(void) const {return element_ids_;} 	//not pointer to avoid compilation error in templates, not used in mixedFERegression
		Real getBarycenter(int i, int j) const {return barycenters_(i,j);}
		UInt getElementId(Id i) const {return element_ids_(i);}

		// Covariates
		//! A method returning a const pointer to the design matrix
		const MatrixXr * getCovariates(void) const {return &covariates_;}

		// Bounday + Initial
		//! A method returning the indexes of the nodes for which is needed to apply Dirichlet Conditions
		const std::vector<UInt> * getDirichletIndices(void) const {return &bc_indices_;}
		//! A method returning the values to apply for Dirichlet Conditions
		const std::vector<Real> * getDirichletValues(void) const {return &bc_values_;}
		//! A method returning the values to apply for Initial Conditions
		const VectorXr * getInitialValues(void) const {return &ic_;}

		// Areal
		//! A method returning a const pointer to the incidence matrix
		const MatrixXi * getIncidenceMatrix(void) const {return &incidenceMatrix_;}
		//! A method returning the number of regions
		UInt getNumberOfRegions(void) const {return nRegions_;}
		bool isArealDataAvg(void) const {return arealDataAvg_;}

		//! A method returning the input order
		UInt getOrder(void) const {return order_;}

		//! A method returning a const pointer to the matrix of weights
		const SpMat * getWeightsMatrix(void) const {return &WeightsMatrix_;}

        bool isSpaceTime(void) const {return flag_SpaceTime_;}
		bool getFlagMass(void) const {return flag_mass_;}
		bool getFlagParabolic(void) const {return flag_parabolic_;}
        bool getFlagIterative(void) const {return flag_iterative_;}
		bool getisFPIRLS(void) const {return isFPIRLS;}

		// Search
		//! A method returning the input search
		UInt getSearch(void) const {return search_;}
};


class  RegressionDataElliptic:public RegressionData
{
	private:
		Diffusion<PDEParameterOptions::Constant> K_;
		Advection<PDEParameterOptions::Constant> beta_;
		Real c_;

	public:
		//! A complete version of the constructor.
		/*!
			It initializes the object storing the R given objects. This is the simplest of the two possible interfaces with R
			\param Rlocations an R-matrix containing the location of the observations.
			\param Robservations an R-vector containing the values of the observations.
			\param Rorder an R-integer containing the order of the approximating basis.
			\param RlambdaS an R-double containing the penalization term of the empirical evidence respect to the prior one.
			\param RK an R-double 2X2 matrix containing the coefficients for a anisotropic DIFFUSION term.
			\param Rbeta an R-double 2-dim vector that contains the coefficients for the TRANSPORT coefficients.
			\param Rc an R-double that contains the coefficient of the REACTION term
			\param Rcovariates an R-matrix storing the covariates of the regression
			\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions in the model with areal data
			\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
					the other are automatically considered in Neumann Condition.
			\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in Rbindex
			\param DOF an R boolean indicating whether dofs of the model have to be computed or not
		        \param RGCVmethod an R-integer indicating the method to use to compute the dofs when DOF is TRUE, can be either 1 (exact) or 2 (stochastic)
		        \param Rnrealizations the number of random points used in the stochastic computation of the dofs
		        \param Rsearch an R-integer to decide the search algorithm type (tree or naive or walking search algorithm).
            \param Rweights an R-vector containing the weights to use for weighted smoothing.
		*/
		explicit RegressionDataElliptic(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			 SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			 SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch);

		explicit RegressionDataElliptic(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			 SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			 SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, SEXP Rweights);

		explicit RegressionDataElliptic(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch);
		
		explicit RegressionDataElliptic(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
					SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
					SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, SEXP Rweights);

		Diffusion<PDEParameterOptions::Constant> const & getK() const {return K_;}
		Advection<PDEParameterOptions::Constant> const & getBeta() const {return beta_;}
		Real const getC() const {return c_;}
};

class RegressionDataEllipticSpaceVarying:public RegressionData
{
	private:
		Diffusion<PDEParameterOptions::SpaceVarying> K_;
		Advection<PDEParameterOptions::SpaceVarying> beta_;
		Reaction c_;
		ForcingTerm u_;

	public:

		//! A complete version of the constructor.
		/*!
			It initializes the object storing the R given objects. This is the simplest of the two possible interfaces with R
			\param Rlocations an R-matrix containing the location of the observations.
			\param Robservations an R-vector containing the values of the observations.
			\param Rorder an R-integer containing the order of the approximating basis.
			\param RlambdaS an R-double containing the penalization term of the empirical evidence respect to the prior one.
			\param RK an R-double 2X2 matrix containing the coefficients for a anisotropic DIFFUSION term.
			\param Rbeta an R-double 2-dim vector that contains the coefficients for the TRANSPORT coefficients.
			\param Rc an R-double that contains the coefficient of the REACTION term
			\param  Ru an R-double vector of length #triangles that contaiins the forcing term integrals.
			\param Rcovariates an R-matrix storing the covariates of the regression
			\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions in the model with areal data
			\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
					the other are automatically considered in Neumann Condition.
			\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in Rbindex
			\param DOF an R boolean indicating whether dofs of the model have to be computed or not
	        	\param RGCVmethod an R-integer indicating the method to use to compute the dofs when DOF is TRUE, can be either 1 (exact) or 2 (stochastic)
	        	\param Rnrealizations the number of random points used in the stochastic computation of the dofs
	        	\param Rsearch an R-integer to decide the search algorithm type (tree or naive or walking search algorithm).
            \param Rweights an R-vector containing the weights to use for weighted smoothing.
		*/
		explicit RegressionDataEllipticSpaceVarying(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch);

		explicit RegressionDataEllipticSpaceVarying(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, SEXP Rweights);

		explicit RegressionDataEllipticSpaceVarying(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch);
		
		explicit RegressionDataEllipticSpaceVarying(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
					SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
					SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, SEXP Rweights);

		Diffusion<PDEParameterOptions::SpaceVarying> const & getK() const {return K_;}
		Advection<PDEParameterOptions::SpaceVarying> const & getBeta() const {return beta_;}
		Reaction const & getC() const {return c_;}
		ForcingTerm const & getU() const {return u_;}
};

//----------------------------------------------------------------------------//
// ------------------------------   GAM DATA ---------------------------------//
//----------------------------------------------------------------------------//
/*! @brief A class that stores the data for the Generalized Additive Models.
 *
 *	It is a derived class of the RegressionHandler data type. It can be: RegressionData, RegressionDataElliptic, or RegressionDataEllipticSpaceVarying
 */
template<typename RegressionHandler>
class  RegressionDataGAM : public RegressionHandler
{
	private:

		VectorXr initialObservations_; //!< A copy of the true observations, which will not be overriden during FPIRLS algorithm.
		std::vector<UInt> initial_observations_indexes_;
		UInt max_num_iterations_; //!< Max number of iterations allowed.
		Real threshold_; //!< Limit in difference among J_k and J_k+1 for which we stop FPIRLS.
		
		// Constructor utilities
		void initializeWeights(void);

	public:
		//! A complete version of the constructor.
		/*!
			It initializes the object storing the R given objects. This is the simplest of the two possible interfaces with R
			\param Rlocations an R-matrix containing the location of the observations.
			\param Robservations an R-vector containing the values of the observations.
			\param Rorder an R-integer containing the order of the approximating basis.
			\param RlambdaS an R-double containing the penalization term of the empirical evidence respect to the prior one.
			\param RK an R-double 2X2 matrix containing the coefficients for a anisotropic DIFFUSION term.
			\param Rbeta an R-double 2-dim vector that contains the coefficients for the TRANSPORT coefficients.
			\param Rc an R-double that contains the coefficient of the REACTION term
			\param Rcovariates an R-matrix storing the covariates of the regression
			\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions in the model with areal data
			\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
					the other are automatically considered in Neumann Condition.
			\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in Rbindex
			\param DOF an R boolean indicating whether dofs of the model have to be computed or not
		        \param RGCVmethod an R-integer indicating the method to use to compute the dofs when DOF is TRUE, can be either 1 (exact) or 2 (stochastic)
		        \param Rnrealizations the number of random points used in the stochastic computation of the dofs
		        \param Rmax_num_iteration an R-integer indicating the max number of steps for the FPIRLS algorithm
		        \param Rthreshold an R-double used for arresting FPIRLS algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		        \param Rtune an R-double parameter used in the computation of the GCV. The default value is 1.
		        \param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
		*/

		//Laplace
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch,
			SEXP Rmax_num_iteration, SEXP Rthreshold);

		// PDE
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, SEXP Rmax_num_iteration, SEXP Rthreshold);

		// PDE SpaceVarying
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, SEXP Rmax_num_iteration, SEXP Rthreshold);

		//Laplace time
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls);
		
		// PDE time
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls);
		
		// PDE SpaceVarying time
		explicit RegressionDataGAM(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls);

		//! A method returning the maximum iteration for the iterative method
		UInt get_maxiter() const {return max_num_iterations_;}
		//! A method returning the treshold
		Real get_treshold() const {return threshold_;}
		//! A method returning a reference to the observations vector
		const VectorXr * getInitialObservations() const {return &initialObservations_;}
		//! A method returning the lambda used in the GAM data
		UInt getNumberofInitialObservations() const {return initial_observations_indexes_.size();}

		//! Update Pseudodata (observations and weights)
		void updatePseudodata(VectorXr& z_, VectorXr& P);
};


// Type definitions for the GAMdata Structure
/** GAMDataLaplace type definition */
typedef RegressionDataGAM<RegressionData> GAMDataLaplace;
/** GAMDataElliptic type definition */
typedef RegressionDataGAM<RegressionDataElliptic> GAMDataElliptic;
/**  GAMDataEllipticSpaceVarying type definition */
typedef RegressionDataGAM<RegressionDataEllipticSpaceVarying> GAMDataEllipticSpaceVarying;



//------------------------------------------------------------------------------------//
// ------------------------------ Mixed Effects DATA ---------------------------------//
//------------------------------------------------------------------------------------//
/*! @brief A class that stores the data for the Mixed Effects Models.
 *
 *	It is a derived class of the RegressionHandler data type. It can be: RegressionData, RegressionDataElliptic, or RegressionDataEllipticSpaceVarying
 */
template<typename RegressionHandler>
class  RegressionDataMixedEffects : public RegressionHandler
{
	private:

		// General FPIRLS quantities
		UInt max_num_iterations_; //!< Max number of iterations allowed.
		Real threshold_; //!< Limit in difference among J_k and J_k+1 for which we stop FPIRLS.
		
		// Mixed Effects specific quantities
		MatrixXr random_effects_covariates_;	
		UInt m_ = 0;	
		UInt q_ = 0;
		UInt n_groups_ = 0;
		std::vector<UInt> group_sizes_;
		std::vector<std::vector<UInt>> ids_perm_;
		
		// Constructor utilities
		void setRandomEffectsCovariates(SEXP Rrandom_effects_covariates);
		void setGroupSizes_and_Perm(SEXP Rgroup_ids);
		void initializeWeights(void);

	public:
		//! A complete version of the constructor.
		/*!
			It initializes the object storing the R given objects. This is the simplest of the two possible interfaces with R
			\param Rlocations an R-matrix containing the location of the observations.
			\param Robservations an R-vector containing the values of the observations.
			\param Rorder an R-integer containing the order of the approximating basis.
			\param RlambdaS an R-double containing the penalization term of the empirical evidence respect to the prior one.
			\param RK an R-double 2X2 matrix containing the coefficients for a anisotropic DIFFUSION term.
			\param Rbeta an R-double 2-dim vector that contains the coefficients for the TRANSPORT coefficients.
			\param Rc an R-double that contains the coefficient of the REACTION term
			\param Rcovariates an R-matrix storing the covariates of the regression
			\param RincidenceMatrix an R-matrix containing the incidence matrix defining the regions in the model with areal data
			\param RBCIndices an R-integer containing the indexes of the nodes the user want to apply a Dirichlet Condition,
					the other are automatically considered in Neumann Condition.
			\param RBCValues an R-double containing the value to impose for the Dirichlet condition, on the indexes specified in Rbindex
			\param DOF an R boolean indicating whether dofs of the model have to be computed or not
		        \param RGCVmethod an R-integer indicating the method to use to compute the dofs when DOF is TRUE, can be either 1 (exact) or 2 (stochastic)
		        \param Rnrealizations the number of random points used in the stochastic computation of the dofs
		        \param Rmax_num_iteration an R-integer indicating the max number of steps for the FPIRLS algorithm
		        \param Rthreshold an R-double used for arresting FPIRLS algorithm. Algorithm stops when two successive iterations lead to improvement in penalized log-likelihood smaller than threshold.
		        \param Rtune an R-double parameter used in the computation of the GCV. The default value is 1.
		        \param RarealDataAvg an R boolean indicating whether the areal data are averaged or not.
			\param Rrandom_effects_covariates an R-matrix of Random Effects covariates for the mixed Effects Regression Model
			\param Rgroup_sizes an R-vector containing the size of each group
			\param Rn_groups an R-integer storing the number of groups
		*/

		//Laplace
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch,
			SEXP Rmax_num_iteration, SEXP Rthreshold, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);

		// PDE
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, 
			SEXP Rmax_num_iteration, SEXP Rthreshold, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);

		// PDE SpaceVarying
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues,
			SEXP RincidenceMatrix, SEXP RarealDataAvg, SEXP Rsearch, 
			SEXP Rmax_num_iteration, SEXP Rthreshold, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);

		//Laplace time
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);
		
		// PDE time
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);
		
		// PDE SpaceVarying time
		explicit RegressionDataMixedEffects(SEXP Rlocations, SEXP RbaryLocations, SEXP Rtime_locations, SEXP Robservations, SEXP Rorder,
			SEXP RK, SEXP Rbeta, SEXP Rc, SEXP Ru, SEXP Rcovariates, SEXP RBCIndices, SEXP RBCValues, SEXP RincidenceMatrix, SEXP RarealDataAvg,
			SEXP Rflag_mass, SEXP Rflag_parabolic, SEXP Rflag_iterative, SEXP Rmax_num_iteration, SEXP Rthreshold, SEXP Ric, SEXP Rsearch, 
			SEXP Rmax_num_iteration_pirls, SEXP Rthreshold_pirls, 
			SEXP Rrandom_effects_covariates, SEXP Rgroup_ids, SEXP Rn_groups);

		//! A method returning the maximum iteration for the iterative method
		UInt get_maxiter() const {return max_num_iterations_;}
		//! A method returning the treshold
		Real get_treshold() const {return threshold_;}
		//! A method returning a const pointer to the design matrix of the mixed effects
		const MatrixXr * getRandomEfectsCovariates(void) const {return &random_effects_covariates_;}
		//! A method returning the number of covariates for the random effects
		const UInt get_q(void) const {return q_;}
		//! A method returning a const pointer to the vector storing the size of each group
		const std::vector<UInt> * getGroupSizes(void) const {return &group_sizes_;}
		//! A method returning a const pointer to the vector storing the ids permutation for FPIRLS
		const std::vector<std::vector<UInt>> * getIdsPerm(void) const {return &ids_perm_;}
		//! A method returning the total number of groups in the data
		const UInt getGroupNumber(void) const {return n_groups_;}

		//! Update weights
		void updateWeights(std::vector<MatrixXr> P);
};


// Type definitions for the MixedEffects data Structure
/** MixedEffectsDataLaplace type definition */
typedef RegressionDataMixedEffects<RegressionData> MixedEffectsDataLaplace;
/** MixedEffectsDataElliptic type definition */
typedef RegressionDataMixedEffects<RegressionDataElliptic> MixedEffectsDataElliptic;
/**  MixedEffectsDataEllipticSpaceVarying type definition */
typedef RegressionDataMixedEffects<RegressionDataEllipticSpaceVarying> MixedEffectsDataEllipticSpaceVarying;

#include "Regression_Data_imp.h"

#endif
