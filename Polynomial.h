//#pragma once

//WORK IN PROGRESS

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <string>
#include <limits>
#include <iomanip>

namespace{
  
  long double SQaM(long double x, int power){
    if(power==1)return x;
    if(power==0)return 1.0L;
    long double result = 1.0L;
    while(power > 0){
      if(power&1)result*=x;
      power>>=1;
      x*=x;
    }
    return result;
  }
  
  //for prettiness
  std::string superscript(int power){
    if(power == 0) return "";
    if(power == 1) return "x";
    std::string result = "";
    std::string super[10] = {"⁰","¹","²","³","⁴","⁵","⁶","⁷","⁸","⁹"};
    while(power > 0){
      result = super[power%10]+result;
      power/=10;
    }
    result = "x"+result;
    return result;
  }
  
  //complex struct for fft
  struct complex {
    long double real, imag;
    complex(long double r = 0, long double i = 0) : real(r), imag(i) {}
    complex operator+(const complex& other) const { return {real + other.real, imag + other.imag}; }
    complex operator-(const complex& other) const { return {real - other.real, imag - other.imag}; }
    complex operator*(const complex& other) const {
        return {real * other.real - imag * other.imag, real * other.imag + imag * other.real};
    }
    complex& operator*=(const complex& other) {
        long double r = real * other.real - imag * other.imag;
        long double i = real * other.imag + imag * other.real;
        real = r; imag = i; return *this;
    }
    complex& operator/=(long double val) { real /= val; imag /= val; return *this; }
  };
  
