template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
Gaussian<T, MATRIX_TYPE, VECTOR_TYPE>::Gaussian( matrix_type sigma )
  : sigmaInverse_( sigma.inverse() )
{}

template < typename T, typename MATRIX_TYPE, typename VECTOR_TYPE >
typename Gaussian<T, MATRIX_TYPE, VECTOR_TYPE>::value_type
Gaussian<T, MATRIX_TYPE, VECTOR_TYPE>::operator()( const vector_type & v1, const vector_type & v2 ) const
{
  vector_type diff = v1 - v2;
  value_type result = diff * sigmaInverse_ * diff.transpose();
  return exp( - 0.5 * result );
}
