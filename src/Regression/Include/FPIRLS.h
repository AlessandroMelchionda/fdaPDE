#ifndef __FPIRLS_H__
#define __FPIRLS_H__

#include <cmath>
#include <math.h>
#include <array>

#include "Mixed_FE_Regression.h"
#include "../../FE_Assemblers_Solvers/Include/Evaluator.h"
#include "../../FdaPDE.h"


//! @brief An abstract class that implement the apply method for the FPIRLS algorithm.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_Base {

  protected:

   const MeshHandler<ORDER, mydim, ndim> & mesh_;
   const std::vector<Real> mesh_time_;
   InputHandler & inputData_; //!< It contains the data of the problem (RegressionDataGAM)
   OptimizationData & optimizationData_; //!< It contains the data of the optimization problem
   MixedFERegression<InputHandler>  regression_;
   
   const UInt lenS_; //! It contains the length of the space-optimization parameters vector
   const UInt lenT_; //! It contains the length of the time-optimization parameters vector

   // the value of the functional is saved deparated (parametric and non-parametric part)
   std::vector<std::vector<std::array<Real,2>>> current_J_values;
   std::vector<std::vector<std::array<Real,2>>> past_J_values; //!< Stores the value of the functional J at each iteration in order to apply the stopping criterion

   std::vector<std::vector<UInt>> n_iterations; //!< Current number of iteration of PIRLS

   VectorXr forcingTerm;
   bool isSpaceVarying = false; //!< True only in space varying case.

   MatrixXv _solution; //!< Stores the system solution.
   MatrixXr _dof; //!< A matrix of VectorXr storing the computed dofs.

   std::vector<std::vector<Real>> _GCV; //!< A vector storing GCV values.
   std::vector<std::vector<Real>> _J_minima;//!< Stores the minimum value of the functional.

   // Evaluation of the solution in the locations and beta estimates
   MatrixXv _beta_hat;//!< Betas estimated if the model has covariates.
   MatrixXv _fn_hat; //!< Function coefficients estimated.

   // Methods (mostly virtual) used to implement the apply() method in a general way.

   //! A method that stops PIRLS based on difference between functionals J_k J_k+1 or n_iterations > max_num_iterations .
   bool stopping_criterion(const UInt& lambdaS_index, const UInt& lambdaT_index);
   //! A virtual method to implement Step (1) of f-PIRLS. It computes the weights and everything else required to perform the weighted regression
   virtual void prepare_weighted_regression(const UInt& lambdaS_index, const UInt& lambdaT_index) = 0;
   //! A method to implement Step (2) of f-PIRLS. It solves the weighted regression problem and stores the solution data in the corresponding objects
   void solve_weighted_regression(const UInt& lambdaS_index, const UInt& lambdaT_index);
   //! A virtual method to implement Step (3) of f-PIRLS. It uses the result of the weighted regression to finalize one iteration of the f-PIRLS and update various parameters
   virtual void update_parameters(const UInt& lambdaS_index, const UInt& lambdaT_index) = 0;
   //! A method that computes and return the current value of the functional J. It is divided in parametric and non parametric part.
   std::array<Real,2> compute_J(const UInt& lambdaS_index, const UInt& lambdaT_index);
   //! A method that computes the parametric part of functional J. It is virtual since this part depends on the FPIRLS specialization.
   virtual Real compute_J_parametric(const UInt& lambdaS_index, const UInt& lambdaT_index) = 0;
   //! A method that computes the GCV value for a given lambda.
   virtual void compute_GCV(const UInt& lambdaS_index, const UInt& lambdaT_index);
   //! A method that computes additional quantities that are required only for specific cases. It is called at the end of the iterative procedure.
   virtual void additional_estimates() = 0;
   
   
  public:

    FPIRLS_Base(const MeshHandler<ORDER,mydim,ndim>& mesh, 
    			InputHandler& inputData, OptimizationData & optimizationData); // Constructor
    
    FPIRLS_Base(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    			InputHandler& inputData, OptimizationData & optimizationData); // Constructor

    //! A virutal destructor
   virtual ~FPIRLS_Base(){};

   //! Main method: perform PIRLS and instanciate the solution in _solution , _dof
   void apply(const ForcingTerm& u);

   //! An inline member that returns a VectorXr, returns the whole solution_.
   inline MatrixXv const & getSolution() const{return _solution;}
   //! A method returning the computed dofs of the model
   inline MatrixXr const & getDOF() const{return _dof;}
   //! A method returning the current value of J
   inline std::vector<std::vector<Real>> const & get_J() const{return _J_minima;}
   //! A inline member that returns a VectorXr, returns the final beta estimate.
   inline MatrixXv const & getBetaEst() const{return _beta_hat;}
   //! An inline member that returns a VectorXr, returns the final function estimates.
   inline MatrixXv const & getFunctionEst() const{return _fn_hat;}
   //! An inline member that returns a the computed (or not) GCV estimates. If GCV is not computed, -1 is returned
   inline std::vector<std::vector<Real>> const & getGCV() const{return _GCV;}
   //! An inline member that return the number of iterations for each lambda
   inline std::vector<std::vector<UInt>> const & getIterations() const{return n_iterations;}


   //! A method returning the computed barycenters of the locationss
   inline MatrixXr const & getBarycenters() const{return regression_.getBarycenters();}
   //! A method returning the element ids of the locations
   inline VectorXi const & getElementIds() const{return regression_.getElementIds();}
   inline UInt get_size_S()const{return this->lenS_;}
   inline UInt get_size_T()const{return this->lenT_;}


};