  //butterfly
  void fft(std::vector<complex>& a, bool invert){
    int n = a.size();
    for(int i = 1, j = 0; i < n; ++i){
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    static constexpr long double TAU = 2*M_PIl;
    for(int len = 2; len <= n; len <<= 1){
        long double ang = TAU / len * (invert ? -1 : 1);
        complex wlen(std::cos(ang), std::sin(ang));
        for(int i = 0; i < n; i += len){
            complex w(1);
            for(int j = 0; j < len / 2; j++){
                complex u = a[i + j];
                complex v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if(invert){
        for (auto& x : a) x /= n;
    }
  }
  
  //orchestra
  std::vector<long double> fft_multiply(const std::vector<long double>& a, const std::vector<long double>& b){
    std::vector<complex> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    int n = 1LL << (32 - __builtin_clz(static_cast<unsigned long int>(a.size() + b.size() - 1)));
    fa.resize(n);
    fb.resize(n);
    fft(fa, false);
    fft(fb, false);
    for (int i = 0; i < n; i++) fa[i] *= fb[i];
    fft(fa, true);
    std::vector<long double> result(n);
    for (int i = 0; i < n; i++) result[i] = fa[i].real;
    return result;
  }
  
  static constexpr long double threshold = 1E-13;
  
}

namespace poly{
  
  class dense;
  class sparse;
  
  //a full poly
  class full{
    
    //alias
    using terms = std::vector<long double>;
    
    private:
      
      terms polynomial;
      bool ascend;
      //to make the polynomial either ascending or descending.
      
    public:
      
      //default constructor
      full(bool asc = true): polynomial(1, 0.0L), ascend(asc){}
      
      //set a max_degree ans optionally set it to ascending.
      full(size_t max_degree, bool asc = true): ascend(asc){
        terms polys(max_degree + 1, 0);
        if(asc) polys[max_degree] = 1.0L;
        else polys[0] = 1.0L;
        this->polynomial = polys;
      }
      
      //set the value directly by a given vector
      full(terms provided, bool asc = true): polynomial(provided), ascend(asc){
        this->trim(this->polynomial, this->ascend);
      }
      
      //make through an braced enclosed initializer list
      full(const std::initializer_list<long double>& provided, bool asc = true): polynomial(provided), ascend(asc){
        this->trim(this->polynomial, this->ascend);
      }
      
      //copy constructor
      full(const full& other){
        this->polynomial = other.polynomial;
        this->ascend = other.ascend;
        this->trim(this->polynomial, this->ascend);
      }
      
      //degree(it's fine to use size as we are trimming the polynomial at appropriate steps)
      size_t degree() const{return this->polynomial.size() - 1;}
      
      //utility functions
      auto begin(){return this->polynomial.begin();}
      auto end(){return this->polynomial.end();}
      auto begin() const {return this->polynomial.begin();}
      auto end() const {return this->polynomial.end();}
      auto rbegin(){return this->polynomial.rbegin();}
      auto rend(){return this->polynomial.rend();}
      auto rbegin() const {return this->polynomial.rbegin();}
      auto rend() const {return this->polynomial.rend();}
      
      //function to set poly representation to ascending
      full ascending() const{
        if(this->ascend) return *this;
        full result(*this);
        std::reverse(result.begin(), result.end());
        result.ascend = true;
        return result;
      }
      
      //function to set poly representation to descending
      full descending() const{
        if(!this->ascend) return *this;
        full result(*this);
        std::reverse(result.begin(), result.end());
        result.ascend = false;
        return result;
      }
      
      //function to reverse whatever the order might be.
      full reverse() const{
        full result(*this);
        std::reverse(result.begin(), result.end());
        result.ascend = !result.ascend;
        return result;
      }
      
      //check if the poly is ascending
      bool isAscending() const{return this->ascend;}
      
      //unary plus symbol
      full operator+() const{return *this;}
      
      //unary minus symbol(negation)
      full operator-() const{
        full result(*this);
        std::transform(result.begin(), result.end(), result.begin(), std::negate<>());
        return result;
      }
      
      //in-place negation
      full negate(){
        std::transform(this->begin(), this->end(), this->begin(), std::negate<>());
        return *this;
      }
      
      //polynomial addition
      full operator+(const full& other) const{
        size_t larger = (this->degree()>other.degree())?(this->degree()):other.degree();
        terms result(larger + 1, 0.0L);
        copyInVec(result, *this);
        if(other.ascend)
          std::transform(other.begin(), other.end(), result.begin(), result.begin(), std::plus<>());
        else
          std::transform(other.rbegin(), other.rend(), result.begin(), result.begin(), std::plus<>());
        trim(result, this->ascend);
        full added_poly(result, this->ascend);
        return added_poly;
      }
      
      //polynomial subtraction
      full operator-(const full& other) const{
        size_t larger = (this->degree()>other.degree())?(this->degree()):other.degree();
        terms result(larger + 1, 0.0L);
        copyInVec(result, *this);
        if(other.ascend)
          std::transform(other.begin(), other.end(), result.begin(), result.begin(), [&](long double a, long double b){return b - a;});
        else
          std::transform(other.rbegin(), other.rend(), result.begin(), result.begin(), [&](long double a, long double b){return b - a;});
        trim(result, this->ascend);
        full subtracted_poly(result, this->ascend);
        return subtracted_poly;
      }
      
      //polynomial in-place addition
      full& operator+=(const full& other){
        terms otherPoly(other.begin(), other.end());
        if(this->ascend ^ other.ascend){
          std::reverse(otherPoly.begin(), otherPoly.end());
        }
        resize_poly(*this, otherPoly);
        std::transform(otherPoly.begin(), otherPoly.end(), this->begin(), this->begin(), std::plus<>());
        trim(this->polynomial, this->ascend);
        return *this;
      }
      
      //polynomial in-place subtraction
      full& operator-=(const full& other){
        terms otherPoly(other.begin(), other.end());
        if(this->ascend ^ other.ascend){
          std::reverse(otherPoly.begin(), otherPoly.end());
        }
        resize_poly(*this, otherPoly);
        std::transform(otherPoly.begin(), otherPoly.end(), this->begin(), this->begin(), [&](long double a, long double b){return b - a;});
        trim(this->polynomial, this->ascend);
        return *this;
      }
      
      //addition with a number
      full operator+(long double a) const{
        full result(*this);
        if(this->ascend) result.polynomial[0] += a;
        else result.polynomial.back() += a;
        return result;
      }
      
      //subtraction with a number
      full operator-(long double a) const{
        full result(*this);
        if(this->ascend) result.polynomial[0] -= a;
        else result.polynomial.back() -= a;
        return result;
      }
      
      //scalar multiplication
      full operator*(long double a) const{
        full result(*this);
        if(a == 0){
          result.polynomial = {0.0L};
          return result;
        }
        if(a == 1) return result;
        std::transform(result.begin(), result.end(), result.begin(), [&](long double p){return a*p;});
        return result;
      }
      
      //scalar subtraction
      full operator/(long double a) const{
        full result(*this);
        if(a == 0){
          throw std::runtime_error("Division by 0, undefined.");
        }
        if(a == 1) return result;
        std::transform(result.begin(), result.end(), result.begin(), [&](long double p){return p/a;});
        return result;
      }
      
      //in-place scalar addition
      full& operator+=(long double a){
        if(this->ascend) this->polynomial[0] += a;
        else this->polynomial.back() += a;
        return *this;
      }
      
      //in-place scalar subtraction
      full& operator-=(long double a){
        if(this->ascend) this->polynomial[0] -= a;
        else this->polynomial.back() -= a;
        return *this;
      }
      
      //in-place scalar multiplication
      full& operator*=(long double a){
        if(a == 0){
          this->polynomial = {0.0L};
          return *this;
        }
        if(a == 1) return *this;
        std::transform(this->begin(), this->end(), this->begin(), [&](long double p){return a*p;});
        return *this;
      }
      
      //in-place scalar division
      full& operator/=(long double a){
        if(a == 0){
          throw std::runtime_error("Division by 0, undefined.");
        }
        if(a == 1) return *this;
        std::transform(this->begin(), this->end(), this->begin(), [&](long double p){return p/a;});
        return *this;
      }
      
      //polynomial evalutaion
      long double operator()(long double x) const{
        long double result = 0.0L;
        if(x == 0) return this->ascend?this->polynomial[0]:this->polynomial.back();
        if(this->ascend) result = std::accumulate(this->rbegin(), this->rend(), 0.0L, [&](long double total, long double coef){return std::fma(x,total,coef);});
        else result = std::accumulate(this->begin(), this->end(), 0.0L, [&](long double total, long double coef){return std::fma(x,total,coef);});
        return result;
      }
      
      //vector copy
      full operator=(const terms& poly){
        this->polynomial = poly;
        this->trim(this->polynomial, this->ascend);
        return *this;
      }
      
      //initializer_list copy
      full operator=(std::initializer_list<long double> poly){
        this->polynomial = poly;
        this->trim(this->polynomial, this->ascend);
        return *this;
      }
      
      //multiplies two polynomials.
      full operator*(const full& other) const{
        terms otherPoly = other.polynomial;
        terms poly(this->degree()+otherPoly.size(), 0.0L);
        if(this->ascend ^ other.ascend) std::reverse(otherPoly.begin(), otherPoly.end());
        if((this->degree()+1)*(other.degree()+1) < 3*(this->degree()+other.degree()+2)*(32-__builtin_clz(static_cast<unsigned int>(this->degree()+other.degree()+2) + 3))){
          const terms& thisPoly = this->polynomial;
          for(int i = 0; i < thisPoly.size(); i++){
            for(int j = 0; j < otherPoly.size(); j++){
              poly[i+j] += thisPoly[i]*otherPoly[j];
            }
          }
        }
        else poly = fft_multiply(this->polynomial, otherPoly);
        full result;
        result.polynomial = poly;
        result.ascend = this->ascend;
        trim(result.polynomial, result.ascend);
        return result;
      }
      
      //in-place poly multipliplication.
      full& operator*=(const full& other){
        terms otherPoly = other.polynomial;
        terms poly(this->degree()+otherPoly.size(), 0.0L);
        if(this->ascend ^ other.ascend) std::reverse(otherPoly.begin(), otherPoly.end());
        if((this->degree()+1)*(other.degree()+1) < 3*(this->degree()+other.degree()+2)*(32-__builtin_clz(static_cast<unsigned int>(this->degree()+other.degree()+2) + 3))){
          terms& thisPoly = this->polynomial;
          for(int i = 0; i < thisPoly.size(); i++){
            for(int j = 0; j < otherPoly.size(); j++){
              poly[i+j] += thisPoly[i]*otherPoly[j];
            }
          }
        }
        else poly = fft_multiply(this->polynomial, otherPoly);
        trim(poly, this->ascend);
        this->polynomial = poly;
        return *this;
      }
      
      //shift the polynomial by 1 unit/multiply the polynomial by x
      full up(size_t t = 1) const{
        if(t==0)return *this;
        full result(*this);
        if(result.ascend){
          result.polynomial.insert(result.polynomial.begin(), t, 0.0L);
        }
        else result.polynomial.insert(result.polynomial.end(), t, 0.0L);
        return result;
      }
      
      //shift the polynomial by 1 unit/divides the polynomial by x
      full down(size_t t = 1) const{
        if(t==0)return *this;
        full result(*this);
        if(t > result.degree()){
          result.polynomial = {0.0L};
          return result;
        }
        if(result.ascend){
          result.polynomial.erase(result.begin(), result.begin() + t);
        }
        else result.polynomial.erase(result.end() - t, result.end());
        return result;
      }
      
      bool isConstant() const{return this->degree()==0;}
      bool isZero() const{return (this->degree()==0)&&this->polynomial[0] == 0;}
      long double leadingCoef() const{return this->ascend?this->polynomial.back():this->polynomial.front();}
      long double constVal() const{return this->ascend?this->polynomial.front():this->polynomial.back();}
      
      //check for leading coefficient as at extreme the one with
      //greater leading coefficient in value will outgrow the other
      bool operator>(const full& other) const{
        return this->degree()>other.degree()?true:
          (this->degree()==other.degree()&&std::abs(this->leadingCoef())
          >std::abs(other.leadingCoef()))?true:false;
      }
      bool operator<(const full& other) const{
        return this->degree()<other.degree()?true:
          (this->degree()==other.degree()&&std::abs(this->leadingCoef())
          <std::abs(other.leadingCoef()))?true:false;
      }
      
      //exact equality
      bool operator==(const full& other) const{
        if(this->degree()!=other.degree()) return false;
        bool opOrder = this->ascend^other.ascend;
        full otherP(other);
        if(opOrder) otherP.reverse();
        auto oIt = otherP.begin();
        auto it = this->begin();
        auto stop = this->end();
        for(; it != stop; it++, oIt++){
          if(std::abs((*it)-(*oIt)) > threshold) return false;
        }
        return true;
      }
      
      bool operator>=(const full& other) const{
        return (*this>other)||(*this==other);
      }
      
      bool operator<=(const full& other) const{
        return (*this<other)||(*this==other);
      }
      
      bool operator!=(const full& other) const{
        return !(*this==other);
      }
      
      //division by another polynomial
      full operator/(const full& other) const{
        //initialize with same order as the numerator(on which the operation is being done.)
        full quotient(this->ascend);
        
        //if it's smaller and or if it's zero, just five zero no need to modify anithing.
        if((this->degree() < other.degree())||this->isZero()) return quotient;
        
        //initialize dividend and divisor.
        full dividend(*this);
        full divisor(other);
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        
        //iteration counter.
        int i = 0;
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          full toadd(power_diff, this->ascend);
          
          //if this is first iteration, it will add the max degree to the quotient
          if(i==0)quotient += toadd*factor;
          //so now we can directly modify the indices, which will be way faster than adding polynomials
          else quotient.polynomial[this->ascend?power_diff:quotient.degree()-power_diff] = factor;
          
          dividend -= divisor.up(power_diff)*factor;
          trim(dividend.polynomial, dividend.ascend);
          
          //update so we don't do polynomial addition
          i=1;
        }
        
        //to correct any near zero values to zero and to trim leading or trailing zero for correct 
        //.degree() implementation.
        trim(quotient.polynomial, quotient.ascend);
        return quotient;
      }
      
      //in-place division
      full& operator/=(const full& other){
        full& quotient = *this;
        if((this->degree() < other.degree())||this->isZero()){
          quotient.polynomial = {0.0L};
          return quotient;
        }
        full dividend(*this);
        quotient.polynomial = {0.0L};
        full divisor(other);
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        int i = 0;
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          full toadd(power_diff, this->ascend);
          if(i==0)quotient += toadd*factor;
          else quotient.polynomial[this->ascend?power_diff:quotient.degree()-power_diff] = factor;
          dividend -= divisor.up(power_diff)*factor;
          trim(dividend.polynomial, dividend.ascend);
          i=1;
        }
        trim(quotient.polynomial, quotient.ascend);
        return quotient;
      }
      
      //exponentiation of polynomial
      full pow(size_t power) const{
        full result(this->ascend);
        if(power==1) return *this;
        full base(*this);
        result += 1.0L;
        while(power > 0){
          if(power&1) result *= base;
          power>>=1;
          base*=base;
        }
        return result;
      }
      
      //differentiate the polynomial
      full derivative() const{
        //copy the polynomial
        full result(*this);
        if(result.isConstant()){
          result.polynomial = {0.0L};
          return result;
        }
        //start from the highest degree(result.degree()-1 for ascendig as after removal
        //the polynomial would be of size result.degree()(from starting) and we have to start
        //at the end so we subtract 1 from it)
        int start = result.ascend?result.degree()-1:0;
        int factor = result.degree();
        //we're strictly checking != so for ascending we'll make it -1 so it stops
        //at last index 0 and for descending it stops at current result.degree() which would be
        //equal to vector's size after removal of constant term and that's out of bound for which
        //the != still helps.
        int stop = result.ascend?-1:result.degree();
        int update = result.ascend?-1:1;
        //removal of constant term
        if(result.ascend) result.polynomial.erase(result.begin(), result.begin() + 1);
        else result.polynomial.pop_back();
        //set alias and multiply by degree of corresponding term
        terms& poly = result.polynomial;
        for(int i = start; i!=stop; i+=update){
          poly[i] *= factor;
          factor--;
        }
        return result;
      }
      
      full nth_derivative(size_t n) const{
        if(n==0) return *this;
        if(this->degree() < n){
          full result(this->ascend);
          return result;
        }
        int prior_deg = this->degree();
        full result(*this);
        if(result.ascend) result.polynomial.erase(result.begin(), result.begin() + n);
        else result.polynomial.erase(result.end() - n, result.end());
        int start = result.ascend?prior_deg-n:0;
        int stop = result.ascend?-1:prior_deg+1-n;
        int update = result.ascend?-1:1;
        long double factor = std::tgamma(prior_deg+1)/std::tgamma(prior_deg-n+1);
        int factor_update = prior_deg;
        long double Ln = n;
        terms& poly = result.polynomial;
        for(int i = start; i != stop; i += update){
          poly[i] *= factor;
          factor *= 1 - (Ln/factor_update);
          factor_update--;
        }
        trim(result.polynomial, result.ascend);
        return result;
      }
      
      //will not be adding nth_integration due to constant ambiguity
      full integrate(long double constant = 0.0L) const{
        full result(*this);
        if(result.isZero()){
          result.polynomial = {constant};
          return result;
        }
        if(result.ascend) result.polynomial.insert(result.begin(), 1, constant);
        else result.polynomial.push_back(constant);
        terms& poly = result.polynomial;
        int start = result.ascend?1:poly.size()-2;
        int stop = result.ascend?poly.size():-1;
        int update = result.ascend?1:-1;
        long long division = 1;
        for(int i = start; i != stop; i += update){
          poly[i] /= division;
          division++;
        }
        trim(poly, result.ascend);
        return result;
      }
      
      //compose a second polynomial into running polynomial.
      full compose(const full& other) const{
        if(this->isConstant()) return *this;
        if(other.isZero()){
          full result(this->ascend);
          result.polynomial = {this->constVal()};
          return result;
        }
        if(other.isConstant()){
          full result(this->ascend);
          const full& alias = *this;
          long double val = alias(other.constVal());
          result.polynomial = {val};
          return result;
        }
        if(other.degree()==1&&other.constVal()==0&&other.polynomial[other.ascend?1:0]==1) return *this;
        full base({1.0L},other.ascend);
        full result({this->constVal()},this->ascend);
        int start = result.ascend?1:this->degree()-1;
        int stop = result.ascend?this->degree()+1:-1;
        int update = result.ascend?1:-1;
        const terms& poly = this->polynomial;
        for(int i = start; i != stop; i += update){
          int count = 1;
          //find next non-zero coefficient(for optimization as exponentiation would be faster
          //repeated multiplication as it uses binary exponentiation.)
          while(i!=stop&&poly[i]==0){
            i+=update;
            count++;
          }
          if(i==stop) break;
          base *= other.pow(count);
          if(poly[i]!=1) result += base*poly[i];
          else result += base;
        }
        trim(result.polynomial, result.ascend);
        return result;
      }
      
      long double definite_integration(long double a, long double b) const{
        full integrated_poly = this->integrate();
        return integrated_poly(b) - integrated_poly(a);
      }
      
      full trunc(size_t degree) const{
        if(this->degree()<=degree) return *this;
        full result(*this);
        if(this->ascend) result.polynomial.erase(result.begin() + degree + 1, result.end());
        else result.polynomial.erase(result.begin(), result.end()-1-degree);
        //as middle terms can be zero and we have to take care of those.
        trim(result.polynomial, result.ascend);
        return result;
      }
      
      //directly use divisin like structure for no redundant calculations previously used
      //such as (*this).trunc(other.degree())-((*this/other).trunc(other.degree())*other).trunc(other.degree())
      //which was used before(which did *this/other which was then further processed(redundant, no need.))
      full operator%(const full& other) const{
        if((this->degree() < other.degree())||this->isZero()) return *this;
        full dividend(*this);
        if(other.isZero()) throw std::runtime_error("Modulo 0, undefined.");
        int otherDeg = other.degree();
        long double leadingcoef = other.leadingCoef();
        while(!(dividend < other)){
          int power_diff = dividend.degree() - otherDeg;
          long double factor = dividend.leadingCoef()/leadingcoef;
          dividend -= other.up(power_diff)*factor;
          trim(dividend.polynomial, dividend.ascend);
        }
        //trimming is done every iteration, so no need.
        return dividend;
      }
      
      full& operator%=(const full& other){
        if((this->degree() < other.degree())||this->isZero()) return *this;
        full& dividend = *this;
        if(other.isZero()) throw std::runtime_error("Modulo 0, undefined.");
        int otherDeg = other.degree();
        long double leadingcoef = other.leadingCoef();
        while(!(dividend < other)){
          int power_diff = dividend.degree() - otherDeg;
          long double factor = dividend.leadingCoef()/leadingcoef;
          dividend -= other.up(power_diff)*factor;
          trim(dividend.polynomial, dividend.ascend);
        }
        //trimming is done every iteration, so no need.
        return dividend;
      }
      
    private:
      
      //trim leading(if descending) and trailing(if ascending) 0s(or near 0 values)
      void trim(terms& poly, bool asc) const{
        if(!asc){
          int j = 0;
          while(j < poly.size() && std::abs(poly[j]) < threshold) j++;
          poly.erase(poly.begin(), poly.begin() + j);
        }
        else while(!poly.empty()&&std::abs(poly.back()) < threshold) poly.pop_back();
        for(int i = 0; i < poly.size(); i++){
          if(std::abs(poly[i]-std::round(poly[i])) < threshold) poly[i] = std::round(poly[i]);
        }
        if(poly.empty()) poly = {0.0L};
      }
      
      //copy in vector(helper function *DRY)
      void copyInVec(terms& vec, const full& toCopy) const{
        if(toCopy.ascend)
          std::copy(toCopy.begin(), toCopy.end(), vec.begin());
        else
          std::copy(toCopy.rbegin(), toCopy.rend(), vec.begin());
      }
      
      //resize polynomial for appropirate addition and subtraction
      void resize_poly(full& first, terms& otherPoly){
        if(first.degree() > otherPoly.size()-1){
          size_t difference = first.degree() - otherPoly.size() + 1;
          if(!first.ascend){
            otherPoly.insert(otherPoly.begin(), difference, 0.0L);
          }
          else otherPoly.insert(otherPoly.end(), difference, 0.0L);
        }
        else{
          size_t difference = otherPoly.size() - 1 - first.degree();
          if(first.ascend){
            first.polynomial.insert(first.polynomial.end(), difference, 0.0L);
          }
          else first.polynomial.insert(first.polynomial.begin(), difference, 0.0L);
        }
      }
    
    //friend functions
    public:
      
      //output stream operator(pretty printing)
      friend std::ostream& operator<<(std::ostream& os, const full& polynomial){
        const terms& poly = polynomial.polynomial;
        int super = polynomial.ascend ? 0 : poly.size() - 1;
        int update = polynomial.ascend ? 1 : -1;
        bool converted = false;
        for(int i = 0; i < poly.size(); i++){
          if(poly[i]==0){
            super += update;
            continue;
          }
          else if(poly[i] > 0) os << (!converted?"":" + ");
          else os << (!converted?"-":" - ");
          if(std::abs(poly[i] - std::round(poly[i])) == 0){
            if(std::abs(std::round(poly[i])) == 1){
              if(polynomial.ascend){
                if(i == 0) os << 1;
              }
              else if(i == poly.size()-1) os << 1;
            }
            else os << (long long)std::abs(poly[i]);
          }
          else if(std::abs(poly[i])>1e6) os << '(' << std::abs(poly[i]) << ')';
          else if(std::abs(poly[i])<1e-6) os << '(' << std::abs(poly[i]) << ')';
          else os << std::abs(poly[i]);
          os << superscript(super);
          super += update;
          converted = true;
        }
        if(!converted) os << 0;
        return os;
      }
      
      friend full operator+(long double a, const full& poly){ return poly+a; }
      friend full operator-(long double a, const full& poly){ return -poly+a;}
      friend full operator*(long double a, const full& poly){ return poly*a; }
      
      operator dense() const;
      operator sparse() const;
      
      friend class dense;
      friend class sparse;
               
  };
    
  class dense{
    
    //alias
    using terms = std::vector<long double>;
    
    private:
      
      //data members
      terms polynomial;
      bool ascend;
      size_t min_deg;
    
    public:
      
      //constructors
      dense(bool asc = true): polynomial(1,0.0L), min_deg(0), ascend(asc){}
      
      dense(size_t max_degree, bool asc = true){
        this->min_deg = max_degree;
        this->polynomial = {1.0L};
        this->ascend = asc;
      }
      
      dense(const terms& provided, size_t min = 0, bool asc = true): polynomial(provided), min_deg(min), ascend(asc){
        this->clean();
      }
      
      dense(const std::initializer_list<long double>& provided, size_t min = 0, bool asc = true): polynomial(provided), min_deg(min), ascend(asc){
        this->clean();
      } 
      
      dense(const dense& other){
        this->polynomial = other.polynomial;
        this->ascend = other.ascend;
        this->min_deg = other.min_deg;
      }
      
      //utility functions
      size_t degree() const{return this->polynomial.size()-1+this->min_deg;}
      
      auto begin(){return this->polynomial.begin();}
      auto end(){return this->polynomial.end();}
      auto begin() const {return this->polynomial.begin();}
      auto end() const {return this->polynomial.end();}
      auto rbegin(){return this->polynomial.rbegin();}
      auto rend(){return this->polynomial.rend();}
      auto rbegin() const {return this->polynomial.rbegin();}
      auto rend() const {return this->polynomial.rend();}
      
      dense ascending() const{
        if(this->ascend) return *this;
        dense result(*this);
        result.ascend = true;
        std::reverse(result.polynomial.begin(), result.polynomial.end());
        return result;
      }
      
      dense descending() const{
        if(!this->ascend) return *this;
        dense result(*this);
        result.ascend = false;
        std::reverse(result.polynomial.begin(), result.polynomial.end());
        return result;
      }
      
      dense reverse() const{
        dense result(*this);
        std::reverse(result.begin(), result.end());
        result.ascend = !result.ascend;
        return result;
      }
      
      bool isAscending() const{return this->ascend;}
      
      //operators
      dense operator+() const{return *this;}
      dense operator-() const{
        dense result(*this);
        std::transform(result.begin(), result.end(), result.begin(), [](long double p){return -p;});
        return result;
      }
      
      //in-place negation
      dense& negate(){
        std::transform(this->begin(), this->end(), this->begin(), [](long double p){return -p;});
        return *this;
      }
      
      //checks and handles
      bool isZero() const{return this->min_deg==0&&(this->ascend?this->polynomial.front()==0:this->polynomial.back()==0);}
      bool isConstant() const{return this->degree()==0;}
      long double constVal() const{return this->min_deg==0?this->ascend?this->polynomial.front():this->polynomial.back():0;}
      long double leadingCoef() const{return this->ascend?this->polynomial.back():this->polynomial.front();}
      
      //addition
      dense operator+(const dense& other) const{
        if(other.isZero()) return *this;
        if(this->isZero()) return other;
        dense result(*this);
        addSub(result, other, false);
        return result;
      }
      
      //addition with a number
      dense operator+(long double val) const{
        if(val==0) return *this;
        dense other({val},0,true);
        dense result(*this);
        addSub(result, other, false);
        return result;
      }
      
      //subtraction
      dense operator-(const dense& other) const{
        if(other.isZero()) return *this;
        if(this->isZero()) return -other;
        dense result(*this);
        addSub(result, other, true);
        return result;
      }
      
      //subtraction by a number
      dense operator-(long double val) const{
        if(val==0) return *this;
        dense other({val},0,true);
        dense result(*this);
        addSub(result, other, true);
        return result;
      }
      
      //in-place addition
      dense& operator+=(const dense& other){
        if(other.isZero()) return *this;
        dense& result = *this;
        addSub(result, other, false);
        return result;
      }
      
      //in-place addition with a number
      dense& operator+=(long double val){
        if(val==0) return *this;
        dense other({val},0,true);
        dense result(*this);
        //can't directl extract a index as min_deg can also not be 0.
        addSub(result, other, false);
        return result;
      }
      
      //in-place subtraction
      dense& operator-=(const dense& other){
        if(other.isZero()) return *this;
        dense& result = *this;
        addSub(result, other, true);
        return result;
      }
      
      //in-place subtraction by a number
      dense& operator-=(long double val){
        if(val==0) return *this;
        dense other({val},0,true);
        dense result(*this);
        addSub(result, other, true);
        return result;
      }
      
      //scalar multiplication
      dense operator*(long double val) const{
        if(val==0){
          dense result(this->ascend);
          return result;
        }
        if(val == 1) return *this;
        dense result(*this);
        std::transform(result.begin(), result.end(), result.begin(), [a = val](long double p){return a*p;});
        result.clean();
        return result;
      }
      
      //in-place scalar multiplication
      dense& operator*=(long double val){
        if(val==0){
          this->polynomial = {0.0L};
          this->min_deg = 0;
          return *this;
        }
        if(val == 1) return *this;
        dense& result = *this;
        std::transform(result.begin(), result.end(), result.begin(), [a = val](long double p){return a*p;});
        result.clean();
        return result;
      }
      
      //polynomial multiplication
      dense operator*(const dense& other) const{
        if(this->isZero()||other.isZero()){
          dense result(this->ascend);
          return result;
        }
        if(other.isConstant()) return *this*other.constVal();
        if(this->isConstant()){
          dense result(other);
          if(this->ascend^other.ascend) result = result.reverse();
          return result*(this->constVal());
        }
        dense result(this->ascend);
        result.min_deg = this->min_deg+other.min_deg;
        const terms& thisPoly = this->polynomial;
        terms otherPoly = other.polynomial;
        if(this->ascend^other.ascend) std::reverse(otherPoly.begin(), otherPoly.end());
        unsigned int m = thisPoly.size(), n = otherPoly.size();
        //choose faster condition
        if(m*n < 3*(m+n)*(32-__builtin_clz(m+n)+3)){
          terms& poly = result.polynomial;
          poly.resize(m+n-1,0.0L);
          for(int i = 0; i < m; i++){
            for(int j = 0; j < n; j++){
              poly[i+j]+=thisPoly[i]*otherPoly[j];
            }
          }
        }
        else result.polynomial = fft_multiply(thisPoly, otherPoly);
        result.clean();
        return result;
      }
      
      //in-place poly multiplication
      dense& operator*=(const dense& other){
        if(this->isZero()||other.isZero()){
          this->polynomial.clear();
          this->min_deg = 0;
          return *this;
        }
        if(other.isConstant()){
          *this *= other.constVal();
          return *this;
        }
        if(this->isConstant()){
          dense result(other);
          if(this->ascend^other.ascend) result = result.reverse();
          *this = result*(this->constVal());
          return *this;
        }
        dense result(this->ascend);
        result.min_deg = this->min_deg+other.min_deg;
        const terms& thisPoly = this->polynomial;
        terms otherPoly = other.polynomial;
        if(this->ascend^other.ascend) std::reverse(otherPoly.begin(), otherPoly.end());
        unsigned int m = thisPoly.size(), n = otherPoly.size();
        if(m*n < 3*(m+n)*(32-__builtin_clz(m+n)+3)){
          terms& poly = result.polynomial;
          poly.resize(m+n-1,0.0L);
          for(int i = 0; i < m; i++){
            for(int j = 0; j < n; j++){
              poly[i+j]+=thisPoly[i]*otherPoly[j];
            }
          }
        }
        else result.polynomial = fft_multiply(thisPoly, otherPoly);
        result.clean();
        *this = result;
        return *this;
      }
      
      //multiplication by x^t
      dense up(size_t t = 1) const{
        dense result(*this);
        result.min_deg += t;
        return result;
      }
      
      //division by x^t, not exactly division but shifting down
      dense down(size_t t = 1) const{
        if(t > this->degree()){
          dense result(this->ascend);
          return result;
        }
        if(this->min_deg >= t){
          dense result(*this);
          result.min_deg -= t;
          return result;
        }
        dense result(*this);
        t -= result.min_deg;
        result.min_deg = 0;
        if(result.ascend) result.polynomial.erase(result.begin(), result.begin() + t);
        else result.polynomial.erase(result.end()-t, result.end());
        result.clean();
        return result;
      }
      
      //boolean operators
      bool operator>(const dense& other) const{
        return this->degree()>other.degree()?true:
          (this->degree()==other.degree()&&std::abs(this->leadingCoef())
          >std::abs(other.leadingCoef()))?true:false;
      }
      
      bool operator<(const dense& other) const{
        return this->degree()<other.degree()?true:
          (this->degree()==other.degree()&&std::abs(this->leadingCoef())
          <std::abs(other.leadingCoef()))?true:false;
      }
      
      bool operator==(const dense& other) const{
        if(this->degree()!=other.degree()||this->min_deg!=other.min_deg) return false;
        bool opOrder = this->ascend^other.ascend;
        dense otherP(other);
        if(opOrder) otherP.reverse();
        auto oIt = otherP.begin();
        auto it = this->begin();
        auto stop = this->end();
        for(; it != stop; it++, oIt++){
          if(std::abs((*it)-(*oIt)) > threshold) return false;
        }
        return true;
      }
      
      bool operator>=(const dense& other) const{
        return *this>other||*this==other;
      }
      
      bool operator<=(const dense& other) const{
        return *this<other||*this==other;
      }
      
      bool operator!=(const dense& other) const{
        return !(*this==other);
      }
      
      //scalar division
      dense operator/(long double val) const{
        if(this->isZero()) return *this;
        if(val == 0) throw std::runtime_error("Division by 0, undefined.");
        dense result(*this);
        std::transform(result.begin(), result.end(), result.begin(), [a=val](long double p){return p/a;});
        result.clean();
        return result;
      }
      
      //in-place scalar division
      dense& operator/=(long double val){
        if(this->isZero()) return *this;
        if(val == 0) throw std::runtime_error("Division by 0, undefined.");
        dense& result = *this;
        std::transform(result.begin(), result.end(), result.begin(), [a=val](long double p){return p/a;});
        result.clean();
        return result;
      }
      
      //poly division
      dense operator/(const dense& other) const{
        dense quotient(this->ascend);
        if((this->degree() < other.degree())||this->isZero()) return quotient;
        quotient.polynomial.resize(this->degree()-other.degree()+1,0.0L);
        dense dividend(*this);
        dense divisor(other);
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        terms& poly = quotient.polynomial;
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          poly[this->ascend?power_diff:poly.size()-1-power_diff] = factor;
          dividend -= divisor.up(power_diff)*factor;
          dividend.clean();
        }
        quotient.clean();
        return quotient;
      }
      
      //in-place poly division
      dense& operator/=(const dense& other){
        if((this->degree() < other.degree())||this->isZero()){
          this->polynomial = {0.0L};
          this->min_deg = 0;
          return *this;
        }
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        dense dividend(*this);
        dense& quotient = *this;
        terms& poly = quotient.polynomial;
        poly.clear();
        quotient.min_deg = 0;
        poly.resize(dividend.degree()-other.degree()+1,0.0L);
        dense divisor(other);
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          poly[this->ascend?power_diff:poly.size()-1-power_diff] = factor;
          dividend -= divisor.up(power_diff)*factor;
          dividend.clean();
        }
        quotient.clean();
        return quotient;
      }
      
      //poly remainder
      dense operator%(const dense& other) const{
        if((this->degree() < other.degree())||this->isZero()) return *this;
        dense dividend(*this);
        dense divisor(other);
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          dividend -= divisor.up(power_diff)*factor;
          dividend.clean();
        }
        return dividend;
      }
      
