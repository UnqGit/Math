#pragma once

#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <stack>
#include <utility>
#include <cctype>

class expression{
  
  private:
    std::string entered_expression;
    std::string modified_expression;
    bool anamoly = true;
    bool allow_variable = true;
    std::string err_log = "NOT MODIFIED.";

    //checks if a string of lowercase letters more than a letter
    //long is followed by a '(' or not (to check if functions are followed
    //by a '(')
    void app_func_call(const std::string& input, bool flag) noexcept{
      int to_check = 0;
      for(int i = 0; i < input.size(); i++){
        if(std::islower(input[i]))to_check++;
        else{
          if(to_check==0) continue;
          if(to_check > 1){
            if(input[i]!='('){
              if(flag) this->err_log = "NO ARGUMENT FOR FUNCTION.";
              else this->err_log = "FUNCTION NOT FOLLOWED BY '('.";
              this->anamoly = true;
              return;
            }
          }
          to_check=0;
        }
      }
      if(to_check>1){
        if(flag) this->err_log = "NO ARGUMENT FOR FUNCTION.";
        else this->err_log = "FUNCTION NOT FOLLOWED BY '('.";
        this->anamoly = true;
        return;
      }
    }
    
    //checks if continuous set of strings in the modified string is
    //an intended function via a map to check easily
    void app_func_name(const std::string& input) noexcept{
      
      //store all the intended function names in a map for
      //fast lookup
      std::unordered_set<std::string> map{
        {"x"}, {"exp"}, {"ln"},
        {"sqrt"}, {"sin"}, {"cos"},
        {"csc"}, {"sec"}, {"tan"},
        {"cot"}, {"asin"}, {"acos"},
        {"acsc"}, {"asec"}, {"atan"},
        {"acot"}, {"sinh"}, {"cosh"},
        {"csch"}, {"sech"}, {"tanh"},
        {"coth"}
      };
      
      std::string to_check;
      
      for(int i = 0; i < input.size(); i++){
        
        //store continuous set of lowercase letters into
        //a temporary string
        if(std::islower(input[i])){
          to_check+=input[i];
        }
        
        else{
          
          //if the prevous letter also wasn't an alphabet
          if(to_check.empty()) continue;
          
          //if the string stored as potential functions aren't
          //present in the map, flag them otherwise reset and continue
          else{
            if(map.count(to_check)){
              if(to_check=="x"&&(!this->allow_variable)){
                this->err_log = "VARIABLE IN NON-FUNCTION EXPRESSION.";
                this->anamoly = true;
                return;
              }
              to_check = "";
              continue;
            }
            else{
              this->err_log = "INVALID STRING OF LETTERS.";
              this->anamoly = true;
              return;
            }
          }
          
        }
      }
      
      //if the temporary variable isn't empty then check it as a
      //necessary function or variable, if not, then flag
      if(!to_check.empty()&&!map.count(to_check)){
        this->err_log = "INVALID STRING OF LETTERS.";
        this->anamoly = true;
        return;
      }
    }
    
    //checks if there are correct numbers of opening and closing
    //parenthesis, if not add where needed
    void app_paren(std::string& input, bool& flag){
      int parenthesis = 0;
      for(int i = 0; i < input.size(); i++){
        if(input[i]=='('){
          parenthesis++;
        }
        else if(input[i]==')'){
          if(parenthesis==0){
            input = "("+input;
            flag = true;
            i++;
          }
          else parenthesis--;
        }  
      }
      input += std::string(parenthesis, ')');
    }

    //called at last of the cleaner function to ensure this works properly
    void paren_cleaner(std::string& input) noexcept{
      //Parenthesis count check is done before calling
      //this function.
      //Initialize a stack to keep track of opening
      //parenthesis index and potential for removal 
  
      std::stack<std::pair<int,bool>> holder;
  
      //only need to iterate through the string once
  
      for(int i = 0; i < input.length(); i++){
    
        if(input[i]=='('){
      
          //if the opening parenthesis isn't preceded by
          //a function, add to the potential
      
          if(i>0){
            char ch = input[i-1];
            if(!std::islower(ch)){
              holder.push({i,1});
            }
            else holder.push({i,0});
          }
       
          //if the parenthesis is at the start, still
          //add potential for removal
      
          else holder.push({i,1});
        }
    
        else if(input[i]==')'){
          
          //error check
          if(holder.empty()||(!holder.empty()&&(holder.top().first==i-1))){
            this->err_log = "INVALID PARENTHESIS ORDER/USE.";
            this->anamoly = true;
            return;
          }
          
          if(holder.top().second){
            
            //if it had potential
            //remove the parenthesis 
        
            input.erase(i,1);
            input.erase(holder.top().first,1);
            i-=2;
          }
      
          //remove the top-most nest from the stack
      
          holder.pop();
    
        }
      }
    }

