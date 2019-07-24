/* Copyright (c) 2012 Massachusetts Institute of Technology
 *
 * [Also included are functions derived from derfc in SLATEC
 *  (netlib.org/slatec), which "is in the public domain"
 *  and hence may be redistributed under these or any terms.]
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* This file provides generic Matlab wrappers for the different Faddeeva::
   functions.  To wrap a specific function, we create a new .cc file with

   #define FADDEEVA_FUNC ...name of function...
   #define FADDEEVA_REAL ...1 if real for real z, 0 if not...
   #include "Faddeeva_mex.cc"
*/

#include "Faddeeva.hh"

#include <mex.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 1 || nrhs > 2) mexErrMsgTxt("expecting one or two arguments");
  if (nlhs > 1) mexErrMsgTxt("expecting only one return value");

  if (!mxIsNumeric(prhs[0]) || !(mxIsDouble(prhs[0]) || mxIsSingle(prhs[0])))
    mexErrMsgTxt(
        "first argument must be numeric array (double or single precision)");

  // relerr = prhs[1], if any
  double relerr;
  if (nrhs < 2)
    relerr = 0;
  else if (mxIsNumeric(prhs[1]) && mxGetM(prhs[1]) * mxGetN(prhs[1]) == 1) {
    if (mxIsDouble(prhs[1]))
      relerr = *mxGetPr(prhs[1]);
    else if (mxIsSingle(prhs[1]))
      relerr = *((float *)mxGetData(prhs[1]));
    else
      mexErrMsgTxt("second argument must be double or single precision");
  } else
    mexErrMsgTxt("second argument must be real scalar");

  mwSize ndim = mxGetNumberOfDimensions(prhs[0]);
  const mwSize *dims = mxGetDimensions(prhs[0]);
  plhs[0] = mxCreateNumericArray(
      ndim, dims, mxDOUBLE_CLASS,
      (FADDEEVA_REAL && !mxIsComplex(prhs[0])) ? mxREAL : mxCOMPLEX);

  size_t N = 1;
  for (mwSize d = 0; d < ndim; ++d) N *= dims[d]; // get total size of array

  if (mxIsDouble(prhs[0])) {
    if (mxIsComplex(prhs[0])) {
      mxComplexDouble *z = mxGetComplexDoubles(prhs[0]);
      mxComplexDouble *wOut = mxGetComplexDoubles(plhs[0]);
#pragma omp parallel for
      for (size_t i = 0; i < N; ++i) {
        std::complex<double> w =
            FADDEEVA_FUNC(std::complex<double>(z[i].real, z[i].imag), relerr);
        wOut[i].real = real(w);
        wOut[i].imag = imag(w);
      }
    } else {
      double *z = mxGetDoubles(prhs[0]);
      double *wOut = mxGetDoubles(plhs[0]);
#pragma omp parallel for
      for (size_t i = 0; i < N; ++i) {
#if FADDEEVA_REAL == 1
        wOut[i] = FADDEEVA_FUNC(z[i]);
#else
        std::complex<double> w =
            FADDEEVA_FUNC(std::complex<double>(z[i], 0), relerr);
        wOut[i].real = real(w);
        wOut[i].imag = imag(w);
#endif
      }
    }
  } else { // single precision
    if (mxIsComplex(prhs[0])) {
      mxComplexSingle *z = mxGetComplexSingles(prhs[0]);
      mxComplexSingle *wOut = mxGetComplexSingles(plhs[0]);
#pragma omp parallel for
      for (size_t i = 0; i < N; ++i) {
        std::complex<double> w =
            FADDEEVA_FUNC(std::complex<double>(z[i].real, z[i].imag), relerr);
        wOut[i].real = real(w);
        wOut[i].imag = imag(w);
      }
    } else {
      float *z = mxGetSingles(prhs[0]);
      float *wOut = mxGetSingles(plhs[0]);
#pragma omp parallel for
      for (size_t i = 0; i < N; ++i) {
#if FADDEEVA_REAL == 1
        wOut[i] = FADDEEVA_FUNC(z[i]);
#else
        std::complex<double> w =
            FADDEEVA_FUNC(std::complex<double>(z[i], 0), relerr);
        wOut[i].real = real(w);
        wOut[i].imag = imag(w);
#endif
      }
    }
  }
}

