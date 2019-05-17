#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
  TK_NOTYPE = 256,
  LMOVE = 255,
  RMOVE = 254,
  BE = 253,
  SE = 252,
  TK_EQ = 251,
  TK_FEQ = 250,
  TK_NUM_16 = 249,
  TK_NUM_8 = 248,
  TK_NUM_10 = 247,
  TK_REG = 246,
  TK_NAG = 245,
  DEREF = 244,
  /* TODO: Add more token types */
};

static struct rule
{
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
    {" +", TK_NOTYPE}, // spaces
    {"\\(", '('},
    {"\\)", ')'},
    {"\\$", '$'},
    {"\\*", '*'},
    {"/", '/'},
    {"%", '%'},
    {"\\+", '+'},
    {"-", '-'},
    {"<<", LMOVE},
    {">>", RMOVE},
    {">=", BE},
    {">", '>'},
    {"<=", SE},
    {"<", '<'}, //有等于号的要放在前面
    {"==", TK_EQ},
    {"!=", TK_FEQ},
    {"&&", '&'},
    {"\\|\\|", '|'},
    {"0x[0-9a-fA-F]{1,8}", TK_NUM_16},
    {"0[0-7]{1,8}", TK_NUM_8},
    {"[0-9]{1,10}", TK_NUM_10},
    {"[a-z]{1,10}", TK_REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    // regcomp 这个函数把指定的正则表达式 rules[i].regex 编译成一种特定的数据格式 re[i]，这样可以使匹配更有效。
    //函数 regexec 会使用这个数据在目标文本串中进行模式匹配。执行成功返回０。
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); //以功能更加强大的扩展正则表达式的方式进行匹配。
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position; //裁下的字段的起始位置
        int substr_len = pmatch.rm_eo;		 //裁下的字段的长度

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        //每次都初始化避免溢出
        for (int j = 0; j < 32; j++)
          tokens[nr_token].str[j] = '\0';
        //把划分后的语句及时存入
        strncpy(tokens[nr_token].str, e + position, substr_len);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          break;
        case '(':
          tokens[nr_token++].type = '(';
          break;
        case ')':
          tokens[nr_token++].type = ')';
          break;
        case '$':
          tokens[nr_token++].type = '$';
          break;
        case '*':
          tokens[nr_token++].type = '*';
          break;
        case '/':
          tokens[nr_token++].type = '/';
          break;
        case '%':
          tokens[nr_token++].type = '%';
          break;
        case '+':
          tokens[nr_token++].type = '+';
          break;
        case '-':
          tokens[nr_token++].type = '-';
          break;
        case LMOVE:
          tokens[nr_token++].type = LMOVE;
          break;
        case RMOVE:
          tokens[nr_token++].type = RMOVE;
          break;
        case BE:
          tokens[nr_token++].type = BE;
          break;
        case SE:
          tokens[nr_token++].type = SE;
          break;
        case '>':
          tokens[nr_token++].type = '>';
          break;
        case '<':
          tokens[nr_token++].type = '<';
          break;
        case TK_EQ:
          tokens[nr_token++].type = TK_EQ;
          break;
        case TK_FEQ:
          tokens[nr_token++].type = TK_FEQ;
          break;
        case '&':
          tokens[nr_token++].type = '&';
          break;
        case '|':
          tokens[nr_token++].type = '|';
          break;
        case TK_NUM_16:
          tokens[nr_token++].type = TK_NUM_16;
          break;
        case TK_NUM_8:
          tokens[nr_token++].type = TK_NUM_8;
          break;
        case TK_NUM_10:
          tokens[nr_token++].type = TK_NUM_10;
          break;
        case TK_REG:
          tokens[nr_token++].type = TK_REG;
          break;
        default:
          TODO();
        }
        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  // printf("num:%d\n",nr_token);
  return true;
}

bool check_parentheses(int p, int q)
{
  if (tokens[p].type == '(' && tokens[q].type == ')')
  {
    int i, j, rValue, flag;
    for (rValue = q - 1, i = p + 1; i <= rValue; i++)
    {
      if (tokens[i].type == '(')
      {
        for (flag = 0, j = rValue; j > i; j--)
        {
          if (tokens[j].type == ')')
          {
            rValue = j - 1;
            flag = 1;
          }
        }
        if (flag == 0) //左括号没有与之匹配的右括号
          return false;
      }
      else if (tokens[i].type == ')')
      { //先出现右括号
        return false;
      }
    }
    return true;
  }
  return false;
}

