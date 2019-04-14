#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_BIG = 255, TK_SMALL = 254, TK_BIG_EQ = 253, 
  TK_SMALL_EQ = 252, TK_LS = 251, TK_RS = 250, TK_PLUS = 249, TK_SUB = 248, 
  TK_MUL = 247, TK_DIV = 246, TK_EQ = 245, TK_DEC = 244, TK_HEX = 243, TK_POINT = 242, 
  TK_MOD = 241, TK_OR = 240, TK_AND = 239, TK_NOT = 238, TK_$ = 237, TK_NOT_EQ = 236, 
  TK_NEG = 235, TK_LBRACE = 234, TK_RBRACE = 233, TK_REG = 232, DEREF = 231

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},         // equal
	{">", TK_BIG},
	{"<", TK_SMALL},
	{">=", TK_BIG_EQ},
	{"<=", TK_SMALL_EQ},
	{"<<", TK_LS},
	{">>", TK_RS},
	{"-", TK_SUB},
	{"\\*", TK_MUL},
	{"\\/", TK_DIV},
	{"[0-9]{1,10}", TK_DEC},
	{"0x[0-9a-fA-F]{1,8}", TK_HEX},
	{"\\.", TK_POINT},
	{"\\%", TK_MOD},
	{"&&", TK_AND},
	{"\\|\\|", TK_OR},
	{"!", TK_NOT},
	{"$", TK_$},
	{"!=", TK_NOT_EQ},
	{"\\(", TK_LBRACE},
	{"\\)", TK_RBRACE},
	{"[a-z]{1,5}", TK_REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )
//rules的个数

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        for (int j = 0; j < 32; j++)
          tokens[nr_token].str[j] = '\0';
        strncpy(tokens[nr_token].str, e + position, substr_len);
        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
					case TK_NOTYPE:
            break;
          case TK_LBRACE:
            tokens[nr_token++].type = TK_LBRACE;
            break;
          case TK_RBRACE:
            tokens[nr_token++].type = TK_RBRACE;
            break;
          case TK_$:
            tokens[nr_token++].type = TK_$;
            break;
          case TK_MUL:
            tokens[nr_token++].type = TK_MUL;
            break;
          case TK_DIV:
            tokens[nr_token++].type = TK_DIV;
            break;
          case TK_PLUS:
            tokens[nr_token++].type = TK_PLUS;
            break;
          case TK_SUB:
            tokens[nr_token++].type = TK_SUB;
            break;
          case TK_MOD:
            tokens[nr_token++].type = TK_MOD;
            break;
          case TK_LS:
            tokens[nr_token++].type = TK_LS;
            break;
					case TK_RS:
            tokens[nr_token++].type = TK_RS;
            break;
          case TK_BIG:
            tokens[nr_token++].type = TK_BIG;
            break;
          case TK_SMALL:
            tokens[nr_token++].type = TK_SMALL;
            break;
          case TK_BIG_EQ:
            tokens[nr_token++].type = TK_BIG_EQ;
            break;
          case TK_SMALL_EQ:
            tokens[nr_token++].type = TK_SMALL_EQ;
            break;
          case TK_NOT_EQ:
            tokens[nr_token++].type = TK_NOT_EQ;
            break;
          case TK_EQ:
            tokens[nr_token++].type = TK_EQ;
            break;
          case TK_HEX:
            tokens[nr_token++].type = TK_HEX;
            break;
          case TK_DEC:
            tokens[nr_token++].type = TK_DEC;
            break;
          case TK_REG:
            tokens[nr_token++].type = TK_REG;
            break;
          default:
            break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q){
  if(tokens[p].type == TK_LBRACE && tokens[q].type == TK_RBRACE){
    int cnt = 0;
    for(int i=p+1; i<q; i++){
      if(tokens[i].type == TK_LBRACE) cnt++;
      else if(tokens[i].type == TK_RBRACE) cnt--;
      if(cnt < 0) return false;
    }
    if(cnt == 0) return true;
    else return false;
  }
  else return false;
}

