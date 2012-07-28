#ifndef GHMM_GHMM_H_
#define GHMM_GHMM_H_


#include "ghmm_default_traits.hpp"
#include <boost/graph/copy.hpp>


namespace ghmm
{


template < typename T, int N, int FULL_N,  typename GHMM_TRAITS = GHMMDefaultTraits< T, N, FULL_N > >
class GHMM
{
public:
  typedef GHMM_TRAITS traits_type;
  typedef typename GHMM_TRAITS::full_observation_type full_observation_type;
  typedef typename GHMM_TRAITS::observation_type observation_type;
  typedef typename GHMM_TRAITS::goal_type goal_type;
  typedef typename GHMM_TRAITS::value_type value_type;
  typedef typename GHMM_TRAITS::graph_type graph_type;
  typedef typename GHMM_TRAITS::distance_type distance_type;
  typedef typename GHMM_TRAITS::observation_gaussian_type observation_gaussian_type;
  typedef typename GHMM_TRAITS::goal_gaussian_type goal_gaussian_type;
  typedef typename GHMM_TRAITS::full_gaussian_type full_gaussian_type;
  typedef typename GHMM_TRAITS::full_matrix_type full_matrix_type;
  typedef typename GHMM_TRAITS::observation_matrix_type observation_matrix_type;
  typedef typename GHMM_TRAITS::goal_matrix_type goal_matrix_type;
  typedef typename GHMM_TRAITS::itm_type itm_type;
  typedef typename GHMM_TRAITS::itm_type::node_type node_type;
  typedef typename itm_type::node_iterator node_iterator;
  typedef typename itm_type::out_edge_iterator out_edge_iterator;
  typedef typename itm_type::in_edge_iterator in_edge_iterator;
  typedef typename GHMM_TRAITS::estimations_type estimations_type;

  GHMM( 
    full_matrix_type fullSigma, 
    observation_matrix_type observationSigma, 
    goal_matrix_type goalSigma,
    value_type insertionDistance, 
    value_type epsilon,
    value_type statePrior,
    value_type transitionPrior
  );

  void initTrack( graph_type & graph ) const;
  void update( graph_type & graph, const observation_type & o ) const;
  void predict( graph_type & graph, uint8_t horizon ) const;
  value_type observationPdf( 
    const graph_type & graph, 
    uint32_t t, 
    const observation_type & o 
  ) const;

  value_type goalPdf( 
    const graph_type & graph, 
    const goal_type & g 
  ) const;

  template < typename IT >
  void learn( IT begin, IT end );

  graph_type & graph();
private:
  typedef typename std::vector< value_type > value_array;

  observation_matrix_type   observationSigma_;
  goal_matrix_type          goalSigma_;
  full_matrix_type          fullSigma_;
  value_type                statePrior_;
  value_type                transitionPrior_;
  graph_type                graph_;
  itm_type                  itm_;
  observation_gaussian_type observationGaussian_;
  goal_gaussian_type        goalGaussian_;
  full_gaussian_type        fullGaussian_;
  uint32_t                  trajectoryCount_;

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
  ) const;

  value_type observationProbability( 
    const full_observation_type & o, 
    const node_type & n 
  ) const;

  value_type goalProbability( 
    const goal_type & g, 
    const node_type & n 
  ) const;
};


#include "GHMM-inline.hpp"


}


#endif //GHMM_GHMM_H_

