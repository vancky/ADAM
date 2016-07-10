#ifndef MATRIXOPS
#define MATRIXOPS
void matrix_transpose(double *A,int m,int n);
void flip_dim(double *A,int m,int n,int dim);
void solve_matrix_eqS(double *A,int m,double *B,double *X);
void matrix_transprod(double *A,int m,int n,double *B);
void matrix_adddiag(double *A,double *B,int n,double lambda);
void matrix_prodplusdiag(double *J,int m,int n,double lambda,double **JTJ);
void matrix_vectorprod(double *A,int m,int n,double *V,double *B,int trans);
void matrix_minus(double *A,int m,int n,double *B);
void matrix_plus(double *A,int m,int n,double *B);
void solve_matrix_eq(double *A,int m,double *B,double *X);
void matrix_plus2(double *A,int m,int n,double *B,double *C);
void matrix_minus2(double *A,int m,int n,double *B,double **C);
void matrix_prod(double  *A,int m,int k,double  *B,int n,double  *C);
void real_matrix_multiplyT_ele(double *A,int *B,int k,int m,double *C);
void real_matrix_multiplyT(double *A,double *B,int m,int k,int n,double *C);
void matrix_prod_ATB(double *A,int m,int k,double *B,int n,double *C);
void matrix_prod_ATBA(double *A,int m,int k,double *B,double *D);
#endif
