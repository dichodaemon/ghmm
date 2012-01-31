template< typename ITM_TRAITS >
ITM<ITM_TRAITS>::ITM( 
  graph_type & graph, 
  distance_type distance, 
  value_type insertionDistance, 
  value_type deletionDistance,
  value_type epsilon 
) : graph_( graph ),
    distance_( distance ),
    insertionDistance_( insertionDistance ),
    deletionDistance_( deletionDistance ),
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
    second = best;
    best = boost::add_vertex( graph_ );
    graph_[best].centroid = o;
  }

  observation_type bestCentroid = graph_[best].centroid;
  observation_type secondCentroid = graph_[second].centroid;
  if (   distance_( bestCentroid, secondCentroid ) 
       < insertionDistance_ ) 
  {
    boost::add_edge( best, second, graph_ );
    boost::add_edge( second, best, graph_ );
  }

  bestCentroid += epsilon_ * ( o - bestCentroid );
  graph_[best].centroid = bestCentroid;

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
  observation_type bestCentroid = graph_[best].centroid;
  observation_type secondCentroid = graph_[second].centroid;

  // Don't know of other way to avoid iterator invalidation
  out_edge_iterator iChild;
  out_edge_iterator eChild;
  typename std::list<node_type> erase;

  for ( boost::tie( iChild, eChild ) = boost::out_edges( best, graph_ );
        iChild != eChild; ++iChild
  ) {
    node_type child = boost::target( *iChild, graph_ );
    observation_type centroid = graph_[child].centroid;
    observation_type center = ( bestCentroid + centroid ) * 0.5;
    if (      distance_( center, secondCentroid ) 
            < distance_( center, centroid )
         && distance_( bestCentroid, centroid ) > deletionDistance_
    ) {
      erase.push_back( child );
    }
  }

  typename std::list<node_type>::iterator iErase;
  typename std::list<node_type>::iterator eErase = erase.end();
  for ( iErase = erase.begin(); iErase != eErase; ++iErase ) {
    boost::remove_edge( best, *iErase, graph_ );
    boost::remove_edge( *iErase, best, graph_ );
    if ( boost::out_edges( *iErase, graph_ ).first == boost::out_edges( *iErase, graph_ ).second ) {
      boost::remove_vertex( *iErase, graph_ );
    }
  }
}

template< typename ITM_TRAITS >
void
ITM<ITM_TRAITS>::handleInsertions( const observation_type & o, node_type best, node_type second )
{  
  observation_type bestCentroid = graph_[best].centroid;
  observation_type secondCentroid = graph_[second].centroid;
  observation_type center = ( bestCentroid + secondCentroid ) * 0.5;

  if (    (    distance_( center, secondCentroid ) < distance_( center, o ) 
            || distance_( secondCentroid, o ) > insertionDistance_ )
       && distance_( bestCentroid, o ) > insertionDistance_ 
  ) {
    node_type r = boost::add_vertex( graph_ );
    graph_[r].centroid = o;
    if ( distance_( bestCentroid, o ) < insertionDistance_ ) {
      boost::add_edge( best, r, graph_ );
      boost::add_edge( r, best, graph_ );
    }
    if (    lastInserted_ != none_ 
         && r != lastInserted_
         && distance_( o, graph_[lastInserted_].centroid ) < insertionDistance_ 
    ) {
      boost::add_edge( r, lastInserted_, graph_ );
      boost::add_edge( lastInserted_, r, graph_);
    }
    lastInserted_ = r;
  }
}