//------------- Template Specialization ----------------


//! @brief A class used for the FPIRLS_base template specialization: Laplace or Elliptic cases.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS: public FPIRLS_Base< InputHandler, ORDER,  mydim,  ndim>{

  public:

    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, InputHandler& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<InputHandler, ORDER, mydim, ndim>(mesh, inputData, optimizationData){};
      
    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    		InputHandler& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<InputHandler, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData){};  
     //! A virtual destructor
   virtual ~FPIRLS(){};

   virtual void apply();
};

//! @brief A class used for the FPIRLS_base template specialization: EllipticSpaceVarying case.
template <UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS< GAMDataEllipticSpaceVarying, ORDER,  mydim,  ndim>: public FPIRLS_Base< GAMDataEllipticSpaceVarying, ORDER,  mydim,  ndim>{

  public:

    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, GAMDataEllipticSpaceVarying& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<GAMDataEllipticSpaceVarying, ORDER, mydim, ndim>(mesh, inputData, optimizationData){};
    
    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real> & mesh_time, GAMDataEllipticSpaceVarying& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<GAMDataEllipticSpaceVarying, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData){};     
    
    virtual ~FPIRLS(){};

    virtual void apply();
};

//! @brief A class used for the FPIRLS_base template specialization: EllipticSpaceVarying case.
template <UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS< MixedEffectsDataEllipticSpaceVarying, ORDER,  mydim,  ndim>: public FPIRLS_Base< MixedEffectsDataEllipticSpaceVarying, ORDER,  mydim,  ndim>{

  public:

    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, MixedEffectsDataEllipticSpaceVarying& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<MixedEffectsDataEllipticSpaceVarying, ORDER, mydim, ndim>(mesh, inputData, optimizationData){};
    
    FPIRLS(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real> & mesh_time, MixedEffectsDataEllipticSpaceVarying& inputData, OptimizationData & optimizationData):
      FPIRLS_Base<MixedEffectsDataEllipticSpaceVarying, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData){};     
    
    virtual ~FPIRLS(){};

    virtual void apply();
};



//------------- Generalized Additive Models (GAM) Specification ----------------

