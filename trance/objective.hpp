// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __TRANCE__OBJECTIVE__HPP__
#define __TRANCE__OBJECTIVE__HPP__ 1

#include <vector>

#include <trance/derivation.hpp>
#include <trance/parser.hpp>
#include <trance/parser_oracle.hpp>
#include <trance/tree.hpp>
#include <trance/model.hpp>
#include <trance/gradient.hpp>
#include <trance/learn_option.hpp>
#include <trance/loss.hpp>

#include <utils/unordered_map.hpp>
#include <utils/compact_set.hpp>

namespace trance
{
  struct Objective
  {
  public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

    typedef Model    model_type;
    typedef Gradient gradient_type;
    
    typedef model_type::word_type      word_type;
    typedef model_type::parameter_type parameter_type;
    typedef model_type::tensor_type    tensor_type;
    typedef model_type::adapted_type   adapted_type;

    typedef LearnOption option_type;

    typedef Parser       parser_type;
    typedef ParserOracle parser_oracle_type;
    typedef Derivation   derivation_type;    

    typedef parser_type::sentence_type  sentence_type;
    typedef parser_type::operation_type operation_type;
    typedef parser_type::state_type     state_type;
    typedef parser_type::heap_type      heap_type;
    typedef parser_type::agenda_type    agenda_type;

    typedef parser_type::feature_vector_type feature_vector_type;

    typedef Loss loss_type;
    
    struct backward_type
    {
      double      loss_;
      tensor_type delta_;
      
      backward_type() : loss_(0.0), delta_() {}
    };
    
    typedef utils::unordered_map<state_type, backward_type,
				 boost::hash<state_type>, std::equal_to<state_type>,
				 std::allocator<std::pair<const state_type, backward_type> > >::type backward_set_type;
    
    typedef utils::compact_set<state_type, 
			       utils::unassigned<state_type>, utils::unassigned<state_type>,
			       boost::hash<state_type>, std::equal_to<state_type>,
			       std::allocator<state_type> > state_set_type;
    typedef std::vector<state_set_type, std::allocator<state_set_type> > state_map_type;
    
    void initialize(const parser_type& candidates,
		    const parser_oracle_type& oracles)
    {
      backward_.clear();
      
      states_.clear();
      states_.resize(std::max(candidates.agenda_.size(), oracles.agenda_.size()));
    }

    template <typename Theta>
    backward_type& backward_state(const Theta& theta, const state_type& state, const double& loss)
    {
      backward_type& back = backward_[state];
      
      back.loss_ += loss;
      
      if (! back.delta_.rows())
	back.delta_ = tensor_type::Zero(theta.hidden_, 1);
      
      return back;
    }

    template <typename Theta>
    backward_type& backward_state(const Theta& theta, const state_type& state)
    {
      backward_type& back = backward_[state];
      
      if (! back.delta_.rows())
	back.delta_ = tensor_type::Zero(theta.hidden_, 1);
      
      return back;
    }
    
    derivation_type   derivation_;
    backward_set_type backward_;
    state_map_type    states_;
    
    tensor_type queue_;
    tensor_type buffer_;
  };
  
};

#endif