      //in-place remainder
      dense& operator%=(const dense& other){
        if((this->degree() < other.degree())||this->isZero()) return *this;
        dense& dividend = *this;
        dense divisor(other);
        if(other.isZero()) throw std::runtime_error("Division by 0, undefined.");
        while(!(dividend < divisor)){
          int power_diff = dividend.degree() - divisor.degree();
          long double factor = dividend.leadingCoef()/divisor.leadingCoef();
          dividend -= divisor.up(power_diff)*factor;
          dividend.clean();
        }
        return dividend;
      }
      
      //truncation
      dense trunc(size_t degree) const{
        if(this->degree()<=degree) return *this;
        if(degree < this->min_deg){
          dense result(this->ascend);
          return result;
        }
        dense result(*this);
        if(this->ascend) result.polynomial.erase(result.begin()+1-result.min_deg+degree, result.end());
        else result.polynomial.erase(result.begin(), result.begin()+result.polynomial.size()-degree+result.min_deg-1);
        result.clean();
        return result;
      }
      
      //binary exponentiation
      dense pow(size_t power) const{
        if(power == 0){
          dense result({1.0L},0,this->ascend);
          return result;
        }
        if(this->isZero()||power==1) return *this;
        dense result({1.0L},0,this->ascend);
        dense base(*this);
        while(power > 0){
          if(power&1) result*=base;
          power>>=1;
          base*=base;
        }
        //no need to call clean as it is already called in multiplication.
        return result;
      }
      