int find_dominated_op(int p, int q){
  int op = -1, cnt = 0, op_type = -1;
  for(int i=p; i<=q; i++){
    if(tokens[i].type == TK_LBRACE) cnt++;
    else if(tokens[i].type == TK_RBRACE) cnt--;
    else if(cnt!=0 || tokens[i].type==TK_HEX || tokens[i].type==TK_DEC) continue;
    else if(cnt == 0){
      if(tokens[i].type==TK_MUL || tokens[i].type==TK_DIV || tokens[i].type==TK_MOD){
        if(op_type != TK_PLUS && op_type != TK_SUB && op_type != TK_LS && op_type != TK_RS &&
         op_type != TK_BIG_EQ && op_type != TK_SMALL_EQ && op_type != TK_SMALL && op_type != TK_BIG && 
         op_type != TK_NOT_EQ && op_type != TK_EQ && op_type != TK_AND && op_type != TK_OR){
           op_type = tokens[i].type;
           op = i;
         }
      }

      if(tokens[i].type==TK_PLUS || tokens[i].type==TK_SUB){
        if(op_type != TK_LS && op_type != TK_RS &&
         op_type != TK_BIG_EQ && op_type != TK_SMALL_EQ && op_type != TK_SMALL && op_type != TK_BIG && 
         op_type != TK_NOT_EQ && op_type != TK_EQ && op_type != TK_AND && op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_LS || tokens[i].type==TK_RS){
        if(op_type != TK_BIG_EQ && op_type != TK_SMALL_EQ && op_type != TK_SMALL && op_type != TK_BIG && 
          op_type != TK_NOT_EQ && op_type != TK_EQ && op_type != TK_AND && op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_BIG_EQ || tokens[i].type==TK_SMALL_EQ || tokens[i].type==TK_BIG || tokens[i].type==TK_SMALL){
        if(op_type != TK_NOT_EQ && op_type != TK_EQ && op_type != TK_AND && op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_EQ || tokens[i].type==TK_NOT_EQ){
        if(op_type != TK_AND && op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_EQ || tokens[i].type==TK_NOT_EQ){
        if(op_type != TK_AND && op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_AND){
        if(op_type != TK_OR){
          op_type = tokens[i].type;
          op = i;
        }
      }

      if(tokens[i].type==TK_OR){
        op_type = tokens[i].type;
        op = i;
      }
    }
  }
  return op;
}

uint32_t eval(int p, int q) {
    if (p > q) {
      printf("Bad Expression!\n");
      assert(0);
    }
    else if (p == q) {
      uint32_t sum = 0;
      if (tokens[p].type == TK_DEC){
        sscanf(tokens[p].str, "%d", &sum);
      }
      else if (tokens[p].type == TK_HEX){
        sscanf(tokens[p].str, "%x", &sum);
      }
      else{
        printf("Bad Expression!\n");
        assert(0);
      }
      return sum;
    }
        /* Single token.
        * For now this token should be a number.
        * Return the value of the number.
        */
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
        * If that is the case, just throw away the parentheses.
        */
        return eval(p + 1, q - 1);
    }
    else {
        /* We should do more things here. */
        uint32_t op, val_1, val_2, result;
        op = find_dominated_op(p, q);
        if(op == -1){
          int k = p;
          while(tokens[k].type==TK_NEG || tokens[k].type==TK_$ || tokens[k].type==DEREF) k++;

          for(int i=k-1; i>=p; i--){
            if(tokens[i].type == TK_NEG){
              sscanf(tokens[i+1].str, "%d", &result);
              result = -result;
            }
            else if(tokens[i].type == TK_$){
              if (strcmp("eip", tokens[i+1].str) == 0) result = cpu.eip;
              for (int j = 0; j < 8; j++){
                if(strcmp(regsl[j], tokens[i+1].str) == 0)
                result = cpu.gpr[j]._32;
              }
            }
            else if(tokens[i].type == DEREF){
              sscanf(tokens[i+1].str, "%d", &result);
              result = vaddr_read(result, 4);
            }
          }
          return result;
        }
        val_1 = eval(p, op - 1);
        val_2 = eval(op + 1, q);
        
        //????
        switch (tokens[op].type){
          case TK_PLUS:
            return val_1 + val_2;
          case TK_SUB:
            return val_1 - val_2;
          case TK_MUL:
            return val_1 * val_2;
          case TK_DIV:
            return val_1 / val_2;
          case TK_MOD:
            return val_1 % val_2;
          case TK_LS:
            return val_1 << val_2;
          case TK_RS:
            return val_1 >> val_2;
          case TK_SMALL_EQ:
            if (val_1 <= val_2) return 1;
            else return 0;
          case TK_BIG_EQ:
            if (val_1 >= val_2) return 1;
            else return 0;
          case TK_SMALL:
            if (val_1 < val_2) return 1;
            else return 0;
          case TK_BIG:
            if (val_1 > val_2) return 1;
            else return 0;
          case TK_EQ:
            return val_1 == val_2;
          case TK_NOT_EQ:
            return val_1 != val_2;
          case TK_AND:
            if (val_1 == 1 && val_2 == 1) return 1;
            else return 0;
          case TK_OR:
            if (val_1 == 1 || val_2 == 1) return 1;
            else return 0;
          default:
            assert(0);
        }
    }
}

bool is_num(int i){
  if(tokens[i].type!=TK_DEC && tokens[i].type!=TK_HEX && tokens[i].type!=TK_LBRACE){
    return false;
  }
  else return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    success = false;
    return 0;
  }


  /* TODO: Insert codes to evaluate the expression. */
  for(int i=0; i<nr_token; i++){
    if(tokens[i].type==TK_SUB && (i==0 || !is_num(i-1))){
      tokens[i].type = TK_NEG;
    }
    else if(tokens[i].type==TK_MUL && (i==0 || !is_num(i-1))){
      tokens[i].type = DEREF;
    }
  }
  return eval(0, nr_token-1);
}
