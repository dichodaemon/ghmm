template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
Mahalanobis<T, MATRIX_TYPE, VECTOR_TYPE>::Mahalanobis( matrix_type sigma )
  : sigmaInverse_( sigma.inverse() )
{}

template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
typename Mahalanobis<T, MATRIX_TYPE, VECTOR_TYPE>::value_type
Mahalanobis<T, MATRIX_TYPE, VECTOR_TYPE>::operator()( const vector_type & v1, const vector_type & v2 )
{
  vector_type diff = v1 - v2;
  return diff * sigmaInverse_ * diff.transpose();
}