      //differentiation
      dense derivative() const{
        if(this->isConstant()){
          dense result(this->ascend);
          return result;
        }
        dense result(*this);
        terms& poly = result.polynomial;
        int i = 0;
        int min = result.min_deg;
        if(result.ascend) std::transform(poly.begin(), poly.end(), poly.begin(), [&](long double p){
          i++;
          return p*((i-1)+min);
        });
        else std::transform(poly.rbegin(), poly.rend(), poly.rbegin(), [&](long double p){
          i++;
          return p*((i-1)+min);
        });
        if(min==0){
          if(result.ascend) poly.erase(poly.begin(), poly.begin()+1);
          else poly.pop_back();
        }
        else result.min_deg--;
        result.clean();
        return result;
      }
      
      //first integral result
      dense integrate(long double constant = 0) const{
        if(this->isZero()){
          dense result({constant},0,this->ascend);
          return result;
        }
        dense result(*this);
        result = result.up(1);
        result+=constant;
        //because we uppped it by 1
        int min = result.min_deg - 1;
        int i = 1;
        //if the constant was zero the minimum degree wouldn't be 0
        //so we can check if we should skip it.
        int j = min == 0;
        if(result.ascend) std::transform(result.begin() + j, result.end(), result.begin() + j, [&](long double p){
          p/=(i+min);
          i++;
          return p;
        });
        else std::transform(result.rbegin() + j, result.rend(), result.rbegin() + j, [&](long double p){
          p/=(i+min);
          i++;
          return p;
        });
        result.clean();
        return result;
      }
      