    void basic_cleanup(std::string& input){
      //convert unicode operators and constants into
      //normal operators and placeholders respectfully
      //reason why exp and sec are chaned temporarily is
      //so that exp doesn't become 2.718*xp
      //and pi should be converted before exp for the cases
      //such as expi where it should be 2.718*x*3.141
      std::string to_change1[22] = {
        "·", "*", "×", "*", "÷", "/", "√", "#",
        "pi", "=", "π", "=", "exp", "EXP", "sec", "SEC",
        "e", "@", "EXP", "exp", "SEC", "sec"
      };
      for(int i = 0; i < 22; i+=2){
        size_t t = input.find(to_change1[i]);
        while(t!=std::string::npos){
          input.replace(t, to_change1[i].length(), to_change1[i+1]);
          t = input.find(to_change1[i], t+to_change1[i].length()-1-(to_change1[i]=="√"?1:0));
          //idk why but the extra check for √ symbol separately
          //was necessary for it to work...
          ///it was due to multi byte nature of √
        }
      }
  
      std::string to_change2[24]={
        "⁰", "0", "¹", "1", "²", "2", "³", "3", "⁴", "4",
        "⁵", "5", "⁶", "6", "⁷", "7", "⁸", "8", "⁹", "9",
        "=", "3.14159265358979323846264338327950288419",
        "@", "2.71828182845904523536028747135266249775"
      };
      std::unordered_set<char> lookup{
        '0','1','2','3','4','5','6','7','8','9',
        '.',')','=','@'
      };
  
      //add approapriate operators where needed
      for(int i = 0; i < 24; i+=2){
        size_t t = input.find(to_change2[i]);
        while(t != std::string::npos){
          std::string substring = to_change2[i];
          int update=0;
          if(t > 0 && (lookup.count(input[t-1])||std::islower(input[t-1])))
            substring = (i<20?"^":"*")+substring;
      
          //we can't check the immediate next character as
          //superscripts are unicode and not single byte ASCII
          if((t+to_change2[i].length()<input.size())&&((std::islower(input[t+to_change2[i].length()]))||
          (lookup.count(input[t+to_change2[i].length()])&&input[t+to_change2[i].length()]!=')'))){
            substring+="*";
            update++;
          }
          input.replace(t,to_change2[i].size(),substring);
          t = input.find(to_change2[i],t+substring.length()-update);
        }
      }
  
      //reason why this couldn't be done in the first loop itself
      //is that something like 2³⁴ would become 2^3^4
      for(int i = 0; i < 24; i+=2){
        size_t t = input.find(to_change2[i]);
        while(t!=std::string::npos){
          input.replace(t, to_change2[i].length(), to_change2[i+1]);
          t = input.find(to_change2[i], t+to_change2[i].length()-1-(i<20?1:0));
          //again idk why but an extra -1 was needed after some changes
          //to the code
          ///again, due to multibyte nature of superscripts
        }
      }
  
      //normalize signs and decimals
      for(int i = 0; i < input.size(); i++){
        if(input[i]=='+'||input[i]=='-'){
          int count = input[i]=='-', j = 1;
          
          //if the current character (we can check characters safely
          //now that we have changed unicode characters into
          //ASCII characters) is a '+' or a '-', find the next character
          //which isn't one of them and replace the entire substring
          //with equivalent sign
          
         while((i+j)<input.size()&&(input[i+j]=='+'||input[i+j]=='-')){
            count+=input[i+j]=='-';
            j++;
          }
          if(j>1) input.replace(i, j, (count&1?"-":"+"));
        }
        
        //if a decimal is found, check if it follows a number, if not insert
        //a 0 before it, and if it isn't followed by a number, remove it
        
        else if(input[i]=='.'){
          if(i==input.size()-1||(i<input.size()-1&&!std::isdigit(input[i+1]))){
            input.erase(i, 1);
            i--;
          }
          if(i>-1&&(input[i]=='.')&&(i==0||(i>0&&!std::isdigit(input[i-1])))){
            input.insert(i,"0");
            i++;
            if(i==input.size()-1||(i<input.size()-1&&!std::isdigit(input[i+1]))){
              input.erase(i, 1);
              i--;
            }
          }
        }
      }
    }

