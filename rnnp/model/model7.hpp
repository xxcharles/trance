// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__MODEL__MODEL7__HPP__
#define __RNNP__MODEL__MODEL7__HPP__ 1

#include <rnnp/model.hpp>

#include <boost/random/uniform_real_distribution.hpp>

namespace rnnp
{
  namespace model
  {
    class Model7 : public Model
    {
    public:
      
    public:
      Model7() : Model() {}
      Model7(const path_type& path) { read(path); }
      Model7(const size_type& hidden,
	     const size_type& embedding,
	     const grammar_type& grammar)
      { initialize(hidden, embedding, grammar); }
      
      void initialize(const size_type& hidden,
		      const size_type& embedding,
		      const grammar_type& grammar);
      
      // IO
      void write(const path_type& path) const;
      void read(const path_type& path);
      void embedding(const path_type& path);
    
      friend
      std::ostream& operator<<(std::ostream& os, const Model7& x);
      friend
      std::istream& operator>>(std::istream& is, Model7& x);
    
      Model7& operator+=(const Model7& x);
      Model7& operator-=(const Model7& x);
      Model7& operator*=(const double& x);
      Model7& operator/=(const double& x);
    
    private:
      template <typename Gen>
      struct __randomize
      {
	__randomize(Gen& gen, const double range=0.01) : gen_(gen), range_(range) {}
      
	template <typename Tp>
	Tp operator()(const Tp& x) const
	{
	  return boost::random::uniform_real_distribution<Tp>(-range_, range_)(const_cast<Gen&>(gen_));
	}
      
	Gen& gen_;
	double range_;
      };

    public:
      template <typename Gen>
      void random(Gen& gen)
      {
	const double range_embed = std::sqrt(6.0 / (embedding_ + 1));
	const double range_c  = std::sqrt(6.0 / (hidden_ + 1));
	const double range_sh = std::sqrt(6.0 / (hidden_ + hidden_ + embedding_ + hidden_));
	const double range_re = std::sqrt(6.0 / (hidden_ + hidden_ + hidden_ + hidden_ + hidden_));
	const double range_u  = std::sqrt(6.0 / (hidden_ + hidden_ + hidden_ + hidden_));
	const double range_qu = std::sqrt(6.0 / (hidden_ + hidden_ + embedding_));
	const double range_f  = std::sqrt(6.0 / (hidden_ + hidden_));
	const double range_i  = std::sqrt(6.0 / (hidden_ + hidden_));
	
	terminal_ = terminal_.array().unaryExpr(__randomize<Gen>(gen, range_embed));
	
	Wc_ = Wc_.array().unaryExpr(__randomize<Gen>(gen, range_c));
	
	Wsh_  = Wsh_.array().unaryExpr(__randomize<Gen>(gen, range_sh));
	
	Wshr_ = Wshr_.array().unaryExpr(__randomize<Gen>(gen, range_sh));
	Wshz_ = Wshz_.array().unaryExpr(__randomize<Gen>(gen, range_sh));
	
	Wre_ = Wre_.array().unaryExpr(__randomize<Gen>(gen, range_re));

	Wrer_ = Wrer_.array().unaryExpr(__randomize<Gen>(gen, range_re));
	Wrez_ = Wrez_.array().unaryExpr(__randomize<Gen>(gen, range_re));
	
	Wu_  = Wu_.array().unaryExpr(__randomize<Gen>(gen, range_u));

	Wur_  = Wur_.array().unaryExpr(__randomize<Gen>(gen, range_u));
	Wuz_  = Wuz_.array().unaryExpr(__randomize<Gen>(gen, range_u));
	
	Wqu_ = Wqu_.array().unaryExpr(__randomize<Gen>(gen, range_qu));
	
	Wf_ = Wf_.array().unaryExpr(__randomize<Gen>(gen, range_f));
	Wi_ = Wi_.array().unaryExpr(__randomize<Gen>(gen, range_i));
      }
      
