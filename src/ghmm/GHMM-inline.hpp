template< typename GHMM_TRAITS >
GHMM<GHMM_TRAITS>::GHMM(
  full_matrix_type fullSigma, 
  matrix_type sigma, 
  value_type insertionDistance, 
  value_type epsilon,
  value_type statePrior,
  value_type transitionPrior
) : sigma_( sigma ),
    statePrior_( statePrior ),
    transitionPrior_( transitionPrior ),
    graph_(),
    itm_( 
      graph_, 
      distance_type( fullSigma ), 
      insertionDistance, 
      epsilon 
    ), 
    gaussian_( sigma ),
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

  std::vector<observation_type> observations;
  IT fullO;
  for ( fullO = begin; fullO != end; ++fullO ) {
    observation_type o;
    for ( uint32_t i = 0; i < o.size(); ++i ) {
      o[i] = (*fullO)[i];
    }
    observations.push_back( o );
  }

  computeForward( observations.begin(), observations.end() );
  computeBackwards( observations.rbegin(), observations.rend() );
  updateParameters( observations.begin(), observations.end() );
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
    // Add self-edges
    boost::add_edge( *n, *n, graph_ );
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
      assert( *n != boost::target( *child, graph_ ) );
      graph_[*child].value /= transitionSum;
      graph_[*child].oldValue = graph_[*child].value;
      //std::cerr << graph_[*n].centroid << "-"
                //<< graph_[boost::target( *child, graph_)].centroid << ": "
                //<< graph_[*child].value
                //<< std::endl;
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
  value_type total = 0;

  uint32_t size = std::distance( begin, end );

  factors_.reserve( size );

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    value_type tmp = 
      graph_[*n].prior * observationProbability( *begin, *n );
    //std::cerr << graph_[*n].prior << ", "
              //<< observationProbability( *begin, *n )
              //<< std::endl;
    graph_[*n].alpha.reserve( size );
    graph_[*n].alpha[0] = tmp;
    total += tmp;
  }

  assert( total > 0 );

  factors_[0] = total;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].alpha[0] /= total;
    assert( graph_[*n].alpha[0] > 0 );
  }

  IT o = ++begin;
  for (uint32_t t = 1; o != end; ++o, ++t ) {
    total = 0;
    for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
          n != nodeEnd; ++n
    ) {
      graph_[*n].alpha[t] = 0;

      typename itm_type::in_edge_iterator parentEdge;
      typename itm_type::in_edge_iterator parentEdgeEnd;

      for ( boost::tie( parentEdge, parentEdgeEnd ) = boost::in_edges( *n, graph_ ); 
            parentEdge != parentEdgeEnd; ++parentEdge
      ) {
        node_type parent = boost::source( *parentEdge, graph_ );
        value_type tmp =   graph_[parent].alpha[t - 1] 
                         * graph_[*parentEdge].value 
                         * observationProbability( *o, *n );
        //std::cerr << graph_[*n].centroid << "-"
                  //<< graph_[parent].centroid << ": "
                  //<< graph_[parent].alpha[t - 1] << ", " 
                  //<< graph_[*parentEdge].value << ", "
                  //<< observationProbability( *o, *n )
                  //<< std::endl;
        graph_[*n].alpha[t] += tmp;
        total += tmp;
      }
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

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::computeBackwards( IT begin, IT end ) 
{
  uint32_t t = std::distance( begin, end );

  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;

  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    graph_[*n].beta.reserve( t );
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
        beta +=   graph_[*childEdge].value 
                * observationProbability( *o, n2 ) 
                * graph_[n2].beta[t + 1]
                / factors_[t];
      }
      assert( beta > 0 );
    }
  }
}

template< typename GHMM_TRAITS >
template< typename IT >
void
GHMM<GHMM_TRAITS>::updateParameters( IT begin, IT end )
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
    value_type tmp = n1Info.alpha[0] * n1Info.beta[0];
    assert( tmp > 0 );
    n1Info.prior = tmp;
    totalPrior += tmp;
  }
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph_ );
        n != nodeEnd; ++n
  ) {
    typename GHMM_TRAITS::node_data_type & n1Info = graph_[*n]; 
    n1Info.prior /= totalPrior;

    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      node_type n2 = boost::target( *childEdge, graph_ );
      typename GHMM_TRAITS::edge_data_type & edgeInfo = graph_[*childEdge];

      value_type tmp = 0;
      uint32_t t = 1;
      for ( IT o = begin; o != end; ++o, ++t ) {
        tmp +=   n1Info.alpha[t - 1] 
               * edgeInfo.oldValue
               * observationProbability( *o, n2 );
        assert( tmp > 0 );
      }
      edgeInfo.value = tmp;
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
      tmp += graph_[*childEdge].value;
    }

    // Normalize and average with old probabilities
    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      typename GHMM_TRAITS::edge_data_type & edgeInfo = graph_[*childEdge];
      edgeInfo.value /= tmp;
      edgeInfo.value =   (   edgeInfo.oldValue * ( trajectoryCount_ - 1 ) 
                           + edgeInfo.value )
                       / trajectoryCount_;

    }
    // Renormalize TODO: this seems inefficient, is there a cleaner way?
    tmp = 0;
    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      tmp += graph_[*childEdge].value;
    }
    for ( boost::tie( childEdge, childEdgeEnd ) = boost::out_edges( *n, graph_ ); 
          childEdge != childEdgeEnd; ++childEdge
    ) {
      graph_[*childEdge].value /= tmp;
    }
      n1Info.prior =   (   n1Info.oldPrior * ( trajectoryCount_ - 1) 
                         + n1Info.prior ) 
                     / trajectoryCount_;
  }
}

template< typename GHMM_TRAITS >
typename GHMM<GHMM_TRAITS>::value_type
GHMM<GHMM_TRAITS>::observationProbability(
  const observation_type & o,
  const node_type & n
) {
  value_type result = gaussian_( o, GHMM_TRAITS::toObservation( graph_[n].centroid ) );
  return result;
  //return 1;
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
  boost::copy_graph( graph_, graph );
  typename itm_type::node_iterator n;
  typename itm_type::node_iterator nodeEnd;
  for ( boost::tie( n, nodeEnd ) = boost::vertices( graph );
        n != nodeEnd; ++n
  ) {
    graph[*n].belief = graph[*n].prior;
  }
}

template< typename GHMM_TRAITS >
typename GHMM< GHMM_TRAITS >::graph_type &
GHMM<GHMM_TRAITS>::graph()
{
  return graph_;
}

