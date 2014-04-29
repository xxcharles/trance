// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__OBJECTIVE__MARGIN_EARLY__HPP__
#define __RNNP__OBJECTIVE__MARGIN_EARLY__HPP__ 1

#include <rnnp/objective/margin.hpp>

#include <rnnp/semiring/logprob.hpp>

namespace rnnp
{
  namespace objective
  {
    struct MarginEarly : public objective::Margin
    {
      typedef rnnp::semiring::Logprob<double> weight_type;
      
      double margin(const model_type& theta,
		    const parser_type& candidates,
		    const parser_oracle_type& oracles,
		    const option_type& option,
		    gradient_type& g)
      {
	if (candidates.agenda_goal_.size() != oracles.agenda_goal_.size())
	  throw std::runtime_error("invalid candidate and oracle pair");
	
	const size_type kbest_candidate_size = candidates.agenda_goal_.back().size();
	const size_type kbest_oracle_size    = oracles.agenda_goal_.back().size();
	
	if (! kbest_candidate_size || ! kbest_oracle_size)
	  return 0.0;
	
	weight_type Z_candidate;
	weight_type Z_oracle;
	
	for (size_type c = 0; c != kbest_candidate_size; ++ c)
	  Z_candidate += semiring::traits<weight_type>::exp(candidates.agenda_goal_.back()[c].score());
	
	for (size_type o = 0; o != kbest_oracle_size; ++ o)
	  Z_oracle += semiring::traits<weight_type>::exp(oracles.agenda_goal_.back()[o].score());
	
	bool found = false;
	double loss = 0.0;
	
	for (size_type c = 0; c != kbest_candidate_size; ++ c)
	  for (size_type o = 0; o != kbest_oracle_size; ++ o) {
	    state_type state_candidate = candidates.agenda_goal_.back()[c];
	    state_type state_oracle    = oracles.agenda_goal_.back()[o];
	      
	    double error_early = 0.0;
	    state_type state_candidate_early;
	    state_type state_oracle_early;
	      
	    while (state_candidate && state_oracle) {
	      if (state_candidate.step() > state_oracle.step())
		state_candidate = state_candidate.derivation();
	      else if (state_oracle.step() > state_candidate.step())
		state_oracle = state_oracle.derivation();
	      else {
		const double score_candidate = state_candidate.score();
		const double score_oracle    = state_oracle.score();
		  
		const bool suffered = score_candidate > score_oracle;
		const double error = std::max(1.0 - (score_oracle - score_candidate), 0.0);
		  
		if (suffered && error > 0.0) {
		  error_early = error;
		  state_candidate_early = state_candidate;
		  state_oracle_early    = state_oracle;
		}
		  
		state_candidate = state_candidate.derivation();
		state_oracle    = state_oracle.derivation();
	      }
	    }
	      
	    if (state_candidate_early && state_oracle_early) {
	      const double& score_candidate = candidates.agenda_goal_.back()[c].score();
	      const double& score_oracle    = oracles.agenda_goal_.back()[o].score();
		
	      const weight_type prob_candidate = rnnp::semiring::traits<weight_type>::exp(score_candidate) / Z_candidate;
	      const weight_type prob_oracle    = rnnp::semiring::traits<weight_type>::exp(score_oracle) / Z_oracle;

	      const double loss_factor = prob_candidate * prob_oracle;
	      
	      backward_type& backward_candidate = backward_[state_candidate_early];
	      backward_type& backward_oracle    = backward_[state_oracle_early];
	      
	      backward_candidate.loss_ += loss_factor;
	      backward_oracle.loss_    -= loss_factor;
	      
	      loss += error_early * loss_factor;
	      found = true;
	    }
	  }
	
	if (! found) return 0.0;

	for (size_type c = 0; c != kbest_candidate_size; ++ c)
	  states_[candidates.agenda_goal_.back()[c].step()].insert(candidates.agenda_goal_.back()[c]);
	
	for (size_type o = 0; o != kbest_oracle_size; ++ o)
	  states_[oracles.agenda_goal_.back()[o].step()].insert(oracles.agenda_goal_.back()[o]);
      
	return loss;
      }
    };
  };
};

#endif
