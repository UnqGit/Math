#pragma once
#include <cmath>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <functional>

//make sure that parsing mechanism is included for
//further ease and proper evaluation
#include "MathParse.h"

namespace SSM{
    
//used to calculate all derivatives of a expression
//from order 0 to n (where cap on n is 500 or warn and 
//ask for order again) at a specific point.

class numericalDer{
  
  using container = std::vector<long double>;
  
  public:
    
    numericalDer(bool a = false): LoadThrow(a){}
    
    //main function(s) to be used
    //overloaded functions for ease-of-use
    container numerical_der(){
      expression expr = get_expression();
      int order = get_order();
      if(expr.is_constant()) order = 0;
      fill_nCr(order);
      long double point = get_point();
      load_throw(expr.result(), order);
      if(unary_funcs.empty()) make_map();
      container result = unified_diff(expr.result(), order, point);
      return result;
    }
    container numerical_der(expression& expr){
      validate_expr(expr);
      int order = get_order();
      if(expr.is_constant()) order = 0;
      fill_nCr(order);
      long double point = get_point();
      load_throw(expr.result(), order);
      if(unary_funcs.empty()) make_map();
      container result = unified_diff(expr.result(), order, point);
      return result;
    }
    container numerical_der(expression& expr, int order){
      validate_expr(expr);
      if(expr.is_constant()) order = 0;
      validate_order(order);
      fill_nCr(order);
      long double point = get_point();
      load_throw(expr.result(), order);
      if(unary_funcs.empty()) make_map();
      container result = unified_diff(expr.result(), order, point);
      return result;
    }
    container numerical_der(expression& expr, int order, long double point){
      validate_expr(expr);
      if(expr.is_constant()) order = 0;
      validate_order(order);
      fill_nCr(order);
      load_throw(expr.result(), order);
      if(unary_funcs.empty()) make_map();
      container result = unified_diff(expr.result(), order, point);
      return result;
    }
    
    void optimize(){this->LoadThrow = true;}
    
    void dismiss(){this->LoadThrow = false;}
    
    void clear(){this->nCr.clear();this->unary_funcs.clear();this->binary_funcs.clear();}
    
  private:
    
    bool LoadThrow;
    
    static constexpr long double PI = 3.141592653589793238462643383279502884197L;
        
    std::vector<container> nCr;
    
