#ifndef WIN_KEY_HPP_INCLUDED
#define WIN_KEY_HPP_INCLUDED

#include <cstdint>



namespace win {

enum class key : uint32_t {
  space    = ' ',

  a        = 'a',
  b        = 'b',
  c        = 'c',
  d        = 'd',
  e        = 'e',
  f        = 'f',
  g        = 'g',
  h        = 'h',
  i        = 'i',
  j        = 'j',
  k        = 'k',
  l        = 'l',
  m        = 'm',
  n        = 'n',
  o        = 'o',
  p        = 'p',
  q        = 'q',
  r        = 'r',
  s        = 's',
  t        = 't',
  u        = 'u',
  v        = 'v',
  w        = 'w',
  x        = 'x',
  y        = 'y',
  z        = 'z',

  a_upper  = 'A',
  b_upper  = 'B',
  c_upper  = 'C',
  d_upper  = 'D',
  e_upper  = 'E',
  f_upper  = 'F',
  g_upper  = 'G',
  h_upper  = 'H',
  i_upper  = 'I',
  j_upper  = 'J',
  k_upper  = 'K',
  l_upper  = 'L',
  m_upper  = 'M',
  n_upper  = 'N',
  o_upper  = 'O',
  p_upper  = 'P',
  q_upper  = 'Q',
  r_upper  = 'R',
  s_upper  = 'S',
  t_upper  = 'T',
  u_upper  = 'U',
  v_upper  = 'V',
  w_upper  = 'W',
  x_upper  = 'X',
  y_upper  = 'Y',
  z_upper  = 'Z',



  escape   = 0xff1b,
  home     = 0xff50,
  left     = 0xff51,
  up       = 0xff52,
  right    = 0xff53,
  down     = 0xff54,
  end      = 0xff57,

  plus     = '+',
  minus    = '-',

  kp_plus  = 0xffab,
  kp_minus = 0xffad,

  kp_0     = 0xffb0,
  kp_1     = 0xffb1,
  kp_2     = 0xffb2,
  kp_3     = 0xffb3,
  kp_4     = 0xffb4,
  kp_5     = 0xffb5,
  kp_6     = 0xffb6,
  kp_7     = 0xffb7,
  kp_8     = 0xffb8,
  kp_9     = 0xffb9,

  f1       = 0xffbe,
  f2       = 0xffbf,
  f3       = 0xffc0,
  f4       = 0xffc1,
  f5       = 0xffc2,
  f6       = 0xffc3,
  f7       = 0xffc4,
  f8       = 0xffc5,
  f9       = 0xffc6,
  f10      = 0xffc7,
  f11      = 0xffc8,
  f12      = 0xffc9,
};



[[nodiscard]] constexpr key key_from_char(char c) {
  return static_cast<key>(c);
}

}

#endif // WIN_KEY_HPP_INCLUDED