      dense nth_derivative(size_t order) const{
        if(order == 0) return *this;
        dense result(*this);
        result = result.down(order);
        if(result.isZero()) return result;
        int deg = this->degree();
        terms& poly = result.polynomial;
        long double factor = std::tgamma(deg+1)/std::tgamma(deg-order+1);
        long double factor_update = deg;
        long double Lo = order;
        if(result.ascend) std::transform(poly.rbegin(), poly.rend(), poly.rbegin(), [&](long double p){
          p*=factor;
          factor *= 1.0 - Lo/factor_update;
          factor_update--;
          return p;
        });
        else std::transform(poly.begin(), poly.end(), poly.begin(), [&](long double p){
          p*=factor;
          factor *= 1.0 - Lo/factor_update;
          factor_update--;
          return p;
        });
        result.clean();
        return result;
      }
      
      long double operator()(long double val) const{
        if(val == 0) return this->constVal();
        long double result = 0.0L;
        if(!this->ascend) result = std::accumulate(this->begin(), this->end(), 0.0L, [x = val](long double total, long double p){return std::fma(x,total,p);});
        else result = std::accumulate(this->rbegin(), this->rend(), 0.0L, [x = val](long double total, long double p){return std::fma(x,total,p);});
        result *= SQaM(val, this->min_deg);
        return result;
      }
      