    void make_map(){
        unary_funcs["exp"] = std::bind(&numericalDer::exp_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["ln"] = std::bind(&numericalDer::ln_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["sin"] = std::bind(&numericalDer::sin_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["cos"] = std::bind(&numericalDer::cos_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["csc"] = std::bind(&numericalDer::csc_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["sec"] = std::bind(&numericalDer::sec_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["tan"] = std::bind(&numericalDer::tan_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["cot"] = std::bind(&numericalDer::cot_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["asin"] = std::bind(&numericalDer::asin_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["acos"] = std::bind(&numericalDer::acos_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["acsc"] = std::bind(&numericalDer::acsc_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["asec"] = std::bind(&numericalDer::asec_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["atan"] = std::bind(&numericalDer::atan_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["acot"] = std::bind(&numericalDer::acot_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["sinh"] = std::bind(&numericalDer::sinh_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["cosh"] = std::bind(&numericalDer::cosh_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["csch"] = std::bind(&numericalDer::csch_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["sech"] = std::bind(&numericalDer::sech_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["tanh"] = std::bind(&numericalDer::tanh_derivatives, this, std::placeholders::_1, std::placeholders::_2);
        unary_funcs["coth"] = std::bind(&numericalDer::coth_derivatives, this, std::placeholders::_1, std::placeholders::_2);

        binary_funcs["pow"] = std::bind(&numericalDer::pow_derivatives, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        binary_funcs["add"] = std::bind(&numericalDer::add_derivatives, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        binary_funcs["sub"] = std::bind(&numericalDer::sub_derivatives, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        binary_funcs["mul"] = std::bind(&numericalDer::mul_derivatives, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        binary_funcs["div"] = std::bind(&numericalDer::div_derivatives, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    
    void load_throw(const std::string& expr, int order){
      if(!this->LoadThrow) return;
      int count = std::count(expr.begin(), expr.end(), '(');
      if(order*count>5000) throw std::runtime_error("Function load is heavy, consider allowing load_heavy order+expression to continue.");
    }
    
    //fill nCr with correct number of terms
    void fill_nCr(int order){
      if(order < nCr.size()) return;
      int start = nCr.size();
      nCr.resize(order + 1);
      for(int i = start; i < order + 1; i++){
        nCr[i].resize(i + 1, 0.0L);
        nCr[i][0] = 1.0L;
        for(int j = 1; j < i; j++){
          nCr[i][j] = nCr[i-1][j-1] + nCr[i-1][j];
        }
        nCr[i][i] = 1.0L;
      }
    }
    
    bool isConstant(const std::string& input){
      size_t t = input.find("x");
      if(t == 0) return false;
      while(t!=std::string::npos){
        if(input[t - 1]!='e') return false;
        t = input.find("x", t+1);
      }
      return true;
    }
    
    //parse string into two substring for binary operations
    std::pair<std::string, std::string> split_string(const std::string& input){
      size_t t = 0;
      int parenthesis = 0;
      for(int i = 0; i < input.length(); i++){
        if(input[i]=='(')parenthesis++;
        else if(input[i]==')'){
          parenthesis--;
          if(parenthesis<0) throw std::runtime_error("An unexpected error occured (error type: invalid parenthesis).");
        }
        else if(input[i]==','&&parenthesis==0){
          t = (long long)i;
          break;
        }
      }
      std::string substr1 = input.substr(0, t);
      std::string substr2 = input.substr(t+1);
      if(substr1.empty()||substr2.empty()) throw std::runtime_error("An unexpected error occured (error type: single argument in binary function).");
      return {substr1, substr2};
    }
    
    int pastNonZero(const container& v){
      if(v.empty()) return 0;
      size_t j = v.size()-1;
      while(!(j<0)&&v[j]==0) j--;
      return j + 1;
    }
      
    //following function is general result of applying derivatives
    //on two summed functions 
    container add_derivatives(const container& u, const container& v, int order){
      size_t max = std::max(u.size(), v.size());
      container f(max, 0);
      for(int i = 0; i < u.size(); i++) f[i]+=u[i];
      for(int i = 0; i < v.size(); i++) f[i]+=v[i];
      return f;
    }
    
    //following function is general result of applying derivatives
    //on two subtracted functions 
    container sub_derivatives(const container& u, const container& v, int order){
      size_t max = std::max(u.size(), v.size());
      container f(max, 0);
      for(int i = 0; i < u.size(); i++) f[i]+=u[i];
      for(int i = 0; i < v.size(); i++) f[i]-=v[i];
      return f;
    }
    
    //when derivating two multiplied equations, I noticed that:
    //f(n) = S{i=0,n}(nCr{n,i}·u(n-i)·v(n))
    //where f(n), u(n), v(n) means nth derivative of the respective fucntion
    //nCr is binomial coefficients and S{i=a,b} is summarion over a to b
    container mul_derivatives(const container& u, const container& v, int order){
      if(u.size()==1&&v.size()==1) order = 0;
      else if(u.size()==1) order = v.size() - 1;
      else if(v.size()==1) order = u.size() - 1;
      container f;
      if((u.size()==1&&u[0]==0)||(v.size()==1&&v[0]==0)){
        f = {0};
        return f;
      }
      f.resize(order + 1, 0);
      for(int n = 0; n < order + 1; n++){
        container& NCR = nCr[n];
        for(int i = 0; i < n+1; i++){
          if((n-i)<u.size()&&(i)<v.size())
            f[n] += NCR[i]*u[n-i]*v[i];
        }
      }
      return f;
    }
    
    //I found that when derivating an equation
    //f = u/v
    //the general result for f(n) comes out to be:
    //f(n) = (u(n) - S{i=0,n-1}(nCr{n,i}·v(n-i)·f(i)))/v
    //or:
    //f(n) = (u(n) - S{i=1,n}(nCr{n,i}·v(i)·f(n-i)))/v
    //we'll be using the second form for better optimization.
    container div_derivatives(const container& u, const container& v, int order){
      if(u.size()==1&&v.size()==1) order = 0;
      if(v[0] == 0) throw std::runtime_error("Division by 0 occurs, please make sure to input correct point(inside domain of the function) to evaluate");
      container f;
      //if u is constant and its value is 0.
      if(u.size()==1&&u[0]==0){
        f = {0};
        return f;
      }
      f.resize(order + 1,0);
      int j = pastNonZero(v);
      for(int n = 0; n < order + 1; n++){
        int min = std::min(j, n + 1);
        container& NCR = nCr[n];
        for(int i = 1; i < min; i++){
          f[n] -= NCR[i]*v[i]*f[n-i];
        }
        if(n < u.size())f[n] += u[n];
        f[n] /= v[0];
      }
      return f;
    }
    
    //by deriving it, i found that for the function
    //f = exp(u), the higher order derivatives are as follows:
    //f(n) = S{i=0,n-1}(nCr{n-1,i}·f(i)·u(n-i))
    //or:
    //f(n) = S{i=1,n}(nCr{n-1,i-1}·f(n-i)·u(i))
    //we'll be using the second formula for optimal route.
    container exp_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container f(order + 1,0);
      int k = pastNonZero(u);
      f[0] = std::exp(u[0]);
      for(int n = 1; n < order + 1; n++){
        int min = std::min(k,n+1);
        container& NCR = nCr[n-1];
        for(int i = 1; i < min; i++){
          f[n] += NCR[i-1]*f[n-i]*u[i];
        }
      }
      return f;
    }
    
    //by deriving it, i found that for the function:
    //f = sqrt(u), the higher order derivatives are as follows:
    //f(n) = (u(n) - S{i=1,n-1}(nCr{n,i}·f(i)·f(n-i)))/(2f(0))
    container sqrt_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(u[0]<0) throw std::runtime_error("Sqrt cannot contain negative number (please check the expression you provided).");
      container f(order + 1,0);
      f[0] = std::sqrt(u[0]);
      if(u[0]==0&&order>0) throw std::runtime_error("Calculting derivative of sqrt at infinity, result undefined, please check the expression you provided.");
      for(int n = 0; n < order + 1; n++){
        container& NCR = nCr[n];
        for(int i = 1; i < n; i++){
          f[n] -= NCR[i]*f[i]*f[n-i];
        }
        if(n < u.size()) f[n] += u[n];
        f[n] /= 2*f[0];
      }
      return f;
    }
    
    //by deriving it, i found that for the function:
    //f = ln(u), the higher order derivatives are as follows:
    //f(n) = (u(n) - S{i=1,n-1}(nCr{n-1,i}·f(n-i)·u(i)))/(u(0))
    //will find last non zero term in this too for optimal use.
    container ln_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      int k = pastNonZero(u);
      if(u[0]<=0) throw std::runtime_error("The value in logarithm can't be negative or 0, please check your expression.");
      container f(order + 1, 0);
      f[0] = std::log(u[0]);
      for(int n = 1; n < order + 1; n++){
        container& NCR = nCr[n-1];
        int min = std::min(k,n);
        for(int i = 1; i < min; i++){
          f[n] -= NCR[i]*f[n-i]*u[i];
        }
        if(n < u.size()) f[n] += u[n];
        f[n] /= u[0];
      }
      return f;
    }
    
    //this is just a trick as i didn't try to find the formula
    //for asin or acos but i do know that they likely involve
    //double summation, so just use exp derivation as:
    //sinh = (e(u)-e(-u))/2 and cosh = (e(u)+e(-u))/2
    container sinh_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container neg_u(u.size());
      std::transform(u.begin(), u.end(), neg_u.begin(), [](long double p){return -p;});
      container pos_eu = exp_derivatives(u, order);
      container neg_eu = exp_derivatives(neg_u, order);
      container f(order + 1);
      for(int n = 0; n < order + 1; n++){
        f[n] = 0.5L*(pos_eu[n] - neg_eu[n]);
      }
      return f;
    }
    
    container cosh_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container neg_u(u.size());
      std::transform(u.begin(), u.end(), neg_u.begin(), [](long double p){return -p;});
      container pos_eu = exp_derivatives(u, order);
      container neg_eu = exp_derivatives(neg_u, order);
      container f(order + 1);
      for(int n = 0; n < order + 1; n++){
        f[n] = 0.5L*(pos_eu[n] + neg_eu[n]);
      }
      return f;
    }
    
    container csch_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(u[0]==0) throw std::runtime_error("Hyperbolic cosecant (cosech/csch) is undefined at argument = 0, please check your expression.");
      container f = sinh_derivatives(u, order);
      container n = {1.0};
      return div_derivatives(n, f, order);
    }
    
    container sech_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container f = cosh_derivatives(u, order);
      container n = {1.0};
      return div_derivatives(n, f, order);
    }
    
    //the direct formula i derived is O(n³) for calculating 
    //all the derivatives from 0 to n, but we can use some
    //tricks here too to get it down to O(n²) but since
    //n³ ≤ 2n+n²(1exp, 1div, 1adding) for n ≤ 2.414(or ~2)(pretty early), i will not be using the formula i derived.
    //Direct formula for f = tanh(u):
    //f(n) = (1 - (f(0))²)·u(n) - 2·S{i=0,n-2}(nCr{n-1,i}·u(i+1)·S{j=0,n-2-i}(nCr{n-2-i,j}·f(j)·f(n-i-j-1)))
    container tanh_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      //included the implementation if you're interested
      //this should give a clear picture of why the 
      //direct approach is expensive.
      
      /*
      if(order + 1 < 27){
        container f(order + 1, 0);
        f[0] = std::tanh(u[0]);
        long double helper = 1.0 - f[0] * f[0];
        
        for(int n = 1; n < order + 1; n++){
          
          long double result = 0.0;
          container& factor = nCr[n-1];
          
          for(int i = 0; i < n - 1; i++){
            long double inner_result = 0;
            if((i + 1) < u.size()){
              
              //accessing it is fine as the loop bollean
              //will always be true only when it is able
              //to access the layer of binomial coefficients.
              
              container& NCR = nCr[n-i-2];
              for(int j = 0; j < n - i - 1; j++){
                inner_result += NCR[j] * f[j] * f[n - i - j - 1];
              }
            }
            result += factor[i] * u[i + 1] * inner_result;
          }
          f[n] = -2.0 * result;
          if(n < u.size()) f[n] += helper * u[n];
        }
        return f;
      }
      */
      
      //f = tanh(u) = (e^(u)-e^(-u))/(e^(u)+e^(-u)):
      //f = (e^(2u) - 1)/(e^(2u) + 1)
      //f = u/v:
      //u = e^(2u) - 1 : v = e^(2u) + 1
      
      container u2(u.size());
      std::transform(u.begin(), u.end(), u2.begin(), [](long double p){return 2.0 * p;});
      container eu2n = exp_derivatives(u2, order);
      container eu2d = eu2n;
      eu2n[0] -= 1;
      eu2d[0] += 1;
      return div_derivatives(eu2n, eu2d, order);
    }
    
    container coth_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container u2(u.size());
      std::transform(u.begin(), u.end(), u2.begin(), [](long double p){return 2.0 * p;});
      container eu2n = exp_derivatives(u2, order);
      container eu2d = eu2n;
      eu2d[0] -= 1;
      eu2n[0] += 1;
      return div_derivatives(eu2n, eu2d, order);
    }
    
    //minimal complex struct, useful for optimizing trig functions
    struct complex{
      long double r;
      long double i;
      complex(): r(0.0L), i(0.0L){}
      complex(long double val): r(val), i(0.0L){}
      complex operator*(long double val){
        complex result;
        result.r = this->r*val;
        result.i = this->i*val;
        return result;
      }
      complex& operator+=(const complex& other){
        this->r += other.r;
        this->i += other.i;
        return *this;
      }
      void I(){
        long double a = this->i;
        this->i = this->r;
        this->r = -a;
      }
    };
    
    //helper for trig function derivations
    //same as exp but each derivative is also multiplied by
    //i when calculating using recursion
    std::vector<complex> exp_polar_der(const container& u, int order){
      if(u.size()==1) order = 0;
      std::vector<complex> f(order + 1, 0.0L);
      int k = pastNonZero(u);
      f[0].r = std::cos(u[0]);
      f[0].i = std::sin(u[0]);
      for(int n = 1; n < order + 1; n++){
        int limit = std::min(k, n+1);
        container& NCR = nCr[n-1];
        for(int i = 1; i < limit; i++){
          f[n] += f[n-i]*(u[i]*NCR[i-1]);
        }
        f[n].I();
      }
      return f;
    }
    
    //i did derive formula for this too but that is O(n³) too
    //so we will be using a clever little trick by using the formula:
    //sin(u) = (exp(i·u)-exp(-i·u))/2, hence the exp_polar_der above
    //the original recurrence relation i found was:
    //f = sin(u(0))
    //f(n) = cos(u(0))u(n) - S{i=1,n-1}(nCr{n-1,i}·u(n-i)·S{j=0,i-1}(nCr{i-1,j}·u(i-j)·f(j)))
    container sin_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      //if you're interested in the actual recurrence relation
      //that doesn't use polar exp, here is an implementation:
      //this is also O(n³) like tanh and is the same reason
      //why I'll not be using it.
      //also the formula equates to
      //cos(u(0))u(n) - S{i=1,n-1}(nCr{n-1,i-1}·u(i)·S{j=1,n-i}(nCr{n-i-1,j-1}·u(j)·f(n-i-j)))
      
      /*
        container f(order + 1, 0);
        f[0] = std::sin(u[0]);
        int k = u.size()-1;
        while(!(k<0)&&u[k]==0) k--;
        k++;
        long double helper = std::cos(u[0]);
        for(int n = 1; n < order + 1; n++){
          long double outer = 0.0;
          int min = std::min(n, k);
          container& factor = nCr[n-1];
          for(int i = 1; i < min; i++){
            long double inner = 0.0;
            container& NCR = nCr[n-i-1];
            int minP = std::min(n-i+1,k);
            for(int j = 1; j < minP; j++){
              inner += NCR[j-1]*u[j]*f[n-i-j];
            }
            outer += factor[i-1]*u[i]*inner;
          }
          f[n] = -outer;
          if(n < u.size()) f[n] += helper*u[n];
        }
        return f;
      */
      
      std::vector<complex> eu = exp_polar_der(u,order);
      container f(order + 1, 0);
      for(int i = 0; i < order + 1; i++){
        f[i] = eu[i].i;
      }
      return f;
    }
    
    //I derived for this too, and its just a minor change from that
    //of sin's recursion relation i derived but it's still not good(O(n³))
    //so I'll be using the euler's identity again.
    //formula for f = cos(u) if you're interested is:    
    //-sin(u(0))u(n) - S{i=1,n-1}(nCr{n-1,i-1}·u(i)·S{j=1,n-i}(nCr{n-i-1,j-1}·u(j)·f(n-i-j)))
    container cos_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      std::vector<complex> eu = exp_polar_der(u,order);
      container f(order + 1, 0);
      for(int i = 0; i < order + 1; i++){
        f[i] = eu[i].r;
      }
      return f;
    }
    
    //it has similar formula as tanh and it is:
    //f = tan(u);
    //f(n) = (1 + (f(0))²)·u(n) + 2·S{i=0,n-2}(nCr{n-1,i}·u(i+1)·S{j=0,n-2-i}(nCr{n-2-i,j}·f(j)·f(n-i-j-1)))
    container tan_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(std::remainder(u[0], PI/2.0)) < 1e-18) throw std::runtime_error("Value in tan is at singularity, please re-configure your expression or choose a different point for evaluation.");
      std::vector<complex> eu = exp_polar_der(u, order);
      container sin(order + 1, 0);
      container cos(order + 1, 0);
      for(int i = 0; i < order + 1; i++){
        sin[i] = eu[i].i;
        cos[i] = eu[i].r;
      }
      return div_derivatives(sin, cos, order);
    }
    
    container csc_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(std::remainder(u[0], PI)) < 1e-18) throw std::runtime_error("Value in cosec is at singularity, please re-configure your expression or choose a different point for evaluation.");
      container n = {1.0L};
      return div_derivatives(n, sin_derivatives(u,order), order);
    }
    
    container sec_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(std::remainder(u[0], PI/2.0)) < 1e-18) throw std::runtime_error("Value in sec is at singularity, please re-configure your expression or choose a different point for evaluation.");
      container n = {1.0L};
      return div_derivatives(n, cos_derivatives(u,order), order);
    }
    
    container cot_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(std::remainder(u[0], PI)) < 1e-18) throw std::runtime_error("Value in tan is at singularity, please re-configure your expression or choose a different point for evaluation.");
      std::vector<complex> eu = exp_polar_der(u, order);
      container sin(order + 1, 0);
      container cos(order + 1, 0);
      for(int i = 0; i < order + 1; i++){
        sin[i] = eu[i].i;
        cos[i] = eu[i].r;
      }
      return div_derivatives(cos, sin, order);
    }
    
    //constant power
    //formula that i had derived:
    //f = u^c:
    //f(n) = (c·u(n)·f(0)+S{i=1,n-1}(f(i)·u(n-i)·(nCr{n-1,i}·c-nCr{n-1,i-1})))/u(0)
    //or f(n) = (c·u(n)·f(0)+S{i=1,n-1}(f(n-i)·u(i)·(nCr{n-1,i}-nCr{n-1,i-1}·c)))/u(0)
    //we'll be using the second method since that allows for more optimization.
    container pow_const_power(const container& u, long double c, int order){
      container f;
      if(c==0){
        //if size of u isn't 0 then i know that order > 0 as that's the only chance any size can be greater than 1
        if(u.size()!=1&&u[0]==0) throw std::runtime_error("Derivation of 0⁰ values function is undefined at this point, please look into your expression and re-enter.");
        f = {1.0L};
        return f;
      }
      if(u.size()==1&&u[0]==0){
        f = {0.0L};
        return f;
      }
      f.resize(order + 1, 0.0L);
      if(u[0]==0) return f;
      int k = pastNonZero(u);
      f[0] = std::pow(u[0], c);
      //case of negative to the power of something that's not divisble by two and negative
      if(std::isnan(f[0])) throw std::runtime_error("Function is undefined at the point, issue was in pow, please check your expression or point of evaluation.");
      for(int n = 1; n < order + 1; n++){
        int limit = std::min(k, n);
        container& NCR = nCr[n-1];
        long double result = 0.0L;
        for(int i = 1; i < limit; i++){
          result += u[i]*f[n-i]*(NCR[i]-NCR[i-1]*c);
        }
        if(n < u.size()) f[n] = c*f[0]*u[n];
        f[n] -= result;
        f[n] /= u[0];
      }
      return f;
    }
    
    //the relation that I derived for this case is:
    //f = c^u:
    //f(n) = ln(c)·(S{i=0,n-1}(nCr{n-1,i}·f(i)·u(n-i)))
    //or f(n) = ln(c)·(S{i=1,n}(nCr{n-1,i}·u(i)·f(n-i)))
    //both are equivalent(we'll be using the second relation).
    container pow_const_base(const container& u, long double c, int order){
      container f;
      if(c == 0){
        if(u[0] >= 0){
          f = {0};
          return f;
        }
        else throw std::runtime_error("Function is undefined at this point, please check your expression (error: 0^(a<0) or re-enter.)");
      }
      f.resize(order + 1, 0.0L);
      f[0] = std::pow(c, u[0]);
      int k = pastNonZero(u);
      //case of negative to the power of something that's not divisble by two and negative
      if(std::isnan(f[0])) throw std::runtime_error("Function definition is invalid at the point or either expression is invalid, please check and re-enter.");
      long double helper = std::log(c);
      if(order>0&&std::isnan(helper)) throw std::runtime_error("Function is discontinuous at this point, unable to calculate derivative(s) (error: c^(u): c is negative), please check your expression, or check the point.");
      for(int n = 1; n < order + 1; n++){
        int limit = std::min(k,n+1);
        long double result = 0.0L;
        container& NCR = nCr[n-1];
        for(int i = 1; i < limit; i++){
          result += NCR[i]*f[n-i]*u[i];
        }
        f[n] = result*helper;
      }
      return f;
    }
    
    //implicit handling of different cases
    container pow_derivatives(const container& u, const container& v, int order){
      if(u.size()==1&&v.size()==1) order = 0;
      if(v.size()==1) return pow_const_power(u, v[0], order);
      //following are remaining at the moment
      if(u.size()==1) return pow_const_base(v, u[0], order);
      if(u[0]==0&&v[0]==0) throw std::runtime_error("Function definition is invalid at the point or either expression is invalid, please check and re-enter.");
      long double check = std::pow(u[0], v[0]);
      if(std::isnan(check)) throw std::runtime_error("Function definition is invalid at the point or either expression is invalid, please check and re-enter.");
      if(std::isnan(std::log(u[0]))) throw std::runtime_error("Function definition is invalid at the point or either expression is invalid, please check and re-enter.");
      return exp_derivatives(mul_derivatives(v, ln_derivatives(u, order), order), order);
    }
    
    //formula for arcsine that i derived is this recursive beast here:
    //f = asin(u):
    //f(n) = (u(n) - S{i=1,n-1}(nCr{n-1,i-1}·f(i)·S{j=1,n-i}(nCr{n-i-1,j-1}·u(n-i-j)·f(j))))/√(1-(u(0))²)
    // or:
    //f(n) = (u(n) - S{i=1,n-1}(nCr{n-1,i-1}·f(i)·S{j=0,n-i-1}(nCr{n-i-1,j}·u(j)·f(n-i-j))))/√(1-(u(0))²)
    //again we'll be using the second form for better potential performance.
    //and for n > 27(n: n³ ≥ 3n²) we'll be using another trick.
    container asin_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(u[0]) > 1) throw std::runtime_error("Arcsin is undefined at |argument| greater than one, please recheck your equation and point of evaluation.");
      container f(order + 1, 0);
      f[0] = std::asin(u[0]);
      if(order == 0) return f;
      if(std::abs(u[0])==1) throw std::runtime_error("Differentiation at point of discontinuation for arcsin is forbidden (error: |argument| = 1), please reconfigure your expression and point.");
      if(order + 1 <= 27){
        long double helper = std::sqrt(1-u[0]*u[0]);
        int k = pastNonZero(u);
        for(int n = 1; n < order + 1; n++){
          long double result = 0.0L;
          container& factor = nCr[n-1];
          for(int i = 1; i < n; i++){
            int limit = std::min(n-i,k);
            container& NCR = nCr[n-i-1];
            long double inner = 0.0L;
            for(int j = 0; j < limit; j++){
              inner += NCR[j]*u[j]*f[n-i-j];
            }
            result += factor[i-1]*f[i]*inner;
          }
          if(n < u.size()) f[n] += u[n];
          f[n] /= helper;
        }
        return f;
      }
      container uTop(u.begin()+1,u.end());
      int trim = (order+1==u.size());
      container u2(u.begin(), u.end()-trim);
      u2 = mul_derivatives(u, u, order-1);
      std::transform(u2.begin(), u2.end(), u2.begin(), [](long double p){return -p;});
      u2[0] += 1.0;
      container further_der = div_derivatives(uTop, sqrt_derivatives(u2, order - 1), order - 1);
      for(int i = 1; i < order + 1; i++) f[i] = further_der[i-1];
      return f;
    }
    
    //it's formula is identical to that of arcsin but with just a negative sign.
    //but we'll just use asin since that's more maintainable.
    //arccos = π/2 - arcsin(1/x)
    container acos_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(u[0]) > 1) throw std::runtime_error("Arccos is undefined at |argument| greater than one, please recheck your equation and point of evaluation.");
      container f(order + 1, 0);
      f[0] = std::acos(u[0]);
      if(order == 0) return f;
      if(std::abs(u[0])==1) throw std::runtime_error("Differentiation at point of discontinuation for arccos is forbidden (error: |argument| = 1), please reconfigure your expression and point.");
      f = asin_derivatives(u, order);
      std::transform(f.begin(), f.end(), f.begin(), [](long double p){return -p;});
      f[0] += PI/2.0;
      return f;
    }
    
    //arccosec(u) = arcsin(1/u)
    container acsc_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(u[0]) < 1) throw std::runtime_error("Arccosec is undefined at |argument| less than one, please recheck your equation and point of evaluation.");
      container f(order + 1, 0);
      f[0] = std::asin(1/u[0]);
      if(order == 0) return f;
      if(std::abs(u[0])==1) throw std::runtime_error("Differentiation at point of discontinuation for arccosec is forbidden (error: |argument| = 1), please reconfigure your expression and point.");
      container num = {1.0};
      container u_reciprocal = div_derivatives(num, u, order);
      return asin_derivatives(u_reciprocal, order);
    }
    