    //replace the '#'(ASCII)(temporarily replaced for '√'(unicode))
    //with sqrt() format
    void add_sqrt(std::string& input) noexcept{
      
      //sets of delimiters to check from
      std::unordered_set<char> map{
        {'+'},{'-'},{'*'},{'/'},{'^'},{')'}
      };
      
      //count the number of #, which we have to change
      
      int count = std::count(input.begin(), input.end(), '#');
      int t = input.length()-1, h, parenthesis=0;
      
      for(int i = 0; i < count; i++){
        
        //find the first '#' from back(right associative)
        //starting from previous index of '#' found
        for(h = t+1; h >= 0; h--)
          if(input[h]=='#'){t=h;break;}
        
        //replace the '#' with sqrt(
        input.replace(t,1,"sqrt(");
        
        //start from the newer index which is changed by
        //adding the 5 character wide "sqrt("
        //and find the appropriate index to insert closing
        //parenthesis
        t += 5;
        int j = t;
        for(; j < input.length(); j++){
          if((map.count(input[j]))&&parenthesis==0){
            if(j==t&&(input[j]=='+'||input[j]=='-')) continue;
            if(!(j==t+1&&((input[j]=='(')&&(input[j-1]=='+'||input[j-1]=='-'))))
            break;
          }
          if(input[j]=='(')parenthesis++;
          else if(input[j]==')')parenthesis--;
        }
        if(j<input.size()-1){
          input.insert(j,")");
        }
        else input+=")";
      }
    }

    //convert ^,/,*,+,- into function like structure
    void binary_ops(std::string& input, char delimiter) noexcept{
      
      //delimiters to make sure we insert at correct position
      std::unordered_set<char> map{
        {'+'},{'-'},{'*'},{'/'},{'^'},{','},{'('}
      };
      
      //initialize function name to be added and the potential
      //error message using a switch and based on input delimiter
      std::string function, error_name;
      bool check_hang = true;
      switch(delimiter){
        case'^':
          function = "pow(";
          error_name = "EXPONENTIATION";
          break;
        case'/':
          function = "div(";
          error_name = "DIVISION";
          break;
        case'*':
          function = "mul(";
          error_name = "MULTIPLICATION";
          break;
        case'-':
          function = "sub(";
          error_name = "SUBTRACTION";
          check_hang = false;
          break;
        case'+':
          function = "add(";
          error_name = "ADDITION";
          check_hang = false;
          break;
      }
      
      //if delimiter is '^' starting position is end of the string
      //otherwise at the start to respect the associativity of
      //operations
      int index = delimiter=='^'?input.length()-2:0, count = std::count(input.begin(), input.end(), delimiter);
      
      //used stop as a lambda to account for update in string and indices
      auto stop = [&](){return delimiter=='^'?0:input.length();};
      //to find the next delimiter go back by 1 if delimiter is carret
      //otherwise go ahead for associativity
      int update = delimiter=='^'?-1:1, offset = delimiter=='^'?1:0;
      
      for(int i = 0; i < count; i++){
        
        for(int j = index+offset; j != stop(); j+=update){
          if(input[j]==delimiter){
            index = j;
            break;
          }
        }
        
        input[index]=',';
        if(index==0||(index>0&&map.count(input[index-1]))){
          
          //if the operator can't have no first argument
          //which are '*','/'&'^' log error anf return
          if(check_hang){
            this->err_log = "HANGING ";
            this->err_log += error_name;
            this->err_log += " OPERATOR.";
            this->anamoly = true;
            return;
          }
          
          //if the standalone operator is '+' erase it, as for 
          //example +8==8
          if(delimiter=='+'){
            input.erase(index, 1);
            index--;
            continue;
          }
          
          //if the standalone operator is '-' subtract from 0
          else{
            input.insert(index>0?index:0, "sub(0");
            index += 5;
          }
        }
        
        //if not standalone find the appropiate left position
        //and insert the function name
        else{
          int parenthesis = 0, j = index - 1;
          for(; j > 0; j--){
            if(map.count(input[j])&&parenthesis==0){
              break;
            }
            if(input[j]==')')parenthesis++;
            else if(input[j]=='(')parenthesis--;
          }
          input.insert(j<=0?0:j+1, function);
          index+=4;
        }
        
        //erase '(' from map as to make sure we don't stop
        //anywhere prematurely and add ')' as a new delimiter
        map.erase('(');
        map.insert(')');
        
        int j = index+1, parenthesis=0;
        for(;j < input.length(); j++){
          if(map.count(input[j])&&parenthesis==0){
            if((delimiter!='+'&&delimiter!='-')&&(input[j]=='+'||input[j]=='-')&&j==index+1) continue;
              break;
          }
          if(input[j]==')')parenthesis--;
          else if(input[j]=='(')parenthesis++;
        }
        
        //if stopped index is right after index it is a trailing
        //operator and so flag it as error
        if(j-index==1||index==input.length()-1){
          this->err_log = "TRAILING ";
          this->err_log += error_name;
          this->err_log += " OPERATOR.";
          this->anamoly = true;
          return;
        }
        
        //insert ')' at the position found
        else if(j<input.length()-1) input.insert(j,")");
        else input+=")";
        
        //reset map to default
        map.erase(')');
        map.insert('(');
      }
    }
    
