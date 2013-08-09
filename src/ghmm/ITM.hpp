#ifndef GHMM_ITM_H_
#define GHMM_ITM_H_


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <algorithm>
#include <list>
#include <iostream>


namespace ghmm
{


template< typename ITM_TRAITS >
class ITM
{
public:
  typedef typename ITM_TRAITS::observation_type observation_type;
  typedef typename ITM_TRAITS::graph_type graph_type;
  typedef typename ITM_TRAITS::value_type value_type;
  typedef typename ITM_TRAITS::node_type node_type;
  typedef typename ITM_TRAITS::node_iterator node_iterator;
  typedef typename ITM_TRAITS::edge_type edge_type;
  typedef typename ITM_TRAITS::out_edge_iterator out_edge_iterator;
  typedef typename ITM_TRAITS::in_edge_iterator in_edge_iterator;
  typedef typename ITM_TRAITS::distance_type distance_type;

  ITM( 
    graph_type & graph, 
    distance_type distance, 
    value_type insertionDistance, 
    value_type epsilon 
  );
  void operator()( const observation_type & o );
private:
  graph_type &  graph_;
  distance_type distance_;
  value_type    insertionDistance_;
  value_type    epsilon_;
  node_type     lastInserted_;
  node_type     none_;

  std::pair<node_type, node_type> findBest( const observation_type & o  );
  void handleDeletions( node_type best, node_type second );
  void handleInsertions( const observation_type & o, node_type best, node_type second );
};

#include "ITM-inline.hpp"

}


#endif //GHMM_ITM_H_