    //arcsec(u) = arccos(1/u) = π/2 - arcsin(1/x)
    container asec_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      if(std::abs(u[0]) < 1) throw std::runtime_error("Arcsec is undefined at |argument| less than one, please recheck your equation and point of evaluation.");
      container f(order + 1, 0);
      f[0] = std::acos(1/u[0]);
      if(order == 0) return f;
      if(std::abs(u[0])==1) throw std::runtime_error("Differentiation at point of discontinuation for arcsec is forbidden (error: |argument| = 1), please reconfigure your expression and point.");
      container num = {1.0};
      container u_reciprocal = div_derivatives(num, u, order);
      f = asin_derivatives(u_reciprocal, order);
      f[0] = PI/2.0 - f[0];
      for(int i = 1; i < f.size(); i++) f[i] = -f[i];
      return f;
    }
    
    //the formula that I've derived for this is:
    //f = atan(u):
    //f(n) = (u(n) - 2·S{i=1,n-1}(nCr{n-1,i}·f(n-i)·S{j=0,i-1}(ncr{i-1,j}·u(j)·u(i-j))))/(1+(u(0))²)
    //again O(n³) but this time the trick is 2n² which is graater than
    //n³ only upto n~4, so we'll be using the trick directly.
    container atan_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container f(order + 1, 0);
      f[0] = std::atan(u[0]);
      if(order == 0) return f;
      int size = u.size();
      
      //also if you want to see the formula in action, here is how it would look:
        /*
          long double helper = 1.0 + u[0]*u[0];
          for(int n = 1; n < order + 1; n++){
            container& factor = nCr[n-1];
            long double outer_sum = 0.0L;
            for(int i = 1; i < n; i++){
              container& NCR = nCr[i-1];
              long double inner_sum = 0.0L;
              for(int j = 0; j < i; j++){
                if(j < size && (i-j) < size)
                  inner_sum += NCR[j]*u[j]*u[i-j];
              }
              outer_sum += factor[i]*f[n-i];
            }
            f[n] = -2.0*outer_sum;
            if(n < size) f[n] += u[n];
            f[n] /= helper;
          }
          return f;
        */
      
      container uTop(u.begin()+1, u.end());
      container u2(u.begin(), u.end()-(size==order+1));
      u2 = mul_derivatives(u2,u2,order-1);
      u2[0] += 1.0;
      container temp = div_derivatives(uTop,u2,order-1);
      for(int i = 1; i < order + 1; i++) f[i] = temp[i-1];
      return f;
    }
    
    container acot_derivatives(const container& u, int order){
      if(u.size()==1) order = 0;
      container f(order + 1, 0.0L);
      f = atan_derivatives(u, order);
      f[0] = PI/2.0 - f[0];
      for(int i = 1; i < order + 1; i++) f[i] = -f[i];
      return f;
    }
    
    //call the appropriate functions according to the name
    std::unordered_map<std::string, std::function<container(const container&, int)>> unary_funcs;
    std::unordered_map<std::string, std::function<container(const container&, const container&, int)>> binary_funcs;
    
    //unified recursive decent
    container unified_diff(const std::string& expr, int order, long double point){
      if(expr.empty()){
        throw std::runtime_error("An unexpected error occured.");
      }
      try{
        long double number = std::stold(expr);
        container result{number};
        return result;
      }catch(const std::invalid_argument&){
        if(expr == "x"){
          container result;
          if(order > 0) result = {point, 1};
          else result = {point};
          return result;
        }
        size_t t = expr.find("(");
        if(t==std::string::npos) throw std::runtime_error("An unexpected error occured.");
        std::string function = expr.substr(0, t);
        bool c = isConstant(expr);
        order = c ? 0 : order;
        container result;
        std::string inner_expr = expr.substr(t+1, expr.length()-2-t);
        if(unary_funcs.count(function)){
          container inner_data = unified_diff(inner_expr, order, point);
          result = unary_funcs[function](inner_data, order);
        }
        else if(binary_funcs.count(function)){
          std::string str1, str2;
            auto [dispatch1, dispatch2] = split_string(inner_expr);
            str1 = std::move(dispatch1);
            str2 = std::move(dispatch2);
          container inner_data1, inner_data2;
            inner_data1 = unified_diff(str1, order, point);
            inner_data2 = unified_diff(str2, order, point);
            result = binary_funcs[function](inner_data1, inner_data2, order);
        }
        container der{result};
        return der;
      }
    }
    
    //get a valid equation from the user
    expression get_expression(){
      expression expr;
      std::cout << "Enter your expression: ";
      std::cin >> expr;
      expr.clean();
      while(!expr.is_safe()){
        std::cout << expr.result();
        std::cout << "\nPlease enter the expression again correctly: ";
        expr.clear();
        std::cin >> expr;
        expr.clean();
      }
      return expr;
    }
    
    //get a valid order from the user withing constraints
    int get_order(){
      int order;
      std::cout << "Enter the order you'd like (0-500): ";
      while(!(std::cin >> order)||(order < 0 || order > 500)){
        if(std::cin.fail()){
          std::cout << "An error occured, please enter a valid number: ";
        }
        else if(order < 0){
          std::cout << "Order can't be negative, please enter again: ";
        }
        else std::cout << "Order can't be greater than 500, please enter again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
      return order;
    }
    
    //get a valid number as the point of differentiation from user
    long double get_point(){
      long double point;
      std::cout << "Enter the point around which you'd like the derivatives: ";
      while(!(std::cin >> point)){
        std::cout << "An error occured, please enter the point again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
      return point;
    }
    
    //if provided equation is not valid, throw an error
    void validate_expr(expression& expr){
      if(!expr.is_safe()){
        expr.clean();
        if(!expr.is_safe()){
          throw std::runtime_error("Provided expression is invalid.\n" + expr.result());
        }
      }
    }
    
    //if provided order is not within constraints, throw an error
    void validate_order(const int& order){
      if(order < 0) throw std::runtime_error("Provided order can't be negative.");
      if(order > 500) throw std::runtime_error("Provided order can't be greater than 500.");
    }
    
};

}