    void func_name_norm(std::string& input){
 
      //converts several aliases to a consistent set of functions
      //things like cosine also get convered because of the order
      //in which they are performed ultimately reducing the number
      //of loops needed.
      //eg, cosine->co-sine->co-sin->cosin->cos
  
      std::string to_change[18] = {
        "sine", "sin",
        "cosin", "cos",
        "tangent", "tan",
        "secant", "sec",
        "cotan", "cot",
        "log", "ln",
        "arc", "a",
        "cosec", "csc",
        "ar", "a"
      };
      for(int i = 0; i < 18; i+=2){
    
        size_t t = input.find(to_change[i]);
    
        while(t!=std::string::npos){
          //a separate check for "arc" to remove incorrectly flagging
          //arcot, which would've become aot without separation
          if(to_change[i]=="arc"){
            if((t+to_change[i].length())<input.length()&&!(
            input[t+to_change[i].length()]=='o'))
            input.replace(t, to_change[i].length(), to_change[i+1]);
          }
      
          else input.replace(t, to_change[i].length(), to_change[i+1]);
          t = input.find(to_change[i], t+to_change[i+1].length());
      
        }
      }
    }

    //change continous sets of implicit multiplication
    //to explicit multiplication
    void implicit_mult(std::string& input){
  
      std::unordered_set<char> lookup{
        'x','0','1','2','3','4','5','6','7','8','9',
        ')','(','#'
      };
  
      for(int i = 0; i < input.length(); i++){
    
        //sqrt rule
        if(input[i]=='#'){
          if(i>0&&lookup.count(input[i-1])&&!
          (input[i-1]=='('||input[i-1]=='#')){
            input.insert(i,"*");
            i++;
          }
      
          //error check
          if((i+1)<input.size()&&input[i+1]==')'){
            this->err_log = "INVALID PARENTHESIS, NO ARGUMENT INSIDE.";
            this->anamoly = true;
            return;
          } 
    
        }
        
        //variable rule
        else if(input[i]=='x'){
          if(i>0&&input[i-1]=='e') continue;
          if(i>0&&lookup.count(input[i-1])&&
          !(input[i-1]=='('||input[i-1]=='#')){
            input.insert(i,"*");
            i++;
          }
          if((i+1)<input.length()&&(std::islower(input[i+1])||(
          (lookup.count(input[i+1])&&input[i+1]!=')')))){
            input.insert(i+1,"*");
          }
        }
        
        //function rule
        else if(std::islower(input[i])){
          if(i>0&&(std::isdigit(input[i-1])||input[i-1]==')')){
            input.insert(i,"*");
            i++;
          }
        }
        
        //opening parenthesis rule
        else if(input[i]=='('){
          if(i>0&&lookup.count(input[i-1])&&
          !(input[i-1]=='('||input[i-1]=='#')){
            input.insert(i,"*");
            i++;
          }
    
          //error check
          if((i+1)<input.size()&&input[i+1]==')'){
            this->err_log = "INVALID PARENTHESIS, NO ARGUMENT INSIDE.";
            this->anamoly = true;
            return;
          } 
    
        }
        
        //closing parenthesis rule
        else if(input[i]==')'){
          if((i+1)<input.length()&&lookup.count(input[i+1])&&
          !(input[i+1]==')')){
            input.insert(i+1,"*");
          }
        }
      }
    }

