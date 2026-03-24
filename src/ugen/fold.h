#ifndef FOLD_H
#define FOLD_H
function is_constant(arg0: pointer): boolean; external;
function fold(arg0: Ptree): pointer; external;
function fold1(arg0: Ptree): boolean; external;
#endif /* FOLD_H */
