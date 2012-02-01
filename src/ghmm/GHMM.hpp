#ifndef GHMM_GHMM_H_
#define GHMM_GHMM_H_


#include "ITM.hpp"


namespace ghmm
{


template < typename GHMM_TRAITS >
class GHMM
{
public:
  typedef typename GHMM_TRAITS::full_observation_type full_observation_type;
  typedef typename GHMM_TRAITS::observation_type observation_type;
  typedef typename GHMM_TRAITS::value_type value_type;
  typedef typename GHMM_TRAITS::graph_type graph_type;
  typedef typename GHMM_TRAITS::full_distance_type full_distance_type;
  typedef typename GHMM_TRAITS::distance_type distance_type;
  typedef typename GHMM_TRAITS::full_matrix_type full_matrix_type;
  typedef typename GHMM_TRAITS::matrix_type matrix_type;
  typedef typename GHMM_TRAITS::itm_type itm_type;
  typedef typename GHMM_TRAITS::itm_type::node_type node_type;
  //typedef typename GHMM_TRAITS::

  GHMM( 
    full_matrix_type fullSigma, 
    matrix_type sigma, 
    value_type insertionDistance, 
    value_type deletionDistance, 
    value_type epsilon,
    value_type statePrior,
    value_type transitionPrior
  );

  void update( graph_type & graph, const observation_type & observation ) const;
  void predict( graph_type & graph, uint8_t horizon ) const;
  void initTrack( graph_type & graph ) const;

  template < typename IT >
  void learn( IT begin, IT end );
private:
  std::vector< value_type > value_array;
  matrix_type sigma_;
  value_type  statePrior_;
  value_type  transitionPrior_;
  graph_type  graph_;
  itm_type    itm_;
  uint32_t    trajectoryCount_;

  value_array alpha_;
  value_array beta_;
  value_array factors_;

  void normalize();

  template < typename IT >
  void computeForward( IT begin, IT end );

  template < typename IT >
  void computeBackwards( IT begin, IT end );

  template < typename IT >
  void updateParameters( IT begin, IT end );

  value_type observationProbability( 
    const observation_type & o, 
    const node_type & n 
  );
};


#include "GHMM-inline.hpp"


}


#endif //GHMM_GHMM_H_

