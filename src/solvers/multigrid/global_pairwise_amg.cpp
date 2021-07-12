/* ************************************************************************
 * Copyright (c) 2018-2021 Advanced Micro Devices, Inc.
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

#include "global_pairwise_amg.hpp"
#include "../../utils/def.hpp"
#include "../../utils/types.hpp"

#include "../../base/global_matrix.hpp"
#include "../../base/global_vector.hpp"
#include "../../base/local_matrix.hpp"
#include "../../base/local_vector.hpp"

#include "../preconditioners/preconditioner.hpp"

#include "../../utils/allocate_free.hpp"
#include "../../utils/log.hpp"
#include "../../utils/math_functions.hpp"

#include <complex>
#include <list>

namespace rocalution
{

    template <class OperatorType, class VectorType, typename ValueType>
    GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::GlobalPairwiseAMG()
    {
        log_debug(this, "GlobalPairwiseAMG::GlobalPairwiseAMG()", "default constructor");

        this->beta_        = static_cast<ValueType>(0.35);
        this->coarse_size_ = 1000;

        // target coarsening factor
        this->coarsening_factor_ = 4.0;

        // number of pre- and post-smoothing steps
        this->iter_pre_smooth_  = 2;
        this->iter_post_smooth_ = 2;

        // set K-cycle to default
        this->cycle_ = 2;

        // disable scaling
        this->scaling_ = false;

        // set default ordering to connectivity ordering
        this->aggregation_ordering_ = 1;
    }

    template <class OperatorType, class VectorType, typename ValueType>
    GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::~GlobalPairwiseAMG()
    {
        log_debug(this, "GlobalPairwiseAMG::GlobalPairwiseAMG()", "destructor");

        this->Clear();
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::Print(void) const
    {
        LOG_INFO("AMG solver");
        LOG_INFO("AMG number of levels " << this->levels_);
        LOG_INFO("AMG using pairwise aggregation");
        LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_ - 2]->GetM());
        int global_nnz = this->op_level_[this->levels_ - 2]->GetNnz();
        LOG_INFO("AMG coarsest level nnz = " << global_nnz);
        LOG_INFO("AMG with smoother:");
        this->smoother_level_[0]->Print();
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::PrintStart_(void) const
    {
        assert(this->levels_ > 0);

        LOG_INFO("AMG solver starts");
        LOG_INFO("AMG number of levels " << this->levels_);
        LOG_INFO("AMG using pairwise aggregation");
        LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_ - 2]->GetM());
        int global_nnz = this->op_level_[this->levels_ - 2]->GetNnz();
        LOG_INFO("AMG coarsest level nnz = " << global_nnz);
        LOG_INFO("AMG with smoother:");
        this->smoother_level_[0]->Print();
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::PrintEnd_(void) const
    {
        LOG_INFO("AMG ends");
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::SetBeta(ValueType beta)
    {
        log_debug(this, "GlobalPairwiseAMG::SetBeta()", beta);

        this->beta_ = beta;
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::SetCoarseningFactor(double factor)
    {
        log_debug(this, "GlobalPairwiseAMG::SetCoarseningFactor()", factor);

        this->coarsening_factor_ = factor;
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::SetOrdering(
        const _aggregation_ordering ordering)
    {
        log_debug(this, "GlobalPairwiseAMG::SetOrdering()", ordering);

        assert(ordering >= 0 && ordering <= 5);

        this->aggregation_ordering_ = ordering;
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::ReBuildNumeric(void)
    {
        assert(this->levels_ > 1);
        assert(this->build_ == true);
        assert(this->op_ != NULL);

        this->op_level_[0]->Clear();
        this->op_level_[0]->ConvertToCSR();

        this->op_->CoarsenOperator(this->op_level_[0],
                                   this->pm_level_[0],
                                   this->dim_level_[0],
                                   this->dim_level_[0],
                                   *this->trans_level_[0],
                                   this->Gsize_level_[0],
                                   this->rG_level_[0],
                                   this->rGsize_level_[0]);

        for(int i = 1; i < this->levels_ - 1; ++i)
        {
            this->op_level_[i]->Clear();
            this->op_level_[i]->ConvertToCSR();

            this->op_level_[i - 1]->CoarsenOperator(this->op_level_[i],
                                                    this->pm_level_[i],
                                                    this->dim_level_[i],
                                                    this->dim_level_[i],
                                                    *this->trans_level_[i],
                                                    this->Gsize_level_[i],
                                                    this->rG_level_[i],
                                                    this->rGsize_level_[i]);
        }

        this->smoother_level_[0]->ResetOperator(*this->op_);
        this->smoother_level_[0]->ReBuildNumeric();
        this->smoother_level_[0]->Verbose(0);

        for(int i = 1; i < this->levels_ - 1; ++i)
        {
            this->smoother_level_[i]->ResetOperator(*this->op_level_[i - 1]);
            this->smoother_level_[i]->ReBuildNumeric();
            this->smoother_level_[i]->Verbose(0);
        }

        this->solver_coarse_->ResetOperator(*this->op_level_[this->levels_ - 2]);
        this->solver_coarse_->ReBuildNumeric();
        this->solver_coarse_->Verbose(0);

        // Convert operator to op_format
        if(this->op_format_ != CSR)
        {
            for(int i = 0; i < this->levels_ - 1; ++i)
            {
                this->op_level_[i]->ConvertTo(this->op_format_);
            }
        }
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::ClearLocal(void)
    {
        log_debug(this, "GlobalPairwiseAMG::ClearLocal()", this->build_);

        if(this->build_ == true)
        {
            for(int i = 0; i < this->levels_ - 1; ++i)
            {
                free_host(&this->rG_level_[i]);
            }

            this->dim_level_.clear();
            this->Gsize_level_.clear();
            this->rGsize_level_.clear();
            this->rG_level_.clear();
        }
    }

    template <class OperatorType, class VectorType, typename ValueType>
    void
        GlobalPairwiseAMG<OperatorType, VectorType, ValueType>::Aggregate_(const OperatorType&  op,
                                                                           Operator<ValueType>* pro,
                                                                           Operator<ValueType>* res,
                                                                           OperatorType*     coarse,
                                                                           ParallelManager*  pm,
                                                                           LocalVector<int>* trans)
    {
        log_debug(
            this, "GlobalPairwiseAMG::Aggregate_()", (const void*&)op, pro, res, coarse, pm, trans);

        assert(pro != NULL);
        assert(res != NULL);
        assert(coarse != NULL);
        assert(trans != NULL);

        LocalMatrix<ValueType>* cast_res = dynamic_cast<LocalMatrix<ValueType>*>(res);
        LocalMatrix<ValueType>* cast_pro = dynamic_cast<LocalMatrix<ValueType>*>(pro);

        assert(cast_res != NULL);
        assert(cast_pro != NULL);

        int  nc;
        int* rG = NULL;
        int  Gsize;
        int  rGsize;

        // Allocate transfer mapping for current level
        trans->Allocate("transfer map", op.GetLocalM());

        op.InitialPairwiseAggregation(
            this->beta_, nc, trans, Gsize, &rG, rGsize, this->aggregation_ordering_);
        op.CoarsenOperator(coarse, pm, nc, nc, *trans, Gsize, rG, rGsize);

        while(static_cast<double>(op.GetM()) / static_cast<double>(coarse->GetM())
              < this->coarsening_factor_)
        {
            coarse->FurtherPairwiseAggregation(
                this->beta_, nc, trans, Gsize, &rG, rGsize, this->aggregation_ordering_);
            op.CoarsenOperator(coarse, pm, nc, nc, *trans, Gsize, rG, rGsize);
        }

        cast_res->CreateFromMap(*trans, op.GetLocalM(), nc, cast_pro);

        // Store data for possible coarse operator rebuild
        this->dim_level_.push_back(nc);
        this->Gsize_level_.push_back(Gsize);
        this->rGsize_level_.push_back(rGsize);
        this->rG_level_.push_back(rG);
    }

    template class GlobalPairwiseAMG<GlobalMatrix<double>, GlobalVector<double>, double>;
    template class GlobalPairwiseAMG<GlobalMatrix<float>, GlobalVector<float>, float>;
#ifdef SUPPORT_COMPLEX
    template class GlobalPairwiseAMG<GlobalMatrix<std::complex<double>>,
                                     GlobalVector<std::complex<double>>,
                                     std::complex<double>>;
    template class GlobalPairwiseAMG<GlobalMatrix<std::complex<float>>,
                                     GlobalVector<std::complex<float>>,
                                     std::complex<float>>;
#endif

} // namespace rocalution
