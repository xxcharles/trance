// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __TRANCE__MODEL_TRAITS__HPP__
#define __TRANCE__MODEL_TRAITS__HPP__ 1

#include <trance/gradient/model1.hpp>
#include <trance/gradient/model2.hpp>
#include <trance/gradient/model3.hpp>
#include <trance/gradient/model4.hpp>
#include <trance/gradient/model5.hpp>

#include <trance/model/model1.hpp>
#include <trance/model/model2.hpp>
#include <trance/model/model3.hpp>
#include <trance/model/model4.hpp>
#include <trance/model/model5.hpp>

#include <trance/parser/model1.hpp>
#include <trance/parser/model2.hpp>
#include <trance/parser/model3.hpp>
#include <trance/parser/model4.hpp>
#include <trance/parser/model5.hpp>

namespace trance
{
  template <typename M>
  struct model_traits { };
    
  template <>
  struct model_traits<model::Model1>
  {
    typedef model::Model1    model_type;
    typedef gradient::Model1 gradient_type;
    typedef parser::Model1   parser_type;
  };
    
  template <>
  struct model_traits<model::Model2>
  {
    typedef model::Model2    model_type;
    typedef gradient::Model2 gradient_type;
    typedef parser::Model2   parser_type;
  };
    
  template <>
  struct model_traits<model::Model3>
  {
    typedef model::Model3    model_type;
    typedef gradient::Model3 gradient_type;
    typedef parser::Model3   parser_type;
  };

  template <>
  struct model_traits<model::Model4>
  {
    typedef model::Model4    model_type;
    typedef gradient::Model4 gradient_type;
    typedef parser::Model4   parser_type;
  };

  template <>
  struct model_traits<model::Model5>
  {
    typedef model::Model5    model_type;
    typedef gradient::Model5 gradient_type;
    typedef parser::Model5   parser_type;
  };

  template <>
  struct model_traits<gradient::Model1>
  {
    typedef model::Model1    model_type;
    typedef gradient::Model1 gradient_type;
    typedef parser::Model1   parser_type;
  };
    
  template <>
  struct model_traits<gradient::Model2>
  {
    typedef model::Model2    model_type;
    typedef gradient::Model2 gradient_type;
    typedef parser::Model2   parser_type;
  };
    
  template <>
  struct model_traits<gradient::Model3>
  {
    typedef model::Model3    model_type;
    typedef gradient::Model3 gradient_type;
    typedef parser::Model3   parser_type;
  };

  template <>
  struct model_traits<gradient::Model4>
  {
    typedef model::Model4    model_type;
    typedef gradient::Model4 gradient_type;
    typedef parser::Model4   parser_type;
  };

  template <>
  struct model_traits<gradient::Model5>
  {
    typedef model::Model5    model_type;
    typedef gradient::Model5 gradient_type;
    typedef parser::Model5   parser_type;
  };
  
  template <>
  struct model_traits<parser::Model1>
  {
    typedef model::Model1    model_type;
    typedef gradient::Model1 gradient_type;
    typedef parser::Model1   parser_type;
  };
    
  template <>
  struct model_traits<parser::Model2>
  {
    typedef model::Model2    model_type;
    typedef gradient::Model2 gradient_type;
    typedef parser::Model2   parser_type;
  };
    
  template <>
  struct model_traits<parser::Model3>
  {
    typedef model::Model3    model_type;
    typedef gradient::Model3 gradient_type;
    typedef parser::Model3   parser_type;
  };

  template <>
  struct model_traits<parser::Model4>
  {
    typedef model::Model4    model_type;
    typedef gradient::Model4 gradient_type;
    typedef parser::Model4   parser_type;
  };

  template <>
  struct model_traits<parser::Model5>
  {
    typedef model::Model5    model_type;
    typedef gradient::Model5 gradient_type;
    typedef parser::Model5   parser_type;
  };
};


#endif