int searchDominantOperator(int p, int q)
{
  int op = -1, cnt = 0, op_type = -1;
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
      cnt++;
    else if (tokens[i].type == ')')
      cnt--;
    //出现在括号中的 and 非运算符不需要处理
    else if (cnt != 0 || tokens[i].type == TK_NUM_16 || tokens[i].type == TK_NUM_8 || tokens[i].type == TK_NUM_10)
      continue;
    //处理双目运算符
    else if (cnt == 0)
    {
      //符号有优先级之分
      if (tokens[i].type == '*' || tokens[i].type == '/' || tokens[i].type == '%')
      {
        if (op_type != '+' && op_type != '-' && op_type != LMOVE && op_type != RMOVE && op_type != BE &&
            op_type != SE && op_type != '>' && op_type != '<' && op_type != TK_FEQ && op_type != TK_EQ &&
            op_type != '|' && op_type != '&')
        {
          op = i;
          op_type = tokens[i].type;
        }
      }
      else if (tokens[i].type == '+' || tokens[i].type == '-')
      {
        if (op_type != LMOVE && op_type != RMOVE && op_type != BE &&
            op_type != SE && op_type != '>' && op_type != '<' && op_type != TK_FEQ && op_type != TK_EQ &&
            op_type != '|' && op_type != '&')
        {
          op = i;
          op_type = tokens[i].type;
        }
      }
      else if (tokens[i].type == LMOVE || tokens[i].type == RMOVE)
      {
        if (op_type != BE &&
            op_type != SE && op_type != '>' && op_type != '<' && op_type != TK_FEQ && op_type != TK_EQ &&
            op_type != '|' && op_type != '&')
        {
          op = i;
          op_type = tokens[i].type;
        }
      }
      else if (tokens[i].type == BE || tokens[i].type == SE || tokens[i].type == '>' || tokens[i].type == '<')
      {
        if (op_type != TK_FEQ && op_type != TK_EQ && op_type != '|' && op_type != '&')
        {
          op = i;
          op_type = tokens[i].type;
        }
      }
      else if (tokens[i].type == TK_FEQ || tokens[i].type == TK_EQ)
      {
        if (op_type != '|' && op_type != '&')
        {
          op = i;
          op_type = tokens[i].type;
        }
      }
      else if (tokens[i].type == '|' || tokens[i].type == '&')
      {
        op = i;
        op_type = tokens[i].type;
      }
    }
  }
  return op;
}

uint32_t eval(int p, int q)
{
  if (p > q)
  {
    printf("Bad expression_1!\n");
    assert(0);
  }
  else if (p == q)
  { //说明是数字，取数即可
    uint32_t sum = -99999;
    if (tokens[p].type == TK_NUM_10)
    {
      sscanf(tokens[p].str, "%d", &sum);
    }
    else if (tokens[p].type == TK_NUM_16)
    {
      sscanf(tokens[p].str, "%x", &sum);
    }
    else if (tokens[p].type == TK_NUM_8)
    {
      sscanf(tokens[p].str, "%o", &sum);
    }
    return sum;
  }
  else if (check_parentheses(p, q) == true)
  {
    return eval(p + 1, q - 1);
  }
  else
  {
    uint32_t op, val_1, val_2, result;
    op = searchDominantOperator(p, q);
    // printf("op:%d\n",op);
    if (op == -1)
    { //函数中里面没有判别的,同时又至少有2位
      int k = p;
      //找到最右侧的单目运算符
      while (tokens[k].type == TK_NAG || tokens[k].type == DEREF || tokens[k].type == '$')
        k++;
      for (int i = k - 1; i >= p; i--)
      {
        if (tokens[i].type == TK_NAG)
        {
          sscanf(tokens[i + 1].str, "%d", &result);
          result *= -1;
        }
        else if (tokens[i].type == DEREF)
        {
          sscanf(tokens[i + 1].str, "%d", &result);
          result = vaddr_read(result, 4);
        }
        else if (tokens[i].type == '$')
        {
          if (strcmp("eip", tokens[i + 1].str) == 0)
            result = cpu.eip;
          for (int j = 0; j < 8; j++)
            if (strcmp(regsl[j], tokens[i + 1].str) == 0)
              result = cpu.gpr[j]._32;
        }
      }
      return result;
    }
    val_1 = eval(p, op - 1);
    // printf("val_1:%d\n",val_1);
    val_2 = eval(op + 1, q);
    // printf("val_2:%d\n",val_2);
    switch (tokens[op].type)
    {
    case '+':
      return val_1 + val_2;
    case '-':
      return val_1 - val_2;
    case '*':
      return val_1 * val_2;
    case '/':
      return val_1 / val_2;
    case '%':
      return val_1 % val_2;
    case LMOVE:
      return val_1 << val_2;
    case RMOVE:
      return val_1 >> val_2;
    case SE:
      if (val_1 <= val_2)
        return 1;
      return 0;
    case BE:
      if (val_1 >= val_2)
        return 1;
      return 0;
    case TK_EQ:
      return val_1 == val_2;
    case TK_FEQ:
      return val_1 != val_2;
    case '&':
      if (val_1 == 0 || val_2 == 0)
        return 0;
      return 1;
    case '|':
      if (val_1 == 1 || val_2 == 1)
        return 1;
      return 0;
    case '<':
      if (val_1 < val_2)
        return 1;
      return 0;
    case '>':
      if (val_1 > val_2)
        return 1;
      return 0;
    default:
      assert(0);
    }
  }
}

bool judge(int x)
{
  if (tokens[x].type != '+' && tokens[x].type != '-' && tokens[x].type != '*' && tokens[x].type != '/' && tokens[x].type != '(' && tokens[x].type != '&' && tokens[x].type != '|' && tokens[x].type != TK_NAG && tokens[x].type != DEREF)
    return false;
  return true;
}

uint32_t expr(char *e)
{
  //词法分析
  if (!make_token(e))
  {
    return 0;
  }
  for (int i = 0; i < nr_token; i++)
  { //提前处理有歧义的单目运算符
    if (tokens[i].type == '-' && (i == 0 || judge(i - 1) == true))
      tokens[i].type = TK_NAG;
    else if (tokens[i].type == '*' && (i == 0 || judge(i - 1)) == true)
      tokens[i].type = DEREF;
  }
  return eval(0, nr_token - 1);
}