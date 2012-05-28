#include <iostream>
#include <ghmm/GHMM.hpp>

using namespace boost;

int main()
{
  typedef ghmm::GHMM<float, 1, 2> GHMMType;
  GHMMType::observation_matrix_type observationSigma;
  observationSigma << 1.0; 
  GHMMType::goal_matrix_type goalSigma;
  goalSigma << 1.0;
  GHMMType::full_matrix_type fullSigma;
  fullSigma << .0025, 0.0,
               0.0, 1.0;
  GHMMType ghmm( 
    fullSigma, 
    observationSigma,
    goalSigma,
    1, 0.01,
    0.001, 0.001
  );

  for ( int i = 0; i < 1; ++i ) {
    std::vector< GHMMType::full_observation_type, Eigen::aligned_allocator<GHMMType::full_observation_type> > trajectory;
    for ( int j = 0; j < 30; ++j ) {
      std::cout << "<<" << j << ">>" << std::endl;
      GHMMType::full_observation_type o;
      o << j * 0.1, 3;
      trajectory.push_back( o );
    }
    std::cout << "-------------------------------------\n";
    ghmm.learn( trajectory.begin(), trajectory.end() );
  }
}
