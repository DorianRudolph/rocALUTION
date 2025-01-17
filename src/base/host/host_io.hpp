/* ************************************************************************
 * Copyright (C) 2018-2020 Advanced Micro Devices, Inc. All rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#ifndef ROCALUTION_HOST_IO_HPP_
#define ROCALUTION_HOST_IO_HPP_

#include <string>

namespace rocalution
{

    template <typename ValueType>
    bool read_matrix_mtx(int&        nrow,
                         int&        ncol,
                         int&        nnz,
                         int**       row,
                         int**       col,
                         ValueType** val,
                         const char* filename);

    template <typename ValueType>
    bool write_matrix_mtx(int              nrow,
                          int              ncol,
                          int              nnz,
                          const int*       row,
                          const int*       col,
                          const ValueType* val,
                          const char*      filename);

    template <typename ValueType>
    bool read_matrix_csr(int&        nrow,
                         int&        ncol,
                         int&        nnz,
                         int**       ptr,
                         int**       col,
                         ValueType** val,
                         const char* filename);

    template <typename ValueType>
    bool write_matrix_csr(int              nrow,
                          int              ncol,
                          int              nnz,
                          const int*       ptr,
                          const int*       col,
                          const ValueType* val,
                          const char*      filename);

} // namespace rocalution

#endif // ROCALUTION_HOST_IO_HPP_
