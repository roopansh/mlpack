/**
 * @file hybrid_error_stat.h
 *
 * @author Bill March (march@gatech.edu)
 *
 * Defines the stat classes for the different kinds of error
 */
 
#ifndef HYBRID_ERROR_STAT_H
#define HYBRID_ERROR_STAT_H

// I should make all of these inherit the basic stat stuff
// The classes should be able to overlap entirely except for the Epsilon_ 
// function and some private variables for the complicated functions 

class GenericErrorStat {

 protected:
 
  index_t query_count_;
  
  double epsilon_;
  
  virtual double Epsilon_(double upper_bound, double lower_bound) = 0;
  
public:
    
  GenericErrorStat() {}
    
  virtual ~GenericErrorStat() {}
    
  void Init(const Matrix& matrix, index_t start, index_t count) {
      
    query_count_ = count;
      
  } // Init() (leaves)
  
  void Init(const Matrix& matrix, index_t start, index_t count, 
            const GenericErrorStat& left, const GenericErrorStat& right) {
    
    query_count_ = count;
    
  } // Init() (non-leaves)
  
  bool CanPrune(double q_upper_bound, double q_lower_bound, 
                index_t reference_count) {
    
    bool prune = false;
    
    double max_error_incurred = 0.5 * (q_upper_bound - q_lower_bound);
    DEBUG_ASSERT(max_error_incurred >= 0.0);
    
    double allowed_error = q_lower_bound * 
      Epsilon_(q_upper_bound, q_lower_bound) * reference_count / query_count_;
    DEBUG_ASSERT(allowed_error >= 0.0);
    
    if (max_error_incurred < allowed_error) {
      
      prune = true;
      
      epsilon_ = epsilon_ - max_error_incurred;
      
      query_count_ = query_count_ - reference_count;
      DEBUG_ASSERT(query_count_ >= 0.0);
      
    }
    
    return prune;
    
  } // CanPrune()  
  
  void set_query_count(index_t new_count) {
  
    query_count_ = new_count;
    
    DEBUG_ASSERT(query_count_ >= 0);
  
  } // set_query_count()
  
  index_t query_count() {
  
    return query_count_;
  
  } // query_count()

}; // GenericErrorStat

/**
 * Prunes with an absolute error criterion
 */
class AbsoluteErrorStat : public GenericErrorStat {



 protected:

  /**
   * Returns the error tolerance as a function of the bounds on Q.  In this case
   * we divide by the lower bound to get absolute error.
   */ 
  double Epsilon_(double upper_bound, double lower_bound) {
  
    double eps = epsilon_ / lower_bound;
    DEBUG_ASSERT(eps >= 0.0);
    
    return (eps);
  
  } // Epsilon_
  

 public:
 
  AbsoluteErrorStat() {}
    
  ~AbsoluteErrorStat() {}
  
  void SetParams(struct datanode* mod) {
  
    epsilon_ = fx_param_double_req(mod, "epsilon");
  
  } // SetParams()

}; // class AbsoluteErrorStat


/** 
 * Prunes with a relative error criterion
 */
class RelativeErrorStat : public GenericErrorStat {

 private:

  index_t query_count_;

protected:
  /**
   * Relative error just depends on epsilon_
   */
  double Epsilon_(double upper_bound, double lower_bound) {
    
    DEBUG_ASSERT(epsilon_ >= 0.0);
    return epsilon_;
    
  } // Epsilon_

 public:
 
  RelativeErrorStat() {}
    
  ~RelativeErrorStat() {}
    
  void SetParams(struct datanode* mod) {
  
    epsilon_ = fx_param_double_req(mod, "epsilon");
    DEBUG_ASSERT(epsilon_ >= 0.0);
  
  
  } // SetParams  
  
  
}; // class RelativeErrorStat


/**
 * Prunes with the hybrid exponential error criterion 
 */
class ExponentialErrorStat : public GenericErrorStat {

 private:
  
  double max_error_;
  
  double steepness_;
  
  double min_error_;
  
  
protected:
  /**
   * Hybrid error using the exponential criterion
   */
  double Epsilon_(double upper_bound, double lower_bound) {
    
    double eps = (max_error_ * exp(-steepness_ * upper_bound)) + min_error_ + 
        epsilon_;
    
    DEBUG_ASSERT(eps >= 0.0);
    
    return (eps);
    
  } // Epsilon_()
  
 public:
 
  ExponentialErrorStat() {}
    
  ~ExponentialErrorStat() {}

    void SetParams(struct datanode* mod) {
    
      max_error_ = fx_param_double_req(mod, "max_error");
    
      steepness_ = fx_param_double_req(mod, "steepness");
      
      min_error_ = fx_param_double_req(mod, "min_error");
      
      epsilon_ = 0.0;
         
    } // SetParams()
  
  
}; // class ExponentialErrorStat

/**
 * Uses a Gaussian hybrid error criterion
 */
class GaussianErrorStat : public GenericErrorStat {

private:
  
  double max_error_;
  
  double steepness_;
  
  double min_error_;
  
  
protected:
  /**
    * Hybrid error using the gaussian criterion
   */
  double Epsilon_(double upper_bound, double lower_bound) {
    
    double eps = (max_error_ * exp(-steepness_ * upper_bound * upper_bound)) 
    + min_error_ + epsilon_;
    
    DEBUG_ASSERT(eps >= 0.0);

    return (eps);
    
  } // Epsilon_()
  
 public:

  GaussianErrorStat() {}
    
  ~GaussianErrorStat() {}
    
    void SetParams(struct datanode* mod) {
    
      max_error_ = fx_param_double_req(mod, "max_error");
      
      steepness_ = fx_param_double_req(mod, "steepness");
      
      min_error_ = fx_param_double_req(mod, "min_error");     
      
      epsilon_ = 0.0;
    
    } // SetParams  

}; // class GaussianErrorStat

class HybridErrorStat : public GenericErrorStat {

private:

  double steepness_;
  
  
protected:

  double Epsilon_(double upper_bound, double lower_bound) {
  
    double eps = (1 - exp(-steepness_ * lower_bound)) * epsilon_;
    
    eps = eps + (exp(-steepness_ * upper_bound) * epsilon_ / lower_bound);
    
    DEBUG_ASSERT(eps >= 0.0);
    
    return eps;
  
  } // Epsilon_()

public:

    HybridErrorStat() {}
    
  ~HybridErrorStat() {}
  
  void SetParams(struct datanode* mod) {
  
    steepness_ = fx_param_double_req(mod, "steepness");
    
    epsilon_ = fx_param_double_req(mod, "epsilon");
  
  } // SetParams()

};  // class HybridErrorStat



#endif