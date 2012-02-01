#include <unittest++/UnitTest++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <boost/graph/adjacency_list.hpp>
#include <ghmm/GHMM.hpp>
#include <ghmm/ITM.hpp>
#include <ghmm/itm_eigen_traits.hpp>
#include <iostream>

using namespace boost;

template < typename T, int N, int FULL_N>
class GHMMTraits
{
public:
  typedef T value_type;
  typedef typename Eigen::Matrix<value_type, 1, FULL_N> full_observation_type;
  typedef typename Eigen::Matrix<value_type, 1, N> observation_type;
  typedef typename Eigen::Matrix<value_type, FULL_N, FULL_N> full_matrix_type;
  typedef typename Eigen::Matrix<value_type, N, N> matrix_type;

  struct node_data_type {
    node_data_type() : prior( 0 ){};
    full_observation_type centroid;
    value_type oldPrior;
    value_type prior;
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
      full_observation_type > full_distance_type;
  typedef typename ghmm::Mahalanobis<
      value_type, 
      matrix_type, 
      observation_type > distance_type;
};

SUITE( GHMM ) {

  //----------------------------------------------------------------------------

  TEST( Defaults )
  {
    typedef GHMMTraits<float, 2, 4> Traits; 
    Traits::graph_type g;
    Traits::matrix_type sigma;
    sigma << 1.0, 0.0, 
           0.0, 1.0;
    Traits::full_matrix_type fullSigma;
    fullSigma << 1.0, 0.0, 0.0, 0.0,
                 0.0, 1.0, 0.0, 0.0,
                 0.0, 0.0, 4.0, 0.0,
                 0.0, 0.0, 0.0, 4.0;
    ghmm::GHMM< Traits > ghmm( 
      fullSigma, 
      sigma,
      1, 1.5, 0.01,
      0.001, 0.001
    );

    for ( int i = 0; i < 1000; ++i ) {
      std::vector< Traits::full_observation_type> trajectory;
      for ( int j = 0; j < 100; ++j ) {
        Traits::full_observation_type o;
        o << j / 10.0, i / 10.0, i / 100.0, j / 100.0;
        trajectory.push_back( o );
      }
      ghmm.learn( trajectory.begin(), trajectory.end() );
    }
  }
}

