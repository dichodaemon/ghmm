#ifndef GHMM_GHMM_DEFAULT_TRAITS_HPP_
#define GHMM_GHMM_DEFAULT_TRAITS_HPP_


#include <ghmm/ITM.hpp>
#include <ghmm/itm_eigen_traits.hpp>
#include <ghmm/Mahalanobis.hpp>
#include <ghmm/Gaussian.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <boost/graph/adjacency_list.hpp>


namespace ghmm
{


template < typename T, int N, int FULL_N>
class GHMMDefaultTraits
{
public:
  typedef T value_type;
  typedef typename Eigen::Matrix<value_type, 1, FULL_N> full_observation_type;
  typedef typename Eigen::Matrix<value_type, 1, N> observation_type;
  typedef typename Eigen::Matrix<value_type, 1, FULL_N - N> goal_type;
  typedef typename Eigen::Matrix<value_type, FULL_N, FULL_N> full_matrix_type;
  typedef typename Eigen::Matrix<value_type, N, N> observation_matrix_type;
  typedef typename Eigen::Matrix<value_type, FULL_N - N, FULL_N - N> goal_matrix_type;
  typedef typename std::vector< value_type > estimations_type;

  struct node_data_type {
    node_data_type() : prior( 0 ){};
    full_observation_type centroid;
    value_type oldPrior;
    value_type prior;
    estimations_type estimations;
    value_type belief;
    std::vector<value_type> alpha;
    std::vector<value_type> beta;
  };

  struct edge_data_type {
    edge_data_type() : value( 0 ){};
    value_type oldValue;
    value_type value;
  };

  typedef typename boost::adjacency_list< 
      boost::hash_setS, 
      boost::vecS, 
      boost::bidirectionalS, 
      node_data_type, 
      edge_data_type > graph_type;
  typedef typename ghmm::itm_eigen_traits< 
      graph_type, 
      value_type, 
      FULL_N > itm_traits;
  typedef typename ghmm::ITM< itm_traits > itm_type;
  typedef typename ghmm::Mahalanobis<
      value_type, 
      full_matrix_type, 
      full_observation_type > distance_type;
  typedef typename ghmm::Gaussian<
      value_type, 
      observation_matrix_type, 
      observation_type > observation_gaussian_type;
  typedef typename ghmm::Gaussian<
      value_type, 
      goal_matrix_type, 
      goal_type > goal_gaussian_type;
  static observation_type toObservation( const full_observation_type & o )
  {
    return o.block( 0, 0, 1, N );
  }

  static observation_type toGoal( const full_observation_type & o )
  {
    return o.block( 0, N, 1, FULL_N - N );
  }
};


}


#endif //GHMM_GHMM_DEFAULT_TRAITS_HPP_

