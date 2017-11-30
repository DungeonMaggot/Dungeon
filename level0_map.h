#ifndef LEVEL0_MAP_H
#define LEVEL0_MAP_H

#if 1
#define LEVEL_0_WIDTH 25
#define LEVEL_0_HEIGHT 21
char *Level0Map =
"    3+4+ 5               "
"  +++  +++6              "
"  2 +     7              "
"  +    + +8           H  "
" +++   + +     +++D+ FG  "
"+++++ ++ +9A++B+ + + +   "
" +++  ++      +  +++E+ LM"
"++S++  ++    C+ +    I ++"
" ++1  +++       +++  + + "
"  0    ++ +++     + KJ++ "
"        +++ + +++>+    N "
"        +   ++    +    + "
"    ++    +++++>++++++ O "
"   >++++++^ ++  +    +++ "
"+++ ++      +   ++++  +P "
"     +   ++++      +     "
"+++ ++ + +         + ++  "
" ++ ++ +++++ ++    ++++  "
" ++ +    + ++++          "
"  ^ ++X  ^   ++          "
"        ++               ";
#else
#define LEVEL_0_WIDTH 4
#define LEVEL_0_HEIGHT 3
char *Level0Map =
"++  "
" ++ "
"++++";
#endif


#endif // LEVEL0_MAP_H