    //check if a superscript is directly followed by
    //a decimal which is likely to be an error
    bool pow_dec_wrong(const std::string& input){
      std::string super[10]={
        "⁰","¹","²","³","⁴","⁵","⁶","⁷","⁸","⁹"
      };
      for(int i = 0; i < 10; i++){
        size_t t = input.find(super[i]);
        while(t!=std::string::npos){
          if((t+super[i].length()<input.length())&&input[t+super[i].length()]=='.')
          return true;
          t = input.find(super[i], t+super[i].length());
        }
      }
      return false;
    }
    
    //check for any invalid characters that aren't allowed
    bool invalid_char(const std::string& input){
      std::unordered_set<std::string> lookup{
        "⁰","¹","²","³","⁴","⁵","⁶","⁷","⁸","⁹",
        "0","1","2","3","4","5","6","7","8","9",
        "e","x","p","s","q","r","t","a","n","i",
        "c","o","w","l","g","h","^","+","-","/",
        "*","÷","×","·","√","π",".","(",")"
      };
      for (int i = 0; i < input.size(); ) {
        unsigned char c = input[i];
        int len = 1;
        //shenanigans for unicode
        if ((c & 0x80) == 0) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        else return true;
        if((i+len)>input.size()) return true;
        if (!lookup.count(input.substr(i, len))) return true;
        i += len;
      }
      return false;
    }

    void remove_empt(std::string& input, bool& flag){
       //remove any empty parenthesis and turn flag boollean
       //to true for further correct error_message
       size_t t = input.find("()");
       while(t!=std::string::npos){
        flag = true;
         input.replace(t, 2, "");
         t = input.find("()", t==0?0:t-1);
       }
     }
    
    //handle incorrect operator orders and malperformed
    //operators
    void malform_ops(std::string& input){
  
      //flag early if encountering an empty string
      if(input.empty()){
        this->err_log = "NO EXPRESSION.";
        this->anamoly = true;
        return;
      }
  
      //sets for fast lookups
      std::unordered_set<char> ops{
        '+','-','*','/','^'
      };
      std::unordered_set<char> sign{
        '+','-'
      };
  
      //flag lone signs
      if(input.size()==1&&ops.count(input.front())){
        this->err_log = "NO FUNCTIONALITY TO OPERATOR.";
        this->anamoly = true;
        return;
      }
  
      for(int i = 0; i < input.length(); i++){
    
        //handled in other functions too but flag for safety
        if(input[i]=='('&&(i+1)<input.length()&&input[i+1]==')'){
          this->err_log = "EMPTY PARENTHESIS, NO ARGUMENT.";
          this->anamoly = true;
          return;
        }
    
        if(ops.count(input[i])){
      
          //if a unary plus sign at the start, flag it
          if(i==0&&input[i]=='+'){
            if(input.size()!=1){
              input.erase(i,1);
              i--;
              continue;
            }
            else{
              this->err_log = "NO FUNCTIONALITY TO OPERATOR.";
              this->anamoly = true;
            }
          }
      
          //rules for error flag or unary plus sign removal
          if(i>0){
            if(input[i-1]=='('&&input[i]!='-'){
              if(input[i]=='+'){
                input.erase(i, 1);
                i--;
                continue;
              }
              this->err_log = "HANGING OPERATOR AFTER PARENTHESIS.";
              this->anamoly = true;
              return;
            }
            if((i+1)<input.size()){
              if(input[i+1]==')'){
                this->err_log = "TRAILING OPERATOR BEFORE PARENTHESIS.";
                this->anamoly = true;
                return;
              }
              else if(ops.count(input[i+1])){
                if(input[i+1]=='+'){
                  input.erase(i+1, 1);
                  i--;
                  continue;
                }
                else if(!sign.count(input[i])&&input[i+1]=='-') continue;
                else{
                  this->err_log = "MALFORMED OPERATORS.";
                  this->anamoly = true;
                  return;
                }
              }
            }
            else{
              this->err_log = "TRAILING OPERATOR.";
              this->anamoly = true;
              return;
            }
          }
          else if(!sign.count(input[i])||((i+1)<input.length()&&sign.count(input[i+1]))){
            this->err_log = "HANGING OPERATOR.";
            this->anamoly = true;
            return;
          }
        }
      }
    }