      //compose a second polynomial into running polynomial.
      dense compose(const dense& other) const{
        if(this->isConstant()) return *this;
        if(other.isZero()){
          dense result(this->ascend);
          result.polynomial = {this->constVal()};
          return result;
        }
        if(other.isConstant()){
          dense result(this->ascend);
          const dense& alias = *this;
          long double val = alias(other.constVal());
          result.polynomial = {val};
          return result;
        }
        if(other.degree()==1&&other.constVal()==0&&other.polynomial.front()==1) return *this;
        dense base({1.0L},0,other.ascend);
        dense result({this->constVal()},0,this->ascend);
        int start = result.ascend?1:this->degree()-1;
        int stop = result.ascend?this->degree()+1:-1;
        int update = result.ascend?1:-1;
        const terms& poly = this->polynomial;
        for(int i = start; i != stop; i += update){
          int count = 1;
          while(i!=stop&&poly[i]==0){
            i+=update;
            count++;
          }
          if(i==stop) break;
          base *= other.pow(count);
          if(poly[i]!=1) result += base*poly[i];
          else result += base;
        }
        result *= other.pow(min_deg);
        //cleaned implicitely in all the processes.
        return result;
      }
      
    private:
      
      void addSub(dense& first, const dense& second, bool sub) const{
        terms& poly = first.polynomial;
        bool oppOrder = first.ascend ^ second.ascend;
        size_t prior_min_deg = first.min_deg;
        size_t prior_deg = first.degree();
        first.min_deg = std::min(prior_min_deg, second.min_deg);
        terms otherPoly = second.polynomial;
        if(oppOrder) std::reverse(otherPoly.begin(), otherPoly.end());
        poly.insert(poly.begin(), prior_min_deg-first.min_deg, 0.0L);
        poly.insert(poly.end(), std::max(prior_deg, second.degree())-prior_deg, 0.0L);
        int shift = first.ascend?(second.min_deg-first.min_deg):(first.degree()-second.degree());
        for(int i = 0; i < otherPoly.size(); i++){
          if(sub) poly[i+shift] -= otherPoly[i];
          else poly[i+shift] += otherPoly[i];
        }
        first.clean();
      }
      
