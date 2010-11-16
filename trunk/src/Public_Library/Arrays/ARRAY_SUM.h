//#####################################################################
// Copyright 2007, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAY_SUM
//#####################################################################
#ifndef __ARRAY_SUM__
#define __ARRAY_SUM__

#include <Arrays/ARRAYS_FORWARD.h>
#include <Matrices_And_Vectors/ARITHMETIC_POLICY.h>
#include <cassert>
namespace PhysBAM{

template<class T_ARRAY1,class T_ARRAY2> class ARRAY_SUM;
template<class T_ARRAY1,class T_ARRAY2> struct IS_ARRAY<ARRAY_SUM<T_ARRAY1,T_ARRAY2> >:public mpl::true_{};
template<class T_ARRAY1,class T_ARRAY2> struct IS_ARRAY_VIEW<ARRAY_SUM<T_ARRAY1,T_ARRAY2> >:public mpl::true_{};

template<class T_ARRAY1,class T_ARRAY2>
class ARRAY_SUM:public ARRAY_EXPRESSION<typename SUM<typename T_ARRAY1::ELEMENT,typename T_ARRAY2::ELEMENT>::TYPE,ARRAY_SUM<T_ARRAY1,T_ARRAY2>,typename T_ARRAY1::INDEX>
{
    typedef typename T_ARRAY1::ELEMENT T1;typedef typename T_ARRAY2::ELEMENT T2;
    typedef typename IF<IS_ARRAY_VIEW<T_ARRAY1>::value,const T_ARRAY1,const T_ARRAY1&>::TYPE T_ARRAY1_VIEW; // if it's an array view we can copy it, otherwise store a reference
    typedef typename IF<IS_ARRAY_VIEW<T_ARRAY2>::value,const T_ARRAY2,const T_ARRAY2&>::TYPE T_ARRAY2_VIEW;
    typedef typename SUM<T1,T2>::TYPE T_SUM;
public:
    typedef T_SUM ELEMENT;typedef typename T_ARRAY1::INDEX INDEX;

    T_ARRAY1_VIEW array1;
    T_ARRAY2_VIEW array2;

    ARRAY_SUM(const T_ARRAY1& array1,const T_ARRAY2& array2)
        :array1(array1),array2(array2)
    {}

    INDEX Size() const
    {INDEX size=array1.Size();assert(size==array2.Size());return size;}

    const T_SUM operator()(const INDEX i) const
    {return array1(i)+array2(i);}

//#####################################################################
};

template<class T1,class T2,class T_ARRAY1,class T_ARRAY2> ARRAY_SUM<T_ARRAY1,T_ARRAY2>
operator+(const ARRAY_BASE<T1,T_ARRAY1,typename T_ARRAY2::INDEX>& array1,const ARRAY_BASE<T2,T_ARRAY2,typename T_ARRAY2::INDEX>& array2)
{return ARRAY_SUM<T_ARRAY1,T_ARRAY2>(array1.Derived(),array2.Derived());}

}
#endif
