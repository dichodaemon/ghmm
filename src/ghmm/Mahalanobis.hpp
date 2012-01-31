#ifndef GHMM_MAHALANOBIS_HPP_
#define GHMM_MAHALANOBIS_HPP_


namespace ghmm
{


template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
class Mahalanobis
{
public:
  typedef MATRIX_TYPE matrix_type;
  typedef VECTOR_TYPE vector_type;
  typedef T value_type;

  Mahalanobis( matrix_type sigma );
  value_type operator()( const vector_type & v1, const vector_type & v2 );
  
private:
  matrix_type sigmaInverse_;
};


#include "Mahalanobis-inline.hpp"


}


#endif //GHMM_MAHALANOBIS_HPP_

