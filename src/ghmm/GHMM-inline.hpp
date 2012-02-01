template< typename GHMM_TRAITS >
GHMM<GHMM_TRAITS>::GHMM(
  full_matrix_type fullSigma, 
  matrix_type sigma, 
  value_type insertionDistance, 
  value_type deletionDistance, 
  value_type epsilon,
  value_type statePrior,
  value_type transitionPrior
) : sigma_( sigma ),
    statePrior_( statePrior ),
    transitionPrior_( transitionPrior ),
    graph_(),
    itm_( 
      graph_, 
      full_distance_type( fullSigma ), 
      insertionDistance, 
      deletionDistance, 
      epsilon 
    ), 
    trajectoryCount_( 0 )
{}

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::learn( IT begin, IT end )
{
  trajectoryCount_++;
  for ( IT o = begin; o != end; ++o ) {
    itm_( *o );
  }
  normalize();
  computeForward( begin, end );
  computeBackwards( begin, end );
  updateParameters( begin, end );
}

template< typename GHMM_TRAITS >
void
GHMM<GHMM_TRAITS>::normalize()
{
  value_type priorSum = 0;

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    if ( graph_[*n].prior < statePrior_ ) {
      graph_[*n].prior = statePrior_;
    }
    priorSum += graph_[*n].prior;

    value_type transitionSum = 0;

    typename itm_type::out_edge_iterator child;
    typename itm_type::out_edge_iterator childEnd;

    for ( boost::tie( child, childEnd ) = boost::out_edges( *n, graph_ ); 
          child != childEnd; ++child
    ) {
      if ( graph_[*child].value < transitionPrior_ ) {
        graph_[*child].value = transitionPrior_;
      }
      transitionSum += graph_[*child].value;
    }

    for ( boost::tie( child, childEnd ) = boost::out_edges( *n, graph_ );
          child != childEnd; ++child
    ) {
      if ( graph_[*child].value < transitionPrior_ ) {
        graph_[*child].value /= transitionSum;
        graph_[*child].oldValue = graph_[*child].value;
      }
    }
  }

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].prior /= priorSum;
    graph_[*n].oldPrior = graph_[*n].prior;
  }
}

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::computeForward( IT begin, IT end )
{
}

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::computeBackwards( IT begin, IT end ) 
{
}

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::updateParameters( IT begin, IT end )
{
}

template< typename GHMM_TRAITS >
typename GHMM<GHMM_TRAITS>::value_type
GHMM<GHMM_TRAITS>::observationProbability(
  const observation_type & o,
  const node_type & n
) {
  return 0;
}

template< typename GHMM_TRAITS >
void
GHMM<GHMM_TRAITS>::update( 
  graph_type & graph, 
  const observation_type & observation 
) const
{
}

template< typename GHMM_TRAITS >
void
GHMM<GHMM_TRAITS>::predict( graph_type & graph, uint8_t horizon ) const
{
}

template< typename GHMM_TRAITS >
void
GHMM<GHMM_TRAITS>::initTrack( graph_type & graph ) const
{
}