      void swap(Model7& x)
      {
	Model::swap(static_cast<Model&>(x));
	
	terminal_.swap(x.terminal_);
      
	Wc_.swap(x.Wc_);
	Bc_.swap(x.Bc_);
	Wfe_.swap(x.Wfe_);
      
	Wsh_.swap(x.Wsh_);
	Bsh_.swap(x.Bsh_);

	Wshr_.swap(x.Wshr_);
	Bshr_.swap(x.Bshr_);

	Wshz_.swap(x.Wshz_);
	Bshz_.swap(x.Bshz_);
	
	Wre_.swap(x.Wre_);
	Bre_.swap(x.Bre_);

	Wrer_.swap(x.Wrer_);
	Brer_.swap(x.Brer_);

	Wrez_.swap(x.Wrez_);
	Brez_.swap(x.Brez_);
	
	Wu_.swap(x.Wu_);
	Bu_.swap(x.Bu_);

	Wur_.swap(x.Wur_);
	Bur_.swap(x.Bur_);

	Wuz_.swap(x.Wuz_);
	Buz_.swap(x.Buz_);
	
	Wqu_.swap(x.Wqu_);
	Bqu_.swap(x.Bqu_);
	Bqe_.swap(x.Bqe_);

	Wf_.swap(x.Wf_);
	Bf_.swap(x.Bf_);
      
	Wi_.swap(x.Wi_);
	Bi_.swap(x.Bi_);
      
	Ba_.swap(x.Ba_);
      }
    
      void clear()
      {
	Model::clear();

	terminal_.setZero();
      
	Wc_.setZero();
	Bc_.setZero();
	Wfe_.clear();
      
	Wsh_.setZero();
	Bsh_.setZero();

	Wshr_.setZero();
	Bshr_.setZero();

	Wshz_.setZero();
	Bshz_.setZero();

	Wre_.setZero();
	Bre_.setZero();

	Wrer_.setZero();
	Brer_.setZero();

	Wrez_.setZero();
	Brez_.setZero();

	Wu_.setZero();
	Bu_.setZero();

	Wur_.setZero();
	Bur_.setZero();

	Wuz_.setZero();
	Buz_.setZero();
	
	Wf_.setZero();
	Bf_.setZero();
      
	Wi_.setZero();
	Bi_.setZero();

	Wqu_.setZero();
	Bqu_.setZero();
	Bqe_.setZero();
	
	Ba_.setZero();
      }
    
    public:
      double l1() const
      {
	double norm = 0.0;
      
	norm += Wc_.lpNorm<1>();
	
	norm += Wsh_.lpNorm<1>();
	norm += Wshr_.lpNorm<1>();
	norm += Wshz_.lpNorm<1>();
	
	norm += Wre_.lpNorm<1>();
	norm += Wrer_.lpNorm<1>();
	norm += Wrez_.lpNorm<1>();
      
	norm += Wu_.lpNorm<1>();
	norm += Wur_.lpNorm<1>();
	norm += Wuz_.lpNorm<1>();

	norm += Wf_.lpNorm<1>();
	norm += Wi_.lpNorm<1>();
	
	norm += Wqu_.lpNorm<1>();
	
	return norm;
      }
    
      double l2() const
      {
	double norm = 0.0;
      
	norm += Wc_.squaredNorm();
      
	norm += Wsh_.squaredNorm();
	norm += Wshr_.squaredNorm();
	norm += Wshz_.squaredNorm();
	
	norm += Wre_.squaredNorm();
	norm += Wrer_.squaredNorm();
	norm += Wrez_.squaredNorm();
      
	norm += Wu_.squaredNorm();
	norm += Wur_.squaredNorm();
	norm += Wuz_.squaredNorm();
	
	norm += Wf_.squaredNorm();
	norm += Wi_.squaredNorm();
	
	norm += Wqu_.squaredNorm();
	
	return std::sqrt(norm);
      }

      void precompute();
      
    public:
      // cache
      tensor_type cache_;
      
      // terminal embedding
      tensor_type terminal_;
    
      // classification
      tensor_type Wc_;
      tensor_type Bc_;
      
      // features
      weights_type Wfe_;
    
      // shift
      tensor_type Wsh_;
      tensor_type Bsh_;

      // shift reset
      tensor_type Wshr_;
      tensor_type Bshr_;

      // shift update
      tensor_type Wshz_;
      tensor_type Bshz_;
    
      // reduce
      tensor_type Wre_;
      tensor_type Bre_;

      // reduce reset
      tensor_type Wrer_;
      tensor_type Brer_;

      // reduce update
      tensor_type Wrez_;
      tensor_type Brez_;
      
      // unary
      tensor_type Wu_;
      tensor_type Bu_;

      // unary reset
      tensor_type Wur_;
      tensor_type Bur_;

      // unary update
      tensor_type Wuz_;
      tensor_type Buz_;

      // final
      tensor_type Wf_;
      tensor_type Bf_;

      // idle
      tensor_type Wi_;
      tensor_type Bi_;
      
      // queue
      tensor_type Wqu_;
      tensor_type Bqu_;
      tensor_type Bqe_;
      
      // axiom
      tensor_type Ba_;
    };
  };
};

namespace std
{
  inline
  void swap(rnnp::model::Model7& x, rnnp::model::Model7& y)
  {
    x.swap(y);
  }
};

#endif
