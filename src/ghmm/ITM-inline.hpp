template< typename ITM_TRAITS >
ITM<ITM_TRAITS>::ITM( 
  graph_type & graph, 
  distance_type distance, 
  value_type insertionDistance, 
  value_type epsilon 
) : graph_( graph ),
    distance_( distance ),
    insertionDistance_( insertionDistance ),
    epsilon_( epsilon ),
    lastInserted_( boost::graph_traits<graph_type>::null_vertex() ),
    none_( boost::graph_traits<graph_type>::null_vertex() )
{
}

template< typename ITM_TRAITS >
void
ITM<ITM_TRAITS>::operator()( const observation_type & o ) 
{
  node_type best;
  node_type second;

  boost::tie( best, second ) = findBest( o );

  if ( best == none_ ) {
    node_type n = boost::add_vertex( graph_ );
    graph_[n].centroid = o;
    return;
  }

  if ( second == none_ ) {
    if ( distance_( graph_[best].centroid, o ) > insertionDistance_ ) {
      second = best;
      best = boost::add_vertex( graph_ );
      graph_[best].centroid = o;
    } else {
      return;
    }
  }

  assert( best != second );
  // This assumes that graph do not handle parallel edges
  boost::add_edge( best, second, graph_ );
  boost::add_edge( second, best, graph_ );

  observation_type & bestCentroid = graph_[best].centroid;
  bestCentroid += epsilon_ * ( o - bestCentroid );

  handleDeletions( best, second );
  handleInsertions( o, best, second );
}

template< typename ITM_TRAITS >
std::pair<typename ITM<ITM_TRAITS>::node_type, typename ITM<ITM_TRAITS>::node_type> 
ITM<ITM_TRAITS>::findBest( const observation_type & o )
{  
  node_type none_ = boost::graph_traits<graph_type>::null_vertex();
  node_iterator begin = boost::vertices( graph_ ).first;
  node_iterator end = boost::vertices( graph_ ).second;
  node_iterator best = end;
  node_iterator second = end;
  value_type bestDistance = std::numeric_limits<value_type>::max();
  value_type secondDistance = std::numeric_limits<value_type>::max();
  for ( node_iterator i = begin; i != end; ++i ) {
    value_type distance = distance_( o, graph_[*i].centroid );
    if ( distance < bestDistance ) {
      second = best;
      secondDistance = bestDistance;
      best = i;
      bestDistance = distance;
    } else if ( distance < secondDistance ) {
      second = i;
      secondDistance = distance;
    }
  }
  return std::make_pair( 
    best == end ? none_ : *best, 
    second == end ? none_ : *second 
  );
}

template< typename ITM_TRAITS >
void
ITM<ITM_TRAITS>::handleDeletions( node_type best, node_type second )
{  
  observation_type & bestCentroid = graph_[best].centroid;
  observation_type & secondCentroid = graph_[second].centroid;

  // Don't know of other way to avoid iterator invalidation than to do this
  // twice.
  out_edge_iterator iChild;
  out_edge_iterator eChild;
  typename std::list<node_type> erase;

  for ( boost::tie( iChild, eChild ) = boost::out_edges( best, graph_ );
        iChild != eChild; ++iChild
  ) {
    node_type child = boost::target( *iChild, graph_ );
    if ( child != best ) { // Do not try to delete self-edges
      observation_type centroid = graph_[child].centroid;
      observation_type center = ( bestCentroid + centroid ) * 0.5;
      if (      distance_( center, secondCentroid ) 
              < distance_( center, centroid )
      ) {
        erase.push_back( child );
      }
    }
  }

  typename std::list<node_type>::iterator iErase;
  typename std::list<node_type>::iterator eErase = erase.end();
  for ( iErase = erase.begin(); iErase != eErase; ++iErase ) {
    boost::remove_edge( best, *iErase, graph_ );
    boost::remove_edge( *iErase, best, graph_ );

    boost::tie( iChild, eChild ) = boost::out_edges( *iErase, graph_ );

    // The or condition is to handle self-edges
    if (   iChild == eChild  
        || (    std::distance( iChild, eChild ) == 1 
             && boost::target( *iChild, graph_ ) == *iErase )
    ) {
      boost::clear_vertex( *iErase, graph_ );
      boost::remove_vertex( *iErase, graph_ );
    }
  }
}

template< typename ITM_TRAITS >
void
ITM<ITM_TRAITS>::handleInsertions( const observation_type & o, node_type best, node_type second )
{  
  observation_type & bestCentroid = graph_[best].centroid;
  observation_type & secondCentroid = graph_[second].centroid;
  observation_type center = ( bestCentroid + secondCentroid ) * 0.5;

  if (    (    distance_( center, secondCentroid ) < distance_( center, o ) 
            || distance_( secondCentroid, o ) > 3 * insertionDistance_  )
       && distance_( bestCentroid, o ) > insertionDistance_ 
  ) {
    node_type r = boost::add_vertex( graph_ );
    graph_[r].centroid = o;
    assert( best != r );
    boost::add_edge( best, r, graph_ );
    boost::add_edge( r, best, graph_ );
    if (  lastInserted_ != none_ 
         && r != lastInserted_
    ) {
      boost::add_edge( r, lastInserted_, graph_ );
      boost::add_edge( lastInserted_, r, graph_);
      lastInserted_ = r;
    }
  } else if (  lastInserted_ != none_ 
       && best != lastInserted_
       //&& distance_( o, graph_[lastInserted_].centroid ) < insertionDistance_  * 3
  ) {
    boost::add_edge( best, lastInserted_, graph_ );
    boost::add_edge( lastInserted_, best, graph_);
    lastInserted_ = best;
  }
  if ( distance_( bestCentroid, secondCentroid ) < 0.5 * insertionDistance_ ) {
    boost::clear_vertex( second, graph_ );
    boost::remove_vertex( second, graph_ );
  }
}
