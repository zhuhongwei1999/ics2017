#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  rtlreg_t temp,temp1,temp2,temp3;
  bool invert = subcode & 0x1;
  
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:{
      rtl_get_OF(dest);
      break;
    }  
    case CC_B:{
      rtl_get_CF(dest);
      break;
    }   
    case CC_E:{
      rtl_get_ZF(dest);
      // printf("cpu.ZF:%d\n",*dest);
      break;
    }  
    case CC_BE:{
      rtl_get_CF(&temp);
      rtl_get_ZF(&temp1);
      rtl_or(dest,&temp,&temp1);
      break;
    }  
    case CC_S:{
      rtl_get_SF(dest);
      break;
    }  
    case CC_L:{
      rtl_get_SF(&temp);
      rtl_get_OF(&temp1);
      rtl_li(dest,(temp!=temp1));
      break;
    }      
    case CC_LE:
        rtl_get_ZF(&temp);
        rtl_get_SF(&temp1);
        rtl_get_OF(&temp2);
        temp3=(temp1!=temp2?1:0);
        rtl_or(dest,&temp,&temp3);
        break;
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
