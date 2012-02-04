#ifndef GHMM_GAUSSIAN_HPP_
#define GHMM_GAUSSIAN_HPP_


namespace ghmm
{


template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
class Gaussian
{
public:
  typedef MATRIX_TYPE matrix_type;
  typedef VECTOR_TYPE vector_type;
  typedef T value_type;

  Gaussian( matrix_type sigma );
  value_type operator()( const vector_type & v1, const vector_type & v2 ) const;
  
private:
  matrix_type sigmaInverse_;
};


#include "Gaussian-inline.hpp"


}


#endif //GHMM_GAUSSIAN_HPP_