      void clean(){
        terms& poly = this->polynomial;
        bool asc = this->ascend;
        size_t& min = this->min_deg;
        int j = 0;
        while(j < poly.size() && std::abs(poly[j]) < threshold) j++;
        poly.erase(poly.begin(), poly.begin() + j);
        if(asc) min += j;
        if(poly.empty()){
          min = 0;
          poly = {0.0L};
          return;
        }
        int size = poly.size()-1;
        j = size;
        while(j >= 0 && std::abs(poly[j]) < threshold) j--;
        if(j < 0){
          min = 0;
          poly = {0.0L};
          return;
        }
        poly.erase(poly.begin()+j+1, poly.end());
        if(!asc) min += size - j;
        if(poly.empty()){
          min = 0;
          poly = {0.0L};
          return;
        }
        for(int i = 0; i < poly.size(); i++)
          if(std::abs(poly[i]-std::round(poly[i])) < threshold) poly[i] = std::round(poly[i]);
        return;
      }
      
    public:
        
      //output stream operator(pretty printing)
      friend std::ostream& operator<<(std::ostream& os, const dense& polynomial){
        const terms& poly = polynomial.polynomial;
        int super = polynomial.ascend ? 0 : poly.size() - 1;
        int update = polynomial.ascend ? 1 : -1;
        bool converted = false;
        int min = polynomial.min_deg;
        for(int i = 0; i < poly.size(); i++){
          if(poly[i]==0){
            super += update;
            continue;
          }
          else if(poly[i] > 0) os << (!converted?"":" + ");
          else os << (!converted?"-":" - ");
          if(std::abs(poly[i] - std::round(poly[i])) == 0){
            if(std::abs(std::round(poly[i])) == 1){
              if(polynomial.ascend){
                if(i == 0 && min == 0) os << 1;
              }
              else if(i == poly.size()-1 && min == 0) os << 1;
            }
            else os << (long long)std::abs(poly[i]);
          }
          else if(std::abs(poly[i])>1e6) os << '(' << std::abs(poly[i]) << ')';
          else if(std::abs(poly[i])<1e-6) os << '(' << std::abs(poly[i]) << ')';
          else os << std::abs(poly[i]);
          os << superscript(super+min);
          super += update;
          converted = true;
        }
        if(!converted) os << 0;
        return os;
      }
      
      friend dense operator+(long double a, const dense& poly){ return poly+a; }
      friend dense operator-(long double a, const dense& poly){ return -poly+a;}
      friend dense operator*(long double a, const dense& poly){ return poly*a; }
      
      operator full() const;
      operator sparse() const;
      
      friend class full;
      friend class sparse;
      
  };
  
  class sparse{
    
    public:
      
      struct term{
        long double coef;
        int power;
      };
      
    private:
      
      std::vector<term> terms;
      bool ascend;
      
  };
  
}

int main(){
  poly::dense poly1({1.0L, 1.0L, 1.0L},4,true);
  poly::dense poly2({1.0L, 1.0L},5,false);
  poly::dense poly3(1,true);

  
  std::cout << poly1 << std::endl;
  std::cout << poly1.derivative() << std::endl;
  std::cout << poly1.derivative().derivative() << std::endl;
  std::cout << poly1.integrate() << std::endl;
  std::cout << poly1.integrate().derivative() << std::endl;
  std::cout << poly1.nth_derivative(2);
}