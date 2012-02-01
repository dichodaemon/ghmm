#include <unittest++/UnitTest++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <boost/graph/adjacency_list.hpp>
#include <ghmm/ITM.hpp>
#include <ghmm/itm_eigen_traits.hpp>
#include <iostream>


struct NodeData {
  Eigen::Matrix<float, 4, 1> centroid;
};

struct EdgeData {
};

typedef boost::adjacency_list< boost::hash_setS, boost::vecS, boost::bidirectionalS, NodeData, EdgeData > Graph;

SUITE( ITM ) {

  //----------------------------------------------------------------------------

  TEST( Defaults )
  {
    typedef ghmm::itm_eigen_traits< Graph, float, 4> Traits;

    Graph g;
    Traits::matrix_type sigma;
    sigma << 1.0, 0.0, 0.0, 0.0, 
             0.0, 1.0, 0.0, 0.0,
             0.0, 0.0, 4.0, 0.0,
             0.0, 0.0, 0.0, 4.0;
    ghmm::ITM< Traits > itm( 
      g, 
      Traits::distance_type( sigma ), 
      1, 1.5, 0.01 
    );

    for ( int i = 0; i < 1000; ++i ) {
      for ( int j = 0; j < 100; ++j ) {
        Traits::observation_type o;
        o << j / 10.0, i / 10.0, i / 100.0, j / 100.0;
        itm( o );
      }
    }
  }

}