    void invalid_decimal(const std::string& input){
  
      //early empty string check
      if(input.empty()){
        this->err_log = "EMPTY STRING.";
        this->anamoly = true;
        return;
      }
  
  
      size_t t = input.find(".");
      if(t==0){
        this->err_log = "HANGING DECIMAL.";
        this->anamoly = true;
        return;
      }
  
      while(t!=std::string::npos){

        //check if decimal is preceded by a number
        if(!std::isdigit(input[t-1])){
          this->err_log = "DECIMAL NOT PRECEDED BY A NUMBER.";
          this->anamoly = true;
          return;
        }

        //if the position is less than the current string length
        if(t<input.length()){

          if(!std::isdigit(input[t+1])){
            this->err_log = "DECIMAL NOT FOLLOWED BY A NUMBER.";
            this->anamoly = true;
            return;
          }

          else{
            int j = 1;

            //find the next non-digit character
            while((t+j)<input.length()&&std::isdigit(input[t+j]))
              j++;

            //if the next non-digit is still within the string
            if((t+j)<input.length()){

              //and it is another decimal, flag it
              if(input[t+j]=='.'){
                this->err_log = ((t+j==input.length()-1)?"TRAILING DECIMAL."
                :"MULTIPLE DECIMALS IN ONE NUMBER.");
                this->anamoly = true;
                return;
              }
            }

            //if the only thing present after decimal were digits
            //this case would catch that and return because no
            //further evaluation is needed
            else return;
          }
        }

        t = input.find(".", t+1);
      }
    }

  public:
    
    expression(){}
    
    expression(bool allow_var): allow_variable(allow_var){}
    
    expression(const std::string& input, bool allow_var = true): entered_expression(input), allow_variable(allow_var){}

    void clean(){
      if(!this->anamoly) return;
        
      this->anamoly = false;
      
      //initialization
      std::string &unmodified = this->entered_expression;
      std::string &input = this->modified_expression;
      input = unmodified;
      if(input.empty()){
        this->anamoly = true;
        this->err_log = "EMPTY STRING.";
        return;
      }
      bool flag = false;
      
      //early formatting for consistency
      std::string temp;
      for(int i = 0; i < input.length(); i++){
        unsigned char c = input[i];
        if(std::isspace(c)) continue;
        else temp+=std::isupper(c)?std::tolower(c):c;
      }
      input = temp;
      
      //capture analomy, a superscript followed by a normal decimal
      if(pow_dec_wrong(input)){
        this->err_log = "POWER FOLLOWED BY NORMAL DECIMAL.";
        this->anamoly = true;
        return;
      }
  
      //initial check for early return of invalid function
      if(invalid_char(input)){
        this->err_log = "INVALID CHARACTERS.";
        this->anamoly = true;
        return;
      }
      
      //alias to norm function names
      func_name_norm(input);
          
      //clean some potential decimal mistakes and
      //flag it if not corrected
      basic_cleanup(input);
  
      //check if even after cleanup decimal issue persist
      invalid_decimal(input);
      if(this->anamoly) return;
      
      //get appropriate parenthesis
      app_paren(input, flag);
      
      remove_empt(input, flag);
      
      implicit_mult(input);
      if(this->anamoly) return;
        
      //change from √ to sqrt() and
      //remove empty parenthesis "()"
      if(input.find("#")!=std::string::npos)add_sqrt(input);
      remove_empt(input, flag);
    
      //remove malperfomed operations such as '+)' (implicit addition)
      malform_ops(input);
      if(this->anamoly) return;
        
      //check for appropriate function calls
      app_func_call(input, flag);
      if(this->anamoly) return;
  
      //check for appropriate functions/variables
      app_func_name(input);
      if(this->anamoly) return;
  
      //even if after correcting inconsistent parenthesis persist
      //flag it and log the error
      int opening_parenthesis = std::count(input.begin(),input.end(),'(');
      int closing_parenthesis = std::count(input.begin(),input.end(),')');
      if(opening_parenthesis!=closing_parenthesis){
        this->err_log = "INCORRECT PARENTHESIS.";
        this->anamoly = true;
        return;
      }
 
      //convert ^+/-* into function-like format
      std::string call = "^/*-+";
      for(auto ch: call){
        binary_ops(input, ch);
        if(this->anamoly) return;
      }
  
      //checking if there are () for potential redundancy removal
      if(input.find("(")!=std::string::npos)paren_cleaner(input);
      if(this->anamoly)return;
      
      //final flag to check if string is empty(or was made empty using
      //the "()" removal block earlier)
      if(input.empty()){
        this->err_log = flag?"INVALID PARENTHESIS.":"EMPTY STRING.";
        this->anamoly = true;
        return;
      }
      
      //update log if successfully updated
      this->err_log = "SUCCESSFULLY MODIFIED.";
      
      //should take less than 50μs for most inputs and should scale
      //well and probably won't exceed 400μs easily
    }
    
