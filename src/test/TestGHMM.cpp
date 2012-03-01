#include <iostream>
#include <ghmm/GHMM.hpp>
#include <unittest++/UnitTest++.h>

using namespace boost;


SUITE( GHMM ) {

  //----------------------------------------------------------------------------

  TEST( Defaults )
  {
    typedef ghmm::GHMM<float, 2, 4> GHMMType;
    GHMMType::observation_matrix_type observationSigma;
    observationSigma << 1.0, 0.0, 
           0.0, 1.0;
    GHMMType::goal_matrix_type goalSigma;
    goalSigma << 4.0, 0.0,
                 0.0, 4.0;
    GHMMType::full_matrix_type fullSigma;
    fullSigma << 1.0, 0.0, 0.0, 0.0,
                 0.0, 1.0, 0.0, 0.0,
                 0.0, 0.0, 4.0, 0.0,
                 0.0, 0.0, 0.0, 4.0;
    GHMMType ghmm( 
      fullSigma, 
      observationSigma,
      goalSigma,
      1, 0.01,
      0.001, 0.001
    );

    for ( int i = 0; i < 100; ++i ) {
      std::vector< GHMMType::full_observation_type> trajectory;
      for ( int j = 0; j < 100; ++j ) {
        GHMMType::full_observation_type o;
        o << j / 10.0, i / 10.0, i / 100.0, j / 100.0;
        trajectory.push_back( o );
      }
      ghmm.learn( trajectory.begin(), trajectory.end() );
    }
  }

  TEST( Track )
  {
    typedef ghmm::GHMM<float, 2, 4> GHMMType;
    GHMMType::observation_matrix_type observationSigma;
    observationSigma << 1.0, 0.0, 
                        0.0, 1.0;
    GHMMType::goal_matrix_type goalSigma;
    goalSigma << 4.0, 0.0,
                 0.0, 4.0;
    GHMMType::full_matrix_type fullSigma;
    fullSigma << 1.0, 0.0, 0.0, 0.0,
                 0.0, 1.0, 0.0, 0.0,
                 0.0, 0.0, 4.0, 0.0,
                 0.0, 0.0, 0.0, 4.0;
    GHMMType ghmm( 
      fullSigma, 
      observationSigma,
      goalSigma,
      1, 0.01,
      0.001, 0.001
    );

    for ( int i = 0; i < 1; ++i ) {
      std::vector< GHMMType::full_observation_type> trajectory;
      for ( int j = 0; j < 100; ++j ) {
        GHMMType::full_observation_type o;
        o << j / 10.0, i / 10.0, i / 100.0, j / 100.0;
        trajectory.push_back( o );
      }
      ghmm.learn( trajectory.begin(), trajectory.end() );
    }

    GHMMType::graph_type g2;
    ghmm.initTrack( g2 );

    for ( int i = 0; i < 100; ++i ) {
      GHMMType::observation_type o;
      GHMMType::goal_type goal;
      o << i / 10.0, 0.0;
      ghmm.update( g2, o );
      ghmm.predict( g2, 20 );
      ghmm.observationPdf( g2, 20, o );
      ghmm.goalPdf( g2, 20, goal );
    }
  }
}

