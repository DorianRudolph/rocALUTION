/* ************************************************************************
 * Copyright (c) 2018 Advanced Micro Devices, Inc.
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

#ifndef ROCALUTION_MULTIGRID_HPP_
#define ROCALUTION_MULTIGRID_HPP_

#include "base_multigrid.hpp"

namespace rocalution {

/** \ingroup solver_module
  * \class MultiGrid
  * \brief MultiGrid Method
  * \details
  * The MultiGrid method can be used with external data, such as externally computed
  * restriction, prolongation and operator hierarchy.
  *
  * \tparam OperatorType - can be LocalMatrix or GlobalMatrix
  * \tparam VectorType - can be LocalVector or GlobalVector
  * \tparam ValueType - can be float, double, std::complex<float> or std::complex<double>
  */
template <class OperatorType, class VectorType, typename ValueType>
class MultiGrid : public BaseMultiGrid<OperatorType, VectorType, ValueType>
{
    public:
    MultiGrid();
    virtual ~MultiGrid();

    virtual void SetRestrictOperator(OperatorType** op);
    virtual void SetProlongOperator(OperatorType** op);
    virtual void SetOperatorHierarchy(OperatorType** op);
};

} // namespace rocalution

#endif // ROCALUTION_MULTIGRID_HPP_