    std::string result() const noexcept{
      return this->anamoly?this->err_log:this->modified_expression;
    }
    
    bool is_safe() const noexcept{
      return !this->anamoly;
    }
    
    expression& operator=(const std::string& new_expr) noexcept{
      this->entered_expression = new_expr;
      this->modified_expression.clear();
      this->anamoly = true;
      this->err_log = "NOT MODIFIED.";
      return *this;
    }
    
    expression& clear() noexcept{
      this->entered_expression.clear();
      this->modified_expression.clear();
      this->anamoly = true;
      this->allow_variable = true;
      this->err_log = "NOT MODIFIED.";
      return *this;
    }
    
    expression& update(const std::string& new_expr, const bool& cleanse = false){
      this->entered_expression = new_expr;
      this->modified_expression.clear();
      this->anamoly = true;
      this->err_log = "NOT MODIFIED.";
      if(cleanse) this->clean();
      return *this;
    }
    
    bool is_function() const noexcept{
      if(this->anamoly) return false;
      if(!this->allow_variable) return false;
      size_t t = this->modified_expression.find("x");
      while(t!=std::string::npos){
        if(t==0||(t>0&&this->modified_expression[t-1]!='e')) return true;
        t=this->modified_expression.find("x", t+1);
      }
      return false;
    }
    
    bool is_constant() const noexcept{
      if(this->anamoly) return false;
      return !this->is_function();
    }
    
    void make_function() noexcept{
      this->allow_variable = true;
    }
    
    void make_constant() noexcept{
      if(this->anamoly){
        this->allow_variable = false;
        return;
      }
      if(this->allow_variable){
        if(this->is_function()){
          this->err_log = "VARIABLE IN NON-FUNCTION EXPRESSION.";
          this->anamoly = true;
        }
        this->allow_variable = false;
      }
    }
    
  //printing and entrance logic for convinience sake  
  protected:
    
    void print(std::ostream& os) const{
      os << "\nOriginal: " << this->entered_expression;
      os << "\nModified: ";
      if(!anamoly&&this->modified_expression.find("(")!=std::string::npos){
        os << '\n';
        int parenthesis = 0;
        std::string string = this->modified_expression;
        for(int i = 0; i < string.size(); i++){
          if(string[i]=='('){
            os << string[i] << "\n";
            parenthesis++;
            os << std::string(2*parenthesis, ' ');
          }
          else if(string[i]==')'){
            parenthesis--;
            if(parenthesis<0) std::cerr << "unexpected error, didn't parse correctly.";
            os << "\n" << std::string(2*parenthesis, ' ') << string[i];
          }
          else if(string[i]==','){
            os << string[i] << "\n" << std::string(2*parenthesis, ' ');
          }
          else os << string[i];
        }
      }
      else os << this->modified_expression;
      os << "\nStatus: " << ((anamoly&&this->err_log!="NOT MODIFIED.")?"":"No ") << "Anamoly Detected.";
      os << "\nLog: " << this->err_log << "\n";
    }
    
    friend std::ostream& operator<<(std::ostream& os, const expression& obj){
      obj.print(os);
      return os;
    }
    friend std::istream& operator>>(std::istream& is, expression& obj){
      std::string line;
      std::getline(is, line);
      obj.entered_expression = line;
      obj.clean();
      return is;
    }
};