//! @brief A class that specify the solver traits needed to deal with GAM problems
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_GAM : public FPIRLS <InputHandler, ORDER, mydim, ndim> {
	
  protected:

	std::vector<std::vector<VectorXr>> WeightsMatrix_; //!< It contains the estimated weights for each lambda_S, lambda_T in the grid evaluation
	
	std::vector<std::vector<VectorXr>> mu_; //!< Mean vector
	std::vector<std::vector<VectorXr>> pseudoObservations_; //! Pseudodata observations
	std::vector<std::vector<VectorXr>> G_; //!< diag(link_deriv(mu)) it is a vector since it would be more memory consuming to declere it as a matrix
	   
	
	bool scale_parameter_flag_; //!< True if the distribution has the scale parameter and if it is not a given input.
	Real _scale_param;
	std::vector<std::vector<Real>> _variance_estimates; //!< Stores the variance estimates for each lambda.

	// Methods to implement Step (1) of f-PIRLS:
	   
	//! A method that assembles G matrix.
	void compute_G(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that computes the pseudodata. It perform step (1) of f-PIRLS.
	void compute_pseudoObs(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that assembles the weights matrix ( a diagonal matrix, hence it is stored as vector).
	void compute_Weights(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! Step (1). A method that computes the weights and everything else required to perform the weighted regression
	void prepare_weighted_regression(const UInt& lambdaS_index, const UInt& lambdaT_index);
	      
	// Methods to implement Step (3) of f-PIRLS
	   
	//! A method that updates mu vector. It perform step (3) of F-PIRLS.
	void compute_mu(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! Step (3). A method that uses the result of the weighted regression to finalize one iteration of the f-PIRLS algorithm and update various parameters
	void update_parameters(const UInt& lambdaS_index, const UInt& lambdaT_index);
	      
	// Other methods

	//! A method that computes and return the parametric part of the functional J.
	Real compute_J_parametric(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that computes the estimates of the variance. It depends on the scale flags: only the Gamma and InvGaussian distributions have the scale parameter.
	void compute_variance_est();
	//! A method that computes additional quantities that are required only for specific cases. It is called at the end of the iterative procedure.
	void additional_estimates();
	   
	   
	// link and other functions. Definited as pure virtual methods, the implementaton depend on the choosen distributions

	//! A pure virtual method that represents the link function: g(.)
	virtual Real link(const Real& mu)const = 0;
	//! A pure virtual method that represents the derivative function: g'(.)
	virtual Real link_deriv(const Real& mu)const = 0;
	//! A pure virtual method that represents the inverse link function: g^-1(.)
	virtual Real inv_link(const Real& theta)const = 0;
	//! A pure virtual method that represents the variance function: V(mu)
	virtual Real var_function(const Real& mu)const = 0;
	//! A pure virtual method that represents the deviation function: used as norm in GCV
	virtual Real dev_function(const Real&mu, const Real& x)const = 0;


  public:

	FPIRLS_GAM(const MeshHandler<ORDER,mydim,ndim>& mesh, 
	  			InputHandler& inputData, OptimizationData & optimizationData, 
				VectorXr mu0, bool scale_parameter_flag, Real scale_param); // Constructor
	    
	FPIRLS_GAM(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
	   			InputHandler& inputData, OptimizationData & optimizationData, 
				VectorXr mu0, bool scale_parameter_flag, Real scale_param); // Constructor

	//! A method that computes the GCV value for a given lambda.
	void compute_GCV(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! An inline member that returns the variance estimates.
	inline std::vector<std::vector<Real>> const & getVarianceEst() const{return  _variance_estimates;}

	//! A virutal destructor
	virtual ~FPIRLS_GAM(){};
	
};


//------------- Family Distributions Spcecification (for GAM) ----------------

//! @brief A class that specify the Bernoulli distribution for the FPIRLS class.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_Bernoulli : public FPIRLS_GAM <InputHandler, ORDER, mydim, ndim> {

  protected:
    inline Real link(const Real& mu)const{ return log(mu/(1 - mu)); }

    inline Real inv_link(const Real& theta)const{ return 1/(1 + exp(-theta)); }

    inline Real link_deriv(const Real& mu)const{ return 1/(mu*(1-mu)); }

    inline Real var_function(const Real& mu)const{ return(mu*(1-mu)); }

    inline Real dev_function(const Real& mu, const Real& x)const{return (x == 0)? 2*log(1/(1-mu)) : 2*log(1/mu);}

  public:

    FPIRLS_Bernoulli(const MeshHandler<ORDER,mydim,ndim>& mesh, InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, inputData, optimizationData, mu0, false, 1){};
    
    FPIRLS_Bernoulli(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData, mu0, false, 1){};
};


//! @brief A class that specify the Poisson distribution for the FPIRLS class.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_Poisson : public FPIRLS_GAM <InputHandler, ORDER, mydim, ndim> {

    protected:

      inline Real link(const Real& mu)const{ return log(mu); }

      inline Real link_deriv(const Real& mu)const{ return 1/mu; }

      inline Real inv_link(const Real& theta)const{ return exp(theta); }

      inline Real var_function(const Real& mu)const{ return mu ;}

      inline Real dev_function(const Real&mu, const Real& x)const{ return (x>0) ? x*log(x/mu) - (x-mu): mu; }

    public:

    FPIRLS_Poisson(const MeshHandler<ORDER,mydim,ndim>& mesh, InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, inputData, optimizationData, mu0, false, 1){};
    
    FPIRLS_Poisson(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData, mu0, false, 1){};

};

//! @brief A class that specify the Exponential distribution for the FPIRLS class.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_Exponential : public FPIRLS_GAM <InputHandler, ORDER, mydim, ndim>
{

    protected:

      inline Real link(const Real& mu)const{ return -1/mu; }

      inline Real link_deriv(const Real& mu)const{ return 1/(mu*mu); }

      inline Real inv_link(const Real& theta)const{ return - 1/theta; }

      inline Real var_function(const Real& mu)const{ return mu*mu ;}

      inline Real dev_function(const Real&mu, const Real& x)const{ return 2*(((x-mu)/mu)-log(x/mu)); }

    public:

    FPIRLS_Exponential(const MeshHandler<ORDER,mydim,ndim>& mesh, InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, inputData, optimizationData, mu0, false, 1){};

    FPIRLS_Exponential(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData, mu0, false, 1){};

};

//------------- Scaled Distributions ----------

//! @brief A class that specify the Gamma distribution for the FPIRLS class.
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_Gamma : public FPIRLS_GAM <InputHandler, ORDER, mydim, ndim> {

    protected:

      inline Real link(const Real& mu)const{ return - 1/mu ; }

      inline Real link_deriv(const Real& mu)const{ return 1/(mu*mu); }

      inline Real inv_link(const Real& theta)const{ return - 1/theta; }

      inline Real var_function(const Real& mu)const{ return mu*mu ;}

      inline Real dev_function(const Real&mu, const Real& x)const{ return 2*(((x-mu)/mu)-log(x/mu)); }

    public:

    FPIRLS_Gamma(const MeshHandler<ORDER,mydim,ndim>& mesh, InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0, bool scale_parameter_flag, Real scale_param):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, inputData, optimizationData, mu0, scale_parameter_flag, scale_param){};
    
    FPIRLS_Gamma(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
    InputHandler& inputData, OptimizationData & optimizationData, VectorXr mu0, bool scale_parameter_flag, Real scale_param):
      FPIRLS_GAM<InputHandler, ORDER, mydim, ndim>(mesh, mesh_time, inputData, optimizationData, mu0, scale_parameter_flag, scale_param){};  

};



//------------- Mixed Effects Models Specification ----------------

//! @brief A class that specify the solver traits needed to deal with GAM problems
template <typename InputHandler, UInt ORDER, UInt mydim, UInt ndim>
class FPIRLS_MixedEffects : public FPIRLS <InputHandler, ORDER, mydim, ndim> {
	
  protected:

	std::vector<std::vector<std::vector<MatrixXr>>> WeightsMatrix_; //!< It contains the estimated weights for each {group, lambda_S, lambda_T} in the grid evaluation
	
	const UInt q_; //!< It contains the number of the Random Effects
	const UInt n_groups_;						//!< It contains the number of groups
	const std::vector<UInt> group_sizes_;				//!< It contains the number of observations for each group
	const std::vector<std::vector<UInt>> ids_perm_;		//!< It contains the permutation map to link FPIRLS matrices to the solver matrices
	
	std::vector< MatrixXr > Z_; //!< It contains the Rando Effects design matrix for each group
	std::vector<MatrixXr> ZTZ_; //!< It atores the result of of [Z_^t Z_] for each group
	std::vector<Eigen::LLT<MatrixXr>> ZtildeTZtilde_; //!< It contains the LLT decomposition of the matrix [Ztilde_i^t Ztilde_i] (computed directly) for each group
	std::vector<std::vector<std::vector<VectorXr>>> b_hat_; //!< It contains the prediction of b_i for each {group, lambda_S, lambda_T}
	Eigen::LLT<MatrixXr> LTL_; //!< It contains the LLT decomposition of the matrix [L^t L]
	MatrixXr A_; //!< It contains matrix A
	std::vector<std::vector<VectorXr>> D_; //!< It contains the current estimate of the precision matrix of the Random Effects for each {lambda_S, lambda_T}
	std::vector<std::vector<VectorXr>> Sigma_b_; //!< It contains the final estimate of the covariance struction of the Rando Effects for each {lambda_S, lambda_T}
	std::vector<std::vector<Real>> sigma_sq_hat_; //!< It contains the estimates of sigma square

	Real counter = 0;
	
	// Constructor utilities
	
	//! Two utilities functions to slice vector and matrices
	VectorXr vector_indexing(const VectorXr * big_vector, const std::vector<UInt> ids);
	MatrixXr matrix_indexing(const MatrixXr * big_matrix, const std::vector<UInt> ids);

	//! A method to compute ZTZ and the initial guess for D_
	void initialize_matrices();

	// Methods to implement Step (1) of f-PIRLS:
	
	//! A method used to compute Z ZtildeTZtildeinv_
	void compute_ZtildeTZtilde(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that assembles the weights matrix (a block diagonal matrix). It takes advantage of the Woodbury Identity
	void compute_Weights(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! Step (1). A method that computes the weights and everything else required to perform the weighted regression
	void prepare_weighted_regression(const UInt& lambdaS_index, const UInt& lambdaT_index); 
	      
	// Methods to implement Step (3) of f-PIRLS
	
	//! A method that performs the E step of the EM procedure. It initializes step (3) of F-PIRLS.
	void compute_bhat(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that estimates the value of sigma^2
	void compute_sigma_sq_hat(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that builds and decomposes matrix [L^t L]
	void build_LTL(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that computes matrix A
	void compute_A(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that performs the M step of the EM procedure. It finalizes step (3) of F-PIRLS.
	//void maximization_step(const UInt& lambdaS_index, const UInt& lambdaT_index) {return;};
	//! Step (3). A method that uses the result of the weighted regression to finalize one iteration of the f-PIRLS algorithm and update various parameters
	void update_parameters(const UInt& lambdaS_index, const UInt& lambdaT_index);
	      
	// Other methods

	//! A method that computes and return the parametric part of the functional J.
	Real compute_J_parametric(const UInt& lambdaS_index, const UInt& lambdaT_index);
	//! A method that computes the estimates of the variance-covariance structure of the random effects.
	void additional_estimates();



  public:

	FPIRLS_MixedEffects(const MeshHandler<ORDER,mydim,ndim>& mesh, 
	  			InputHandler& inputData, OptimizationData & optimizationData); // Constructor
	    
	FPIRLS_MixedEffects(const MeshHandler<ORDER,mydim,ndim>& mesh, const std::vector<Real>& mesh_time,
	   			InputHandler& inputData, OptimizationData & optimizationData); // Constructor

	//! A method that computes the GCV value for a given lambda.
	void compute_GCV(const UInt& lambdaS_index, const UInt& lambdaT_index);

	// Getters
	std::vector<std::vector<VectorXr>> const getSigma_b(void) const {return Sigma_b_;};
	std::vector<std::vector<std::vector<VectorXr>>> const get_b_hat(void) const {return b_hat_;};

	//! A destructor
	~FPIRLS_MixedEffects() = default;
	
};



#include "FPIRLS_imp.h"

#endif
