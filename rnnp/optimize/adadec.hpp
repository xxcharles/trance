// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__OPTIMIZE_ADADEC__HPP__
#define __RNNP__OPTIMIZE_ADADEC__HPP__ 1

// AN EMPIRICAL STUDY OF LEARNING RATES IN DEEP NEURAL NETWORKS FOR SPEECH RECOGNITION
//
// ICASSP 2013
//
// http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6638963
//
// slightly differ in that we do not memorize the history of delta, but
// use decaying (gamma == 0.95)

#include <rnnp/optimize.hpp>
#include <rnnp/model_traits.hpp>

#include <utils/mathop.hpp>

namespace rnnp
{
  namespace optimize
  {
    template <typename Theta>
    struct AdaDec : public Optimize
    {
      typedef typename model_traits<Theta>::model_type    model_impl_type;
      typedef typename model_traits<Theta>::gradient_type gradient_impl_type;
      
      AdaDec(const Theta& theta,
	     const option_type& option)
	: G_(theta),
	  lambda_(option.lambda_), eta0_(option.eta0_), epsilon_(option.epsilon_), gamma_(option.gamma_)
      { G_.clear(); }
      
      double decay()
      {
	eta0_ *= 0.5;
	return eta0_;
      }
      
      void operator()(model_impl_type& theta,
		      const gradient_impl_type& gradient,
		      const option_type& option) const;
      
      static inline
      double learning_rate(const double& eta0, const double& epsilon, const double& g)
      {
	const double rate eta0 / std::sqrt(epsilon + g);

	return std::isfinite(rate) ? rate : 1e-40;
      }
      
      struct update_visitor_regularize
      {
	update_visitor_regularize(tensor_type& theta,
				  tensor_type& G,
				  const tensor_type&  g,
				  const double& scale,
				  const double& lambda,
				  const double& eta0,
				  const double& epsilon,
				  const double& gamma)
	  : theta_(theta), G_(G), g_(g), scale_(scale), lambda_(lambda), eta0_(eta0), epsilon_(epsilon), gamma_(gamma) {}
      
	void init(const tensor_type::Scalar& value, tensor_type::Index i, tensor_type::Index j)
	{
	  operator()(value, i, j);
	}
      
	void operator()(const tensor_type::Scalar& value, tensor_type::Index i, tensor_type::Index j)
	{
	  if (g_(i, j) == 0) return;
	
	  G_(i, j) = G_(i, j) * gamma_ + (g_(i, j) * scale_) * (g_(i, j) * scale_);
	
	  const double rate = learning_rate(eta0_, epsilon_, G_(i, j));
	  const double x1 = theta_(i, j) - rate * scale_ * g_(i, j);
	
	  theta_(i, j) = utils::mathop::sgn(x1) * std::max(0.0, std::fabs(x1) - rate * lambda_);
	}
      
	tensor_type&       theta_;
	tensor_type&       G_;
	const tensor_type& g_;
      
	const double scale_;
	const double lambda_;
	const double eta0_;
	const double epsilon_;
	const double gamma_;
      };

      struct update_visitor
      {
	update_visitor(tensor_type& theta,
		       tensor_type& G,
		       const tensor_type&  g,
		       const double& scale,
		       const double& eta0,
		       const double& epsilon,
		       const double& gamma)
	  : theta_(theta), G_(G), g_(g), scale_(scale), eta0_(eta0), epsilon_(epsilon), gamma_(gamma) {}
      
	void init(const tensor_type::Scalar& value, tensor_type::Index i, tensor_type::Index j)
	{
	  operator()(value, i, j);
	}
      
	void operator()(const tensor_type::Scalar& value, tensor_type::Index i, tensor_type::Index j)
	{
	  if (g_(i, j) == 0) return;
	
	  G_(i, j) = G_(i, j) * gamma_ + (g_(i, j) * scale_) * (g_(i, j) * scale_);
	  
	  theta_(i, j) -= learning_rate(eta0_, epsilon_, G_(i, j)) * scale_ * g_(i, j);
	}
      
	tensor_type&       theta_;
	tensor_type&       G_;
	const tensor_type& g_;
      
	const double scale_;
	const double eta0_;
	const double epsilon_;
	const double gamma_;
      };
      
