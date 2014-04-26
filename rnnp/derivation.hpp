// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__DERIVATION__HPP__
#define __RNNP__DERIVATION__HPP__ 1

#include <rnnp/sentence.hpp>
#include <rnnp/span.hpp>
#include <rnnp/symbol.hpp>
#include <rnnp/parser.hpp>
#include <rnnp/tree.hpp>
#include <rnnp/debinarize.hpp>

namespace rnnp
{
  class Derivation
  {
  public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef uint32_t  index_type;

    typedef Tree tree_type;
    
    typedef Parser parser_type;
    
    typedef parser_type::sentence_type  sentence_type;
    typedef parser_type::word_type      word_type;
    typedef parser_type::symbol_type    symbol_type;
    
    typedef parser_type::operation_type operation_type;
    
    typedef parser_type::state_type   state_type;
    
  public:
    Derivation() {}
    Derivation(const state_type& state) { assign(state); }
    
  public:

    void assign(state_type state)
    {
      assign(state, binarized_);
      debinarize(binarized_, tree_);
    }

    void assign(state_type state, tree_type& tree)
    {
      switch (state.operation().operation()) {
      case operation_type::AXIOM:
	break;
      case operation_type::FINAL:
      case operation_type::IDLE:
	assign(state.derivation(), tree);
	break;
      case operation_type::UNARY:
	tree.label_ = state.label();
	tree.antecedent_.resize(1);
	assign(state.derivation(), tree.antecedent_.front());
	break;
      case operation_type::SHIFT:
	tree.label_ = state.label();
	tree.antecedent_ = tree_type::antecedent_type(1, state.head());
	break;
      case operation_type::REDUCE:
	tree.label_ = state.label();
	tree.antecedent_.resize(2);
	assign(state.reduced(), tree.antecedent_.front());
	assign(state.derivation(), tree.antecedent_.back());
	break;
      }
    }
    
    void clear()
    {
      binarized_.clear();
      tree_.clear();
    }
    
    
  public:    
    tree_type binarized_;
    tree_type tree_;
  };
};

#endif
