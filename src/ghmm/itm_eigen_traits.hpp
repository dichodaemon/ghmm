#ifndef GHMM_ITM_EIGEN_TRAITS_H_
#define GHMM_ITM_EIGEN_TRAITS_H_


#include <eigen3/Eigen/Core>
#include <boost/graph/graph_traits.hpp>


#include "Mahalanobis.hpp"


namespace ghmm
{


template < typename G, typename T, int N >
class itm_eigen_traits
{
public:
  typedef G graph_type;
  typedef T value_type;
  typedef Eigen::Matrix<T, 1, N> observation_type;
  typedef Eigen::Matrix<T, N, N> matrix_type;
  typedef  Mahalanobis<T, matrix_type, observation_type> distance_type;
  typedef typename boost::graph_traits<graph_type>::vertex_descriptor node_type;
  typedef typename boost::graph_traits<graph_type>::vertex_iterator node_iterator;
  typedef typename boost::graph_traits<graph_type>::edge_descriptor edge_type;
  typedef typename boost::graph_traits<graph_type>::out_edge_iterator out_edge_iterator;
  typedef typename boost::graph_traits<graph_type>::in_edge_iterator in_edge_iterator;
};


}


#endif //GHMM_ITM_EIGEN_TRAITS_H_