      void update(tensor_type& theta,
		  tensor_type& G,
		  const matrix_embedding_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	if (lambda_ != 0.0) {
	  matrix_embedding_type::const_iterator eiter_end = grad.end();
	  for (matrix_embedding_type::const_iterator eiter = grad.begin(); eiter != eiter_end; ++ eiter) {
	    const size_type col = eiter->first.id();
	    const tensor_type& g = eiter->second;
	  
	    for (tensor_type::Index row = 0; row != eiter->second.rows(); ++ row) 
	      if (g(row, 0) != 0.0) {
		G(row, col) = G(row, col) * gamma_ + (g(row, 0) * scale) * (g(row, 0) * scale);
		
		const double rate = learning_rate(eta0_, epsilon_, G(row, col));
		const double x1 = theta(row, col) - rate * scale * g(row, 0);
	      
		theta(row, col) = utils::mathop::sgn(x1) * std::max(0.0, std::fabs(x1) - rate * lambda_);
	      }
	  }
	} else {
	  matrix_embedding_type::const_iterator eiter_end = grad.end();
	  for (matrix_embedding_type::const_iterator eiter = grad.begin(); eiter != eiter_end; ++ eiter) {
	    const size_type col = eiter->first.id();
	    const tensor_type& g = eiter->second;
	  
	    for (tensor_type::Index row = 0; row != eiter->second.rows(); ++ row) 
	      if (g(row, 0) != 0.0) {
		G(row, col) = G(row, col) * gamma_ + (g(row, 0) * scale) * (g(row, 0) * scale);
		
		theta(row, col) -= learning_rate(eta0_, epsilon_, G(row, col)) * scale * g(row, 0);
	      }
	  }
	}
      }
      
      void update(tensor_type& theta,
		  tensor_type& G,
		  const matrix_category_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	if (regularize && lambda_ != 0.0) {
	  matrix_category_type::const_iterator giter_end = grad.end();
	  for (matrix_category_type::const_iterator giter = grad.begin(); giter != giter_end; ++ giter) {
	    const size_type rows = giter->second.rows();
	    const size_type cols = giter->second.cols();
	    const size_type offset = rows * giter->first.non_terminal_id();
	    
	    const tensor_type& g = giter->second;
	    
	    for (tensor_type::Index col = 0; col != g.cols(); ++ col) 
	      for (tensor_type::Index row = 0; row != g.rows(); ++ row) 
		if (g(row, col) != 0) {
		  G.block(offset, 0, rows, cols)(row, col) =
		    G.block(offset, 0, rows, cols)(row, col) * gamma_ + (g(row, col) * scale) * (g(row, col) * scale);
		  
		  tensor_type::Scalar& x = theta.block(offset, 0, rows, cols)(row, col);
		  
		  const double rate = learning_rate(eta0_, epsilon_, G.block(offset, 0, rows, cols)(row, col));
		  const double x1 = x - rate * scale * g(row, col);
		  
		  x = utils::mathop::sgn(x1) * std::max(0.0, std::fabs(x1) - rate * lambda_);
		}
	  }
	} else {
	  matrix_category_type::const_iterator giter_end = grad.end();
	  for (matrix_category_type::const_iterator giter = grad.begin(); giter != giter_end; ++ giter) {
	    const size_type rows = giter->second.rows();
	    const size_type cols = giter->second.cols();
	    const size_type offset = rows * giter->first.non_terminal_id();
	    
	    const tensor_type& g = giter->second;
	    
	    for (tensor_type::Index col = 0; col != g.cols(); ++ col) 
	      for (tensor_type::Index row = 0; row != g.rows(); ++ row) 
		if (g(row, col) != 0) {
		  G.block(offset, 0, rows, cols)(row, col) =
		    G.block(offset, 0, rows, cols)(row, col) * gamma_ + (g(row, col) * scale) * (g(row, col) * scale);
		  
		  const double rate = learning_rate(eta0_, epsilon_, G.block(offset, 0, rows, cols)(row, col));
		  
		  theta.block(offset, 0, rows, cols)(row, col) -= rate * scale * g(row, col);
		}
	  }
	}
      }

      void update(model_type::weights_type& theta,
		  model_type::weights_type& Gs,
		  const gradient_type::weights_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	if (lambda_ != 0.0) {
	  gradient_type::weights_type::const_iterator fiter_end = grad.end();
	  for (gradient_type::weights_type::const_iterator fiter = grad.begin(); fiter != fiter_end; ++ fiter) 
	    if (fiter->second != 0) {
	      model_type::weights_type::value_type& x = theta[fiter->first];
	      model_type::weights_type::value_type& G = Gs[fiter->first];
	      const gradient_type::weights_type::mapped_type& g = fiter->second;
	      
	      G = G * gamma_ + (g * scale) * (g * scale);
	      
	      const double rate = learning_rate(eta0_, epsilon_, G);
	      const double x1 = x - rate * scale * g;
	      
	      x = utils::mathop::sgn(x1) * std::max(0.0, std::fabs(x1) - rate * lambda_);
	    }
	} else {
	  gradient_type::weights_type::const_iterator fiter_end = grad.end();
	  for (gradient_type::weights_type::const_iterator fiter = grad.begin(); fiter != fiter_end; ++ fiter) 
	    if (fiter->second != 0) {
	      model_type::weights_type::value_type& x = theta[fiter->first];
	      model_type::weights_type::value_type& G = Gs[fiter->first];
	      const gradient_type::weights_type::mapped_type& g = fiter->second;
	      
	      G = G * gamma_ + (g * scale) * (g * scale);
	      
	      x -= learning_rate(eta0_, epsilon_, G) * scale * g;
	    }
	}
      }
      
