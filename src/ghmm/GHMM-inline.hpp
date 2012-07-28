template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
GHMM<T, N, FULL_N, GHMM_TRAITS>::GHMM(
  full_matrix_type fullSigma, 
  observation_matrix_type observationSigma, 
  goal_matrix_type goalSigma, 
  value_type insertionDistance, 
  value_type epsilon,
  value_type statePrior,
  value_type transitionPrior
) : observationSigma_( observationSigma ),
    goalSigma_( goalSigma ),
    statePrior_( statePrior ),
    transitionPrior_( transitionPrior ),
    graph_(),
    itm_( 
      graph_, 
      distance_type( fullSigma ), 
      insertionDistance, 
      epsilon 
    ), 
    observationGaussian_( observationSigma ),
    goalGaussian_( goalSigma ),
    fullGaussian_( fullSigma ),
    trajectoryCount_( 0 )
{}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
template< typename IT >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::learn( IT begin, IT end )
{
  trajectoryCount_++;
  for ( IT o = begin; o != end; ++o ) {
    itm_( *o );
  }
  normalize();

  computeForward( begin, end );
  computeBackwards( std::reverse_iterator<IT>( end ), std::reverse_iterator<IT>( begin ) );
  updateParameters( begin, end );
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::normalize()
{
  value_type priorSum = 0;

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    // Add self-edges
    boost::add_edge( *n, *n, graph_ );
    if ( graph_[*n].probability <= statePrior_ ) {
      graph_[*n].probabilitySum   = statePrior_;
    }
    priorSum += graph_[*n].probabilitySum;

    value_type transitionSum = 0;

    typename itm_type::out_edge_iterator child;
    typename itm_type::out_edge_iterator childEnd;

    for ( boost::tie( child, childEnd ) = boost::out_edges( *n, graph_ ); 
          child != childEnd; ++child
    ) {
      if ( graph_[*child].numeratorSum < transitionPrior_ ) {
        graph_[*child].numeratorSum = transitionPrior_;
        graph_[*child].denominatorSum = transitionPrior_;
      }
      graph_[*child].probability = graph_[*child].numeratorSum / graph_[*child].denominatorSum;
      transitionSum += graph_[*child].probability;
    }

    for ( boost::tie( child, childEnd ) = boost::out_edges( *n, graph_ );
          child != childEnd; ++child
    ) {
      graph_[*child].probability = graph_[*child].probability / transitionSum;
    }
  }

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].probability = graph_[*n].probabilitySum / priorSum;
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
template< typename IT >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::computeForward( IT begin, IT end )
{
  value_type total = 0;

  uint32_t size = std::distance( begin, end );

  factors_.resize( size );

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    value_type tmp = 
      graph_[*n].probability * observationProbability( *begin, *n );
    if ( ! tmp > 1E-40 ) {
      tmp = 1E-40;
    }
    graph_[*n].alpha.resize( size );
    graph_[*n].alpha[0] = tmp;
    total += tmp;
  }

  //assert( total > 0 );

  factors_[0] = total;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].alpha[0] /= total;
    //assert( graph_[*n].alpha[0] > 0 );
  }

  IT o = ++begin;
  for (uint32_t t = 1; o != end; ++o, ++t ) {
    total = 0;
    for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
          n != nodeEnd; ++n
    ) {
      typename GHMM_TRAITS::node_data_type & n1Info = graph_[*n]; 
      n1Info.alpha[t] = 0;

      typename itm_type::in_edge_iterator parentEdge;
      typename itm_type::in_edge_iterator parentEdgeEnd;

      for ( boost::tie( parentEdge, parentEdgeEnd ) = boost::in_edges( *n, graph_ ); 
            parentEdge != parentEdgeEnd; ++parentEdge
      ) {
        node_type parent = boost::source( *parentEdge, graph_ );
        value_type tmp =   graph_[parent].alpha[t - 1] 
                         * graph_[*parentEdge].probability 
                         * observationProbability( *o, *n );
        n1Info.alpha[t] += tmp;
      }
      if ( n1Info.alpha[t] < 1E-40 ) {
        n1Info.alpha[t] = 1E-40;
      }
      total += n1Info.alpha[t];
      assert( n1Info.alpha[t] > 0 );
    }
    assert( total > 0 );
    factors_[t] = total;

    for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
          n != nodeEnd; ++n
    ) {
      graph_[*n].alpha[t] /= total;
    }
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
template< typename IT >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::computeBackwards( IT begin, IT end ) 
{
  uint32_t t = std::distance( begin, end );

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].beta.resize( t + 1 );
    graph_[*n].beta[t] = 1;
  }

  t--;

  for ( IT o = begin; o != end; --t, ++o ) {
    for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
          n != nodeEnd; ++n
    ) {
      value_type & beta = graph_[*n].beta[t];
      beta = 0;

      typename itm_type::out_edge_iterator childEdge;
      typename itm_type::out_edge_iterator childEdgeEnd;

      for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
            childEdge != childEdgeEnd; ++childEdge
      ) {
        node_type n2 = boost::target( *childEdge, graph_ );
        beta +=   graph_[*childEdge].probability 
                * observationProbability( *o, n2 ) 
                * graph_[n2].beta[t + 1]
                / factors_[t];
      }
      if ( ! beta > 1E-40 ) {
        beta = 1E-40;
      }
    }
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
template< typename IT >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::updateParameters( IT begin, IT end )
{
  value_type totalPrior = 0;

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  typename itm_type::out_edge_iterator childEdge;
  typename itm_type::out_edge_iterator childEdgeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1Info = graph_[*n]; 
    value_type tmp = n1Info.alpha[0] * n1Info.beta[0] * factors_[0];
    if ( ! tmp > 1E-40 ) {
      tmp = 1E-40;
    }
    n1Info.probabilitySum += tmp;
    totalPrior += n1Info.probabilitySum;
  }
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1Info = graph_[*n]; 
    n1Info.probability /= totalPrior;

    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      node_type n2 = boost::target( *childEdge, graph_ );
      typename GHMM_TRAITS::edge_data_type & edgeInfo = graph_[*childEdge];
      typename GHMM_TRAITS::node_data_type & n2Info = graph_[*n]; 

      value_type numerator   = 0;
      value_type denominator = 0;
      uint32_t t = 1;
      for ( IT o = begin; o != end; ++o, ++t ) {
        numerator +=   n1Info.alpha[t - 1] 
                     * edgeInfo.probability
                     * observationProbability( *o, n2 ) 
                     * n2Info.beta[t];
        denominator += n1Info.alpha[t - 1] * n1Info.beta[t - 1] * factors_[t - 1];
      }
      edgeInfo.numeratorSum += numerator;
      edgeInfo.denominatorSum += denominator;
      std::cerr << edgeInfo.numeratorSum << ", " << edgeInfo.denominatorSum << std::endl;
    }
  }
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    // Compute new transition sum
    typename GHMM_TRAITS::node_data_type & n1Info = graph_[*n]; 
    value_type tmp = 0;
    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      graph_[*childEdge].probability = graph_[*childEdge].numeratorSum / graph_[*childEdge].denominatorSum;
      tmp += graph_[*childEdge].probability;
      if ( tmp == tmp ) {
      } else {
        std::cerr << graph_[*childEdge].numeratorSum << "/" << graph_[*childEdge].denominatorSum << std::endl;
      }
    }

    assert( tmp == tmp );
    assert( tmp > 0 );

    // Normalize and average with old probabilities
    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      typename GHMM_TRAITS::edge_data_type & edgeInfo = graph_[*childEdge];
      edgeInfo.probability = edgeInfo.probability / tmp;    
    }
    n1Info.probability = n1Info.probabilitySum / totalPrior;  
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::value_type
GHMM<T, N, FULL_N, GHMM_TRAITS>::observationProbability(
  const observation_type & o,
  const node_type & n
) const
{
  value_type result = observationGaussian_( o, GHMM_TRAITS::toObservation( graph_[n].centroid ) );
  return result;
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::value_type
GHMM<T, N, FULL_N, GHMM_TRAITS>::observationProbability(
  const full_observation_type & o,
  const node_type & n
) const
{
  value_type result = fullGaussian_( o, graph_[n].centroid );
  return result;
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::value_type
GHMM<T, N, FULL_N, GHMM_TRAITS>::goalProbability(
  const goal_type & g,
  const node_type & n
) const
{
  value_type result = observationGaussian_( g, GHMM_TRAITS::toGoal( graph_[n].centroid ) );
  return result;
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::update( 
  graph_type & graph, 
  const observation_type & o
) const
{
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  value_type total = 0;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1 = graph[*n];
    n1.estimations.resize( 1 );
    n1.estimations[0] = 0;

    typename itm_type::in_edge_iterator parentEdge;
    typename itm_type::in_edge_iterator parentEdgeEnd;

    for ( boost::tie( parentEdge, parentEdgeEnd ) = boost::in_edges( *n, graph ); 
          parentEdge != parentEdgeEnd; ++parentEdge
    ) {
      node_type parent = boost::source( *parentEdge, graph );
      assert( graph[parent].belief == graph[parent].belief );
      assert( graph[*parentEdge].probability == graph[*parentEdge].probability );
      std::cerr << "observation: " << o << ", *n:" << graph_[*n].centroid << std::endl;
      assert( observationProbability( o, *n ) == observationProbability( o, *n ) );
      value_type tmp =   graph[parent].belief 
                       * graph[*parentEdge].probability 
                       * observationProbability( o, *n );
      n1.estimations[0] += tmp;
    }
    if ( n1.estimations[0] < 1E-40 ) {
      n1.estimations[0] = 1E-40;
    }
    total += n1.estimations[0];
  }


  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    assert( total == total );
    assert( total > 0 );
    typename GHMM_TRAITS::node_data_type & n1 = graph[*n];
    n1.estimations[0] /= total;
    n1.belief = n1.estimations[0];
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::predict( graph_type & graph, uint8_t horizon ) const
{
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1 = graph[*n];
    n1.estimations.resize( horizon + 1 );
    n1.estimations[0] = n1.belief;
  }

  for ( uint32_t t = 1; t <= horizon; ++t ) {
    for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
          n != nodeEnd; ++n
    ) {
      typename GHMM_TRAITS::node_data_type & n1 = graph[*n];
      n1.estimations[t] = 0;

      typename itm_type::in_edge_iterator parentEdge;
      typename itm_type::in_edge_iterator parentEdgeEnd;

      for ( boost::tie( parentEdge, parentEdgeEnd ) = boost::in_edges( *n, graph ); 
            parentEdge != parentEdgeEnd; ++parentEdge
      ) {
        node_type parent = boost::source( *parentEdge, graph );
        n1.estimations[t] +=  graph[parent].estimations[t - 1]
                            * graph[*parentEdge].probability;
      }
    }
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
void
GHMM<T, N, FULL_N, GHMM_TRAITS>::initTrack( graph_type & graph ) const
{
  boost::copy_graph( graph_, graph );
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1 = graph[*n];
    n1.belief = n1.probability;
    n1.estimations.reserve( 1 );
    n1.estimations[0] = n1.probability;
  }
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::value_type 
  GHMM<T, N, FULL_N, GHMM_TRAITS>::observationPdf(
  const graph_type & graph, 
  uint32_t t, 
  const observation_type & o 
) const 
{
  value_type result = 1E-40;
  value_type beliefTotal = 0;
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    result += graph[*n].estimations[t] * observationProbability( o, *n ) + 1E-40;
    beliefTotal += graph[*n].estimations[t];
  }
  assert( result == result );
  assert( result  > 0 );
  return result;
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::value_type 
  GHMM<T, N, FULL_N, GHMM_TRAITS>::goalPdf(
  const graph_type & graph, 
  const goal_type & g 
) const 
{
  value_type result = 1E-40;
  value_type beliefTotal = 0;
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    result += graph[*n].estimations[0] * goalProbability( g, *n ) + 1E-40;
    beliefTotal += graph[*n].estimations[0];
  }
  assert( result == result );
  assert( result  > 0 );
  return result;
}

template < typename T, int N, int FULL_N,  typename GHMM_TRAITS >
typename GHMM<T, N, FULL_N, GHMM_TRAITS>::graph_type &
GHMM<T, N, FULL_N, GHMM_TRAITS>::graph()
{
  return graph_;
}

