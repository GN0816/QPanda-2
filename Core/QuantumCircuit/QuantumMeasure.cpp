/*
Copyright (c) 2017-2018 Origin Quantum Computing. All Right Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "QuantumMeasure.h"
#include "Utilities/ConfigMap.h"
USING_QPANDA
using namespace std;
QMeasure  QPanda::Measure(Qubit * target_qubit,ClassicalCondition  classical_cond)
{
    auto target_cbit = classical_cond.getExprPtr()->getCBit();
    if (nullptr == target_cbit)
    {
        QCERR("param error");
        throw invalid_argument("param error");
    }
    QMeasure measure(target_qubit, target_cbit);
    return measure;
}

QMeasure::QMeasure(const QMeasure & old_measure)
{
    m_measure = old_measure.m_measure;
}

QMeasure::QMeasure(Qubit * qubit, CBit * cbit)
{
    auto class_name = ConfigMap::getInstance()["QMeasure"];
    auto measure = QuantunMeasureFactory::getInstance().getQuantumMeasure(class_name, qubit, cbit);
    m_measure.reset(measure);
}

std::shared_ptr<QNode> QMeasure::getImplementationPtr()
{
    if (!m_measure)
    {
        QCERR("Unknown internal error");
        throw runtime_error("Unknown internal error");
    }
    return dynamic_pointer_cast<QNode>(m_measure);
}

QMeasure::~QMeasure()
{
    m_measure.reset();
}

Qubit * QMeasure::getQuBit() const
{
    if (nullptr == m_measure)
    {
        QCERR("Unknown internal error");
        throw runtime_error("Unknown internal error");
    }
    return m_measure->getQuBit();
}

CBit * QMeasure::getCBit() const
{
    if (!m_measure)
    {
        QCERR("Unknown internal error");
        throw runtime_error("Unknown internal error");
    }
    return m_measure->getCBit();
}


NodeType QMeasure::getNodeType() const
{
    if (!m_measure)
    {
        QCERR("Unknown internal error");
        throw runtime_error("Unknown internal error");
    }
    return (dynamic_pointer_cast<QNode >(m_measure))->getNodeType();
}

void QuantunMeasureFactory::registClass(string name, CreateMeasure method)
{
    m_measureMap.insert(pair<string, CreateMeasure>(name, method));
}

AbstractQuantumMeasure * QuantunMeasureFactory::getQuantumMeasure(std::string & classname, Qubit * pQubit, CBit * pCBit)
{
    auto aiter = m_measureMap.find(classname);
    if (aiter != m_measureMap.end())
    {
        return aiter->second(pQubit, pCBit);
    }
    else
    {
        QCERR("can not find targit measure class");
        throw runtime_error("can not find targit measure class");
    }
}

NodeType OriginMeasure::getNodeType() const
{
    return m_node_type;
}

void OriginMeasure::execute(QPUImpl * quantum_gates, QuantumGateParam * param)
{
    int iResult = quantum_gates->qubitMeasure(getQuBit()->getPhysicalQubitPtr()->getQubitAddr());
    if (iResult < 0)
    {
        QCERR("result error");
        throw runtime_error("result error");
    }
    CBit * cexpr = getCBit();
    if (nullptr == cexpr)
    {
        QCERR("unknow error");
        throw runtime_error("unknow error");
    }

    cexpr->setValue(iResult);

    string name = cexpr->getName();
    auto aiter = param->m_return_value.find(name);
    if (aiter != param->m_return_value.end())
    {
        aiter->second = (bool)iResult;
    }
    else
    {
        param->m_return_value.insert(pair<string, bool>(name, (bool)iResult));
    }
}

OriginMeasure::OriginMeasure(Qubit * qubit, CBit * cbit):
    m_target_qubit(qubit),
    m_target_cbit(cbit),
    m_node_type(MEASURE_GATE)
{
}

Qubit * OriginMeasure::getQuBit() const
{
    return m_target_qubit;
}

CBit * OriginMeasure::getCBit() const
{
    return m_target_cbit;
}

REGISTER_MEASURE(OriginMeasure);