      void update(tensor_type& theta,
		  tensor_type& G,
		  const tensor_type& g,
		  const double scale,
		  const bool regularize) const
      {
	if (regularize && lambda_ != 0.0) {
	  update_visitor_regularize visitor(theta, G, g, scale, lambda_, eta0_, epsilon_, gamma_);
	  
	  theta.visit(visitor);
	} else {
	  update_visitor visitor(theta, G, g, scale, eta0_, epsilon_, gamma_);
	  
	  theta.visit(visitor);
	}
      }
      
    private:
      Theta G_;
      
      double lambda_;
      double eta0_;
      double epsilon_;
      double gamma_;
    };

    template <>
    inline
    void AdaDec<model::Model1>::operator()(model::Model1& theta,
					   const gradient::Model1& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
	
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model2>::operator()(model::Model2& theta,
					   const gradient::Model2& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model3>::operator()(model::Model3& theta,
					   const gradient::Model3& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);

	update(theta.Wqu_, G.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, G.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, G.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model4>::operator()(model::Model4& theta,
					   const gradient::Model4& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model5>::operator()(model::Model5& theta,
					   const gradient::Model5& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);

	update(theta.Wqu_, G.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, G.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, G.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model6>::operator()(model::Model6& theta,
					   const gradient::Model6& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void AdaDec<model::Model7>::operator()(model::Model7& theta,
					   const gradient::Model7& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);

	update(theta.Wshr_, G.Wshr_, gradient.Wshr_, scale, true);
	update(theta.Bshr_, G.Bshr_, gradient.Bshr_, scale, false);

	update(theta.Wshz_, G.Wshz_, gradient.Wshz_, scale, true);
	update(theta.Bshz_, G.Bshz_, gradient.Bshz_, scale, false);
	
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);

	update(theta.Wrer_, G.Wrer_, gradient.Wrer_, scale, true);
	update(theta.Brer_, G.Brer_, gradient.Brer_, scale, false);

	update(theta.Wrez_, G.Wrez_, gradient.Wrez_, scale, true);
	update(theta.Brez_, G.Brez_, gradient.Brez_, scale, false);
	
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wur_, G.Wur_, gradient.Wur_, scale, true);
	update(theta.Bur_, G.Bur_, gradient.Bur_, scale, false);

	update(theta.Wuz_, G.Wuz_, gradient.Wuz_, scale, true);
	update(theta.Buz_, G.Buz_, gradient.Buz_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);

	update(theta.Wqu_, G.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, G.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, G.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }
    
    template <>
    inline
    void AdaDec<model::Model8>::operator()(model::Model8& theta,
					   const gradient::Model8& gradient,
					   const option_type& option) const
    {
      if (! gradient.count_) return;

      const double scale = 1.0 / gradient.count_;
      
      model_impl_type& G = const_cast<model_impl_type&>(G_);
      
      if (option.learn_embedding())
	update(theta.terminal_, G.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  G.Wc_,  gradient.Wc_, scale, true);
	update(theta.Bc_,  G.Bc_,  gradient.Bc_, scale, false);
	update(theta.Wfe_, G.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Psh_, G.Psh_, gradient.Psh_, scale, true);
	update(theta.Qsh_, G.Qsh_, gradient.Qsh_, scale, true);
	update(theta.Wsh_, G.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, G.Bsh_, gradient.Bsh_, scale, false);
	
	update(theta.Pre_, G.Pre_, gradient.Pre_, scale, true);
	update(theta.Qre_, G.Qre_, gradient.Qre_, scale, true);
	update(theta.Wre_, G.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, G.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Pu_, G.Pu_, gradient.Pu_, scale, true);
	update(theta.Qu_, G.Qu_, gradient.Qu_, scale, true);
	update(theta.Wu_, G.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, G.Bu_, gradient.Bu_, scale, false);

	update(theta.Wf_, G.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, G.Bf_, gradient.Bf_, scale, false);

	update(theta.Wi_, G.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, G.Bi_, gradient.Bi_, scale, false);

	update(theta.Wqu_, G.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, G.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, G.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, G.Ba_, gradient.Ba_, scale, false);
      }
    }

  };
};

#endif
